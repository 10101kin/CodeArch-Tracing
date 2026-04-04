/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 *
 * Production driver code for TC4xx EGTM ATOM PWM (3 complementary inverter phases + 1 ADC trigger channel)
 *
 * Requirements implemented:
 *  - Unified EGTM PWM instance with 4 logical channels (3 complementary + 1 single trigger)
 *  - Center-aligned complementary for phases U,V,W with 1us dead-time (DTM clock)
 *  - Edge-aligned 50% duty trigger channel for TMADC routing
 *  - Period-event interrupt on base channel using pulse-notify mode, provider CPU0
 *  - EGTM CMU clock enable block with dynamic frequency read (CLK0)
 *  - Pin routing via OutputConfig, no separate pin-map calls
 *  - Persistent module state for handle, channels and runtime parameters
 *  - Minimal ISR toggling a diagnostic GPIO; empty period-event callback
 *
 * Notes:
 *  - Watchdogs are NOT touched here (must be managed in CpuX_Main.c only)
 *  - No STM timing utilities are used here
 *  - Followed iLLD unified PWM init patterns, adapted to EGTM for TC4xx
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ============================
 * Configuration Macros (values from user-confirmed migration data)
 * ============================ */
#define PWM_NUM_LOGICAL_CHANNELS       (4u)
#define PWM_FREQUENCY                  (20000.0f)   /* Hz */
#define PHASE_U_DUTY                   (25.0f)      /* percent */
#define PHASE_V_DUTY                   (50.0f)      /* percent */
#define PHASE_W_DUTY                   (75.0f)      /* percent */
#define ADC_TRIG_DUTY                  (50.0f)      /* percent */
#define PHASE_DUTY_STEP                (0.01f)      /* percent - not used here, reserved for updater API */
#define ISR_PRIORITY_ATOM              (100)        /* Interrupt priority for period-event */

/* Diagnostic LED/GPIO (toggle in ISR) - compound macro: &MODULE_Pxx, y */
#define LED                            &MODULE_P03, 9

/* ============================
 * Pin Routing Macros (validated user-requested pin assignments)
 * ============================ */
#define PHASE_U_HS                     (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                     (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)
#define PHASE_V_HS                     (&IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT)
#define PHASE_V_LS                     (&IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT)
#define PHASE_W_HS                     (&IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT)
#define PHASE_W_LS                     (&IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT)
#define ADC_TRIGGER_OUT                (&IfxEgtm_ATOM0_3_TOUT6_P02_6_OUT)

/* ============================
 * Module persistent state
 * ============================ */

typedef struct
{
    IfxEgtm_Pwm                 pwm;                                   /* unified PWM handle */
    IfxEgtm_Pwm_Channel         channels[PWM_NUM_LOGICAL_CHANNELS];    /* channel runtime data (driver fills this) */
    float32                     dutyCycles[PWM_NUM_LOGICAL_CHANNELS];  /* last commanded duty in percent */
    float32                     phases[3];                              /* electrical phase offsets for 3 inverter phases */
    IfxEgtm_Pwm_DeadTime        deadTimes[3];                           /* dead-time used per complementary phase */
} EgtmAtom_State;

IFX_STATIC EgtmAtom_State g_egtmAtomState; /* zero-initialized by C runtime */
IFX_STATIC boolean        g_egtmCmuInitDone = FALSE; /* CMU enable guard */

/* ============================
 * ISR and Callback
 * ============================ */

/* Period-event callback: MUST be non-static and empty body */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR: toggle diagnostic GPIO only (vector/prio resolved by IFX_INTERRUPT and InterruptConfig) */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* ============================
 * Local helpers (internal only)
 * ============================ */
IFX_STATIC void egtm_enableClocksOnce(void)
{
    if (g_egtmCmuInitDone == FALSE)
    {
        Ifx_EGTM *egtm = &MODULE_EGTM;

        /* enable EGTM clock (pattern adapted from GTM example, migrated to EGTM) */
        {
            float32 frequency = IfxEgtm_Cmu_getModuleFrequency(egtm);
            IfxEgtm_enable(egtm);
            IfxEgtm_Cmu_setGclkFrequency(egtm, frequency);
            IfxEgtm_Cmu_setClkFrequency(egtm, IfxEgtm_Cmu_Clk_0, frequency);
            /* FXCLK used by TOM; CLK0 used by ATOM. Keep both enabled for flexibility */
            IfxEgtm_Cmu_enableClocks(egtm, IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0);
        }

        g_egtmCmuInitDone = TRUE;
    }
}

/* ============================
 * Public API implementation
 * ============================ */

/*
 * Initialize unified EGTM PWM for:
 *  - Three complementary center-aligned inverter channels (U,V,W) on ATOM0 CH0..CH2, with 1us dead-time
 *  - One single-output edge-aligned 50% duty trigger channel on ATOM0 CH3 (intended for TMADC trigger)
 *  - Period-event interrupt on base logical channel (channel 0)
 */
void initEgtmAtom(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;                                        /* main PWM config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[PWM_NUM_LOGICAL_CHANNELS];      /* per-channel config */
    IfxEgtm_Pwm_OutputConfig     outputCfg[PWM_NUM_LOGICAL_CHANNELS];          /* per-channel output pins */
    IfxEgtm_Pwm_DtmConfig        dtmCfg[3];                                     /* DTM for complementary channels */
    IfxEgtm_Pwm_InterruptConfig  irqCfg;                                        /* period-event interrupt */

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configurations */
    /* Phase U (logical channel 0) */
    outputCfg[0].pin                      = PHASE_U_HS;
    outputCfg[0].complementaryPin         = PHASE_U_LS;
    outputCfg[0].polarity                 = Ifx_ActiveState_high;  /* high-side active HIGH */
    outputCfg[0].complementaryPolarity    = Ifx_ActiveState_low;   /* low-side  active LOW  */
    outputCfg[0].outputMode               = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (logical channel 1) */
    outputCfg[1].pin                      = PHASE_V_HS;
    outputCfg[1].complementaryPin         = PHASE_V_LS;
    outputCfg[1].polarity                 = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity    = Ifx_ActiveState_low;
    outputCfg[1].outputMode               = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (logical channel 2) */
    outputCfg[2].pin                      = PHASE_W_HS;
    outputCfg[2].complementaryPin         = PHASE_W_LS;
    outputCfg[2].polarity                 = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity    = Ifx_ActiveState_low;
    outputCfg[2].outputMode               = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Trigger channel (logical channel 3) */
    outputCfg[3].pin                      = ADC_TRIGGER_OUT;
    outputCfg[3].complementaryPin         = NULL_PTR; /* single output */
    outputCfg[3].polarity                 = Ifx_ActiveState_high;
    outputCfg[3].complementaryPolarity    = Ifx_ActiveState_low;   /* unused */
    outputCfg[3].outputMode               = IfxPort_OutputMode_pushPull;
    outputCfg[3].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time (DTM) for complementary channels: 1 microsecond rising and falling */
    dtmCfg[0].deadTime.rising             = 1e-6f;
    dtmCfg[0].deadTime.falling            = 1e-6f;
    dtmCfg[0].fastShutOff                 = NULL_PTR; /* not used */

    dtmCfg[1].deadTime.rising             = 1e-6f;
    dtmCfg[1].deadTime.falling            = 1e-6f;
    dtmCfg[1].fastShutOff                 = NULL_PTR;

    dtmCfg[2].deadTime.rising             = 1e-6f;
    dtmCfg[2].deadTime.falling            = 1e-6f;
    dtmCfg[2].fastShutOff                 = NULL_PTR;

    /* 5) Interrupt configuration for PWM period event (base channel only) */
    irqCfg.mode                           = IfxEgtm_Pwm_IrqMode_pulseNotify;
    irqCfg.isrProvider                    = IfxSrc_Tos_cpu0;
    irqCfg.priority                       = ISR_PRIORITY_ATOM;
    irqCfg.vmId                           = IfxSrc_VmId_0;
    irqCfg.periodEvent                    = &IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent                      = NULL_PTR;  /* not used */

    /* 6) Per-channel configuration */
    /* Logical channel indices CH0..CH3 map to ATOM0 channels 0..3 respectively */

    /* CH0: Phase U - complementary, center-aligned */
    channelConfig[0].timerCh              = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase                = 0.0f;
    channelConfig[0].duty                 = PHASE_U_DUTY;
    channelConfig[0].alignment            = IfxEgtm_Pwm_Alignment_centerAligned;
    channelConfig[0].dtm                  = &dtmCfg[0];
    channelConfig[0].output               = &outputCfg[0];
    channelConfig[0].mscOut               = NULL_PTR; /* not used */
    channelConfig[0].interrupt            = &irqCfg;  /* base channel gets the interrupt */

    /* CH1: Phase V - complementary, center-aligned */
    channelConfig[1].timerCh              = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase                = 0.0f;
    channelConfig[1].duty                 = PHASE_V_DUTY;
    channelConfig[1].alignment            = IfxEgtm_Pwm_Alignment_centerAligned;
    channelConfig[1].dtm                  = &dtmCfg[1];
    channelConfig[1].output               = &outputCfg[1];
    channelConfig[1].mscOut               = NULL_PTR;
    channelConfig[1].interrupt            = NULL_PTR; /* no interrupt on this channel */

    /* CH2: Phase W - complementary, center-aligned */
    channelConfig[2].timerCh              = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase                = 0.0f;
    channelConfig[2].duty                 = PHASE_W_DUTY;
    channelConfig[2].alignment            = IfxEgtm_Pwm_Alignment_centerAligned;
    channelConfig[2].dtm                  = &dtmCfg[2];
    channelConfig[2].output               = &outputCfg[2];
    channelConfig[2].mscOut               = NULL_PTR;
    channelConfig[2].interrupt            = NULL_PTR; /* no interrupt on this channel */

    /* CH3: ADC trigger - single output, edge-aligned 50% */
    channelConfig[3].timerCh              = IfxEgtm_Pwm_SubModule_Ch_3;
    channelConfig[3].phase                = 0.0f;
    channelConfig[3].duty                 = ADC_TRIG_DUTY;
    channelConfig[3].alignment            = IfxEgtm_Pwm_Alignment_edgeAligned;
    channelConfig[3].dtm                  = NULL_PTR;           /* no dead-time for single output */
    channelConfig[3].output               = &outputCfg[3];
    channelConfig[3].mscOut               = NULL_PTR;
    channelConfig[3].interrupt            = NULL_PTR;           /* no interrupt on trigger channel */

    /* 7) Complete main PWM configuration */
    config.cluster                        = IfxEgtm_Cluster_0;             /* EGTM Cluster 0 */
    config.subModule                      = IfxEgtm_Pwm_SubModule_atom;    /* ATOM submodule */
    config.syncStart                      = TRUE;                          /* enable sync start */
    config.syncUpdateEnabled              = TRUE;                          /* enable sync update */
    config.numChannels                    = PWM_NUM_LOGICAL_CHANNELS;
    config.channels                       = channelConfig;                 /* attach per-channel config */
    config.frequency                      = PWM_FREQUENCY;                 /* 20 kHz */
    config.clockSource.atom               = IfxEgtm_Cmu_Clk_0;             /* ATOM uses CLK0 */
    config.dtmClockSource                 = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM driven by CMU CLK0 */

    /* 8) EGTM CMU clock enable (guarded) */
    egtm_enableClocksOnce();

    /* 9) Initialize unified PWM driver (applies duties, alignments, DTMs and pin routing) */
    {
        boolean ok = IfxEgtm_Pwm_init(&g_egtmAtomState.pwm, g_egtmAtomState.channels, &config);
        if (ok != TRUE)
        {
            /* In production, add error handling/logging here */
            return;
        }
    }

    /* 10) Store initial state for runtime use */
    g_egtmAtomState.dutyCycles[0] = channelConfig[0].duty; /* U */
    g_egtmAtomState.dutyCycles[1] = channelConfig[1].duty; /* V */
    g_egtmAtomState.dutyCycles[2] = channelConfig[2].duty; /* W */
    g_egtmAtomState.dutyCycles[3] = channelConfig[3].duty; /* Trigger */

    g_egtmAtomState.phases[0]     = channelConfig[0].phase;
    g_egtmAtomState.phases[1]     = channelConfig[1].phase;
    g_egtmAtomState.phases[2]     = channelConfig[2].phase;

    g_egtmAtomState.deadTimes[0]  = dtmCfg[0].deadTime;
    g_egtmAtomState.deadTimes[1]  = dtmCfg[1].deadTime;
    g_egtmAtomState.deadTimes[2]  = dtmCfg[2].deadTime;

    /* 11) Configure diagnostic GPIO for ISR toggling */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 12) (Optional here) Route EGTM trigger to TMADC via trigger unit if required by integration
       The unified PWM already drives the trigger pin at 50% duty, 20 kHz on the configured pad.
       ADC trigger routing to TMADC (e.g., IfxEgtm_Trigger_trigToAdc to AdcTriggerSignal_0, falling edge)
       shall be performed in the ADC init module according to the selected TMADC trigger mux. */
}
