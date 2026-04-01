/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 * Production driver: EGTM ATOM PWM (3-phase complementary) + ADC trigger channel (TMADC)
 *
 * Notes:
 * - Follows unified IfxEgtm_Pwm initialization pattern with arrays for channels, outputs, and DTM.
 * - Uses EGTM Cluster 0, ATOM0 channels 0..2 for 3-phase, and channel 3 for ADC trigger (50% edge-aligned).
 * - CMU clocks enabled inside an enable-guard; dynamic module frequency read.
 * - Interrupt configured only on base channel (logical channel 0) with period event callback.
 * - Watchdog handling MUST NOT be here per AURIX architecture; keep in CpuX_Main.c only.
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ======================== Configuration Macros (numeric values from requirements) ======================== */
#define EGTM_NUM_INVERTER_CHANNELS    (3)
#define EGTM_NUM_LOGICAL_CHANNELS     (4) /* 3 inverter + 1 trigger */

#define PWM_FREQUENCY                 (20000.0f)        /* Hz */
#define PHASE_U_DUTY                  (25.0f)           /* percent */
#define PHASE_V_DUTY                  (50.0f)           /* percent */
#define PHASE_W_DUTY                  (75.0f)           /* percent */
#define ADC_TRIG_DUTY                 (50.0f)           /* percent */
#define PHASE_DUTY_STEP               (0.01f)           /* percent step (not used here; for runtime updates in application) */

#define ISR_PRIORITY_ATOM             (100)             /* Interrupt priority for PWM period event */

/* LED/diagnostic toggle pin as compound macro (port, pin) */
#define LED                           &MODULE_P03, 9

/* ======================== Pin Macros (validated pin symbols only) ======================== */
/* User-requested pins; only use symbols that exist in the validated list. Others set to NULL_PTR with integration note. */
#define PHASE_U_HS                    (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)   /* P02.0 */
#define PHASE_U_LS                    (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)  /* P02.1 */
#define PHASE_V_HS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT when available in PinMap */
#define PHASE_V_LS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT when available in PinMap */
#define PHASE_W_HS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT when available in PinMap */
#define PHASE_W_LS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT when available in PinMap */
#define ADC_TRIG_OUT                  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_3_TOUT6_P02_6_OUT when available in PinMap */

/* ======================== Module State ======================== */
typedef struct
{
    IfxEgtm_Pwm               pwm;                                 /* Unified EGTM PWM driver handle */
    IfxEgtm_Pwm_Channel       channels[EGTM_NUM_LOGICAL_CHANNELS]; /* Persistent channel state array */
    float32                   dutyCycles[EGTM_NUM_LOGICAL_CHANNELS];
    float32                   phases[EGTM_NUM_INVERTER_CHANNELS];
    IfxEgtm_Pwm_DeadTime      deadTimes[EGTM_NUM_INVERTER_CHANNELS];
} EgtmAtomModuleState;

IFX_STATIC EgtmAtomModuleState g_egtmAtomState;

/* ======================== ISR and Period Callback ======================== */
/* ISR: MUST only toggle diagnostic GPIO. Priority must match ISR_PRIORITY_ATOM. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period event callback: MUST be non-static and have empty body. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ======================== initEgtmAtom Implementation ======================== */
/*
 * Initialize a unified EGTM PWM instance for:
 *  - Three complementary channels (center-aligned, 1 us dead-time)
 *  - One single-output channel (edge-aligned, 50% duty) used as ADC trigger
 */
void initEgtmAtom(void)
{
    /* 1) Declare all local configuration structures */
    IfxEgtm_Pwm_Config           config;                                    /* Main PWM config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_NUM_LOGICAL_CHANNELS];  /* Per-channel configs */
    IfxEgtm_Pwm_OutputConfig     output[EGTM_NUM_LOGICAL_CHANNELS];         /* Output routing configs */
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM_NUM_INVERTER_CHANNELS];     /* Dead-time configs for complementary channels */
    IfxEgtm_Pwm_InterruptConfig  irqCfg;                                     /* Period-event interrupt config */

    /* 2) Initialize the main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Fill output configuration for each logical PWM channel */
    /* Phase U (Channel 0): complementary */
    output[0].pin                     = PHASE_U_HS;
    output[0].complementaryPin        = PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;            /* High-side active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;             /* Low-side active low */
    output[0].mode                    = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (Channel 1): complementary */
    output[1].pin                     = PHASE_V_HS; /* NULL_PTR until validated symbol is provided */
    output[1].complementaryPin        = PHASE_V_LS; /* NULL_PTR until validated symbol is provided */
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].mode                    = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (Channel 2): complementary */
    output[2].pin                     = PHASE_W_HS; /* NULL_PTR until validated symbol is provided */
    output[2].complementaryPin        = PHASE_W_LS; /* NULL_PTR until validated symbol is provided */
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].mode                    = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Trigger channel (Channel 3): single output, edge-aligned 50% */
    output[3].pin                     = ADC_TRIG_OUT; /* NULL_PTR until validated symbol is provided */
    output[3].complementaryPin        = NULL_PTR;
    output[3].polarity                = Ifx_ActiveState_high;
    output[3].complementaryPolarity   = Ifx_ActiveState_low; /* don't care */
    output[3].mode                    = IfxPort_OutputMode_pushPull;
    output[3].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration for complementary inverter channels (1 us both edges) */
    dtmConfig[0].deadTime.rising      = 1e-6f;
    dtmConfig[0].deadTime.falling     = 1e-6f;
    dtmConfig[0].fastShutOff          = NULL_PTR;

    dtmConfig[1].deadTime.rising      = 1e-6f;
    dtmConfig[1].deadTime.falling     = 1e-6f;
    dtmConfig[1].fastShutOff          = NULL_PTR;

    dtmConfig[2].deadTime.rising      = 1e-6f;
    dtmConfig[2].deadTime.falling     = 1e-6f;
    dtmConfig[2].fastShutOff          = NULL_PTR;

    /* 5) Configure interrupt for PWM period event (base channel only) */
    irqCfg.mode        = IfxEgtm_Pwm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: three center-aligned complementary + one edge-aligned trigger */
    /* Channel 0: Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg; /* base channel gets period interrupt */
    channelConfig[0].alignment = IfxEgtm_Pwm_Alignment_center;

    /* Channel 1: Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;
    channelConfig[1].alignment = IfxEgtm_Pwm_Alignment_center;

    /* Channel 2: Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;
    channelConfig[2].alignment = IfxEgtm_Pwm_Alignment_center;

    /* Channel 3: ADC trigger (edge-aligned 50%, no dead-time) */
    channelConfig[3].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;
    channelConfig[3].phase     = 0.0f;
    channelConfig[3].duty      = ADC_TRIG_DUTY;
    channelConfig[3].dtm       = NULL_PTR;
    channelConfig[3].output    = &output[3];
    channelConfig[3].mscOut    = NULL_PTR;
    channelConfig[3].interrupt = NULL_PTR;
    channelConfig[3].alignment = IfxEgtm_Pwm_Alignment_edge;

    /* Complete main PWM config */
    config.cluster             = IfxEgtm_Cluster_0;
    config.subModule           = IfxEgtm_SubModule_atom0;                 /* ATOM0 */
    config.syncStart           = TRUE;
    config.syncUpdateEnabled   = TRUE;
    config.numChannels         = EGTM_NUM_LOGICAL_CHANNELS;
    config.channels            = channelConfig;                           /* attach per-channel configs */
    config.frequency           = PWM_FREQUENCY;
    config.clockSource.atom    = IfxEgtm_Cmu_Clk_0;                        /* ATOM uses CLK0 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM source */

    /* 7) Enable-guard: if EGTM is not enabled, enable it and configure CMU clocks */
    if (FALSE == IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, frequency);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* 8) Initialize the unified PWM driver with persistent handle and channels array */
    {
        boolean ok;
        ok = IfxEgtm_Pwm_init(&g_egtmAtomState.pwm, g_egtmAtomState.channels, &config);
        if (ok == FALSE)
        {
            /* In production, add error handling/logging as needed */
            return;
        }
    }

    /* 9) Store initial duties, dead-times, and phases into module state */
    g_egtmAtomState.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtomState.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtomState.dutyCycles[2] = PHASE_W_DUTY;
    g_egtmAtomState.dutyCycles[3] = ADC_TRIG_DUTY;

    g_egtmAtomState.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtomState.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtomState.deadTimes[2] = dtmConfig[2].deadTime;

    g_egtmAtomState.phases[0] = 0.0f;
    g_egtmAtomState.phases[1] = 0.0f;
    g_egtmAtomState.phases[2] = 0.0f;

    /* 10) Configure diagnostic GPIO for ISR toggling */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 11) Configure EGTM trigger route to TMADC/ADC: ATOM0 CH3 -> AdcTriggerSignal_0 (falling edge) */
    /* The following call relies on the IfxEgtm_Trigger API available in iLLD. */
    IfxEgtm_Trigger_trigToAdc(&MODULE_EGTM,
                              IfxEgtm_SubModule_atom0,
                              IfxEgtm_Pwm_SubModule_Ch_3,
                              IfxEgtm_Trigger_AdcTriggerSignal_0,
                              IfxEgtm_Trigger_Edge_falling);

    /* 12) Start synchronized PWM channels: syncStart=TRUE in config handles activation */
    /* No additional start call required. Interrupt is enabled only on base channel by configuration. */
}
