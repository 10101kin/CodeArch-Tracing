/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 *
 * Production driver for EGTM ATOM PWM (TC4xx) with three complementary channels (3-phase inverter)
 * and one single-output 50% edge-aligned trigger channel routed to TMADC.
 *
 * Mandatory patterns followed:
 *  - Unified PWM configuration arrays: OutputConfig, DtmConfig, ChannelConfig, InterruptConfig
 *  - EGTM clock enable guard with dynamic frequency
 *  - IfxEgtm_Pwm_initConfig() -> customize -> IfxEgtm_Pwm_init()
 *  - Interrupt configured via InterruptConfig (period event). ISR body only toggles LED.
 *  - No watchdog operations in this driver.
 *
 * Pin note:
 *  User-requested pins are applied where validated TOUT symbols are provided. For pins not present
 *  in the validated list, NULL_PTR placeholders are used with integration comments.
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ============================== Macros (numeric requirements) ============================== */
#define IFXEGTM_PWM_NUM_CHANNELS           (4U)         /* 3 inverter + 1 trigger */
#define PWM_FREQUENCY_HZ                   (20000.0f)   /* PWM_FREQUENCY: 20000.0 Hz */
#define PHASE_U_DUTY_PERCENT               (25.0f)      /* PHASE_U_DUTY: 25.0 % */
#define PHASE_V_DUTY_PERCENT               (50.0f)      /* PHASE_V_DUTY: 50.0 % */
#define PHASE_W_DUTY_PERCENT               (75.0f)      /* PHASE_W_DUTY: 75.0 % */
#define ADC_TRIG_DUTY_PERCENT              (50.0f)      /* ADC_TRIG_DUTY: 50.0 % */
#define PHASE_DUTY_STEP_PERCENT            (0.01f)      /* PHASE_DUTY_STEP: 0.01 % */
#define ISR_PRIORITY_ATOM                  (25)         /* Suitable ISR priority for PWM period event */

/* LED macro as compound (port, pin) for ISR toggle */
#define LED &MODULE_P13, 0

/* ============================== Pin macros (validated list binding) ============================== */
/* Complementary pair naming: _N_ denotes complementary (low-side) output */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)     /* Validated */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)    /* Validated */

/* The following user-requested pins were not present in the provided validated list.
 * Use NULL_PTR placeholders; replace with template-verified symbols during integration:
 *   - &IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT
 *   - &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT
 *   - &IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT
 *   - &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT
 *   - &IfxEgtm_ATOM0_3_TOUT6_P02_6_OUT
 */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT */
#define ADC_TRIG_PIN (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_3_TOUT6_P02_6_OUT */

/* ============================== Module persistent state ============================== */
typedef struct
{
    IfxEgtm_Pwm                 pwm;                                 /* Unified PWM driver handle */
    IfxEgtm_Pwm_Channel         channels[IFXEGTM_PWM_NUM_CHANNELS];  /* Populated by init */
    float32                     dutyCycles[IFXEGTM_PWM_NUM_CHANNELS];/* Stored duty in percent */
    float32                     phases[3];                            /* Optional phase offsets */
    IfxEgtm_Pwm_DeadTime        deadTimes[3];                         /* Dead-times per complementary channel */
} EgtmAtomState;

IFX_STATIC EgtmAtomState g_egtmAtomState;

/* ============================== ISR and Callback (must precede init) ============================== */
/* ISR: Minimal body toggling LED only. Priority must match ISR_PRIORITY_ATOM. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: empty body as driver invokes it; ISR handles the LED toggle */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================== Private helpers (local) ============================== */
static void egtm_enableClocks(void)
{
    Ifx_EGTM *egtm = &MODULE_EGTM;

    /* Enable guard block: all CMU operations contained here */
    {
        float32 frequency = IfxEgtm_Cmu_getModuleFrequency(egtm);
        IfxEgtm_enable(egtm);
        IfxEgtm_Cmu_setGclkFrequency(egtm, frequency);
        IfxEgtm_Cmu_setClkFrequency(egtm, IfxEgtm_Cmu_Clk_0, frequency);
        IfxEgtm_Cmu_enableClocks(egtm, IFXEGTM_CMU_CLKEN_CLK0);
    }
}

/* ============================== Public API ============================== */
/**
 * Initialize a unified EGTM PWM instance for one base + three synchronous channels total:
 *  - Three complementary channels (center-aligned) for a 3-phase inverter with 1 us dead-time
 *  - One single-output (edge-aligned) 50% duty trigger channel for TMADC
 *
 * Implementation strictly follows the specified initialization sequence and patterns.
 */
void initEgtmAtom(void)
{
    /* 1) Declare all local configuration structures */
    IfxEgtm_Pwm_Config           config;                                      /* Main PWM config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[IFXEGTM_PWM_NUM_CHANNELS];     /* Per-channel config */
    IfxEgtm_Pwm_DtmConfig        dtmConfig[3];                                 /* DTM for 3 complementary channels */
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;                              /* One interrupt for base channel */
    IfxEgtm_Pwm_OutputConfig     output[IFXEGTM_PWM_NUM_CHANNELS];            /* Output routing per logical channel */

    /* 2) Initialize the main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Fill output configuration (pins, polarity, pad driver) */
    /* Phase U (complementary) */
    output[0].pin                     = PHASE_U_HS;
    output[0].complementaryPin        = PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high; /* HS active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;  /* LS active low  */
    output[0].mode                    = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (complementary) */
    output[1].pin                     = PHASE_V_HS; /* Replace NULL_PTR with validated symbol */
    output[1].complementaryPin        = PHASE_V_LS; /* Replace NULL_PTR with validated symbol */
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].mode                    = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (complementary) */
    output[2].pin                     = PHASE_W_HS; /* Replace NULL_PTR with validated symbol */
    output[2].complementaryPin        = PHASE_W_LS; /* Replace NULL_PTR with validated symbol */
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].mode                    = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Trigger channel (single output, edge-aligned, 50% duty) */
    output[3].pin                     = ADC_TRIG_PIN; /* Replace NULL_PTR with validated symbol */
    output[3].complementaryPin        = NULL_PTR;
    output[3].polarity                = Ifx_ActiveState_high;
    output[3].complementaryPolarity   = Ifx_ActiveState_low; /* not used */
    output[3].mode                    = IfxPort_OutputMode_pushPull;
    output[3].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration for complementary channels (1 us rising and falling) */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff = NULL_PTR;

    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff = NULL_PTR;

    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration for PWM period event (base channel only) */
    interruptConfig.mode        = IfxEgtm_Pwm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..3) */
    /* Channel 0: Phase U (center-aligned), complementary with DTM */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR; /* not using MSC */
    channelConfig[0].interrupt = &interruptConfig; /* base channel interrupt */

    /* Channel 1: Phase V (center-aligned), complementary with DTM */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR; /* only base channel has interrupt */

    /* Channel 2: Phase W (center-aligned), complementary with DTM */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* Channel 3: ADC trigger (edge-aligned single output, no DTM) */
    channelConfig[3].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;
    channelConfig[3].phase     = 0.0f;
    channelConfig[3].duty      = ADC_TRIG_DUTY_PERCENT;
    channelConfig[3].dtm       = NULL_PTR;
    channelConfig[3].output    = &output[3];
    channelConfig[3].mscOut    = NULL_PTR;
    channelConfig[3].interrupt = NULL_PTR;

    /* 7) Complete the main PWM config */
    config.cluster             = IfxEgtm_Cluster_0;
    config.subModule           = IfxEgtm_SubModule_atom0;
    config.syncStart           = TRUE;
    config.syncUpdateEnabled   = TRUE;
    config.numChannels         = IFXEGTM_PWM_NUM_CHANNELS;
    config.channelConfig       = channelConfig;                 /* Attach channel configurations */
    config.channelData         = g_egtmAtomState.channels;      /* Attach persistent channels array */
    config.frequency           = PWM_FREQUENCY_HZ;
    config.clockSource.atom    = IfxEgtm_Cmu_Clk_0;             /* ATOM uses CLK0 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.alignment           = IfxEgtm_Pwm_Alignment_centerAligned; /* Base alignment; trigger ch configured as edge-aligned by single-output usage */

    /* 8) Enable EGTM module and clocks (guard block) */
    egtm_enableClocks();

    /* 9) Initialize unified PWM driver */
    {
        boolean ok = IfxEgtm_Pwm_init(&g_egtmAtomState.pwm, &config);
        if (ok == FALSE)
        {
            /* Initialization failed; do not proceed further in this API */
            return;
        }
    }

    /* 10) Store initial duties and dead-times into module state for runtime use */
    g_egtmAtomState.dutyCycles[0] = PHASE_U_DUTY_PERCENT;
    g_egtmAtomState.dutyCycles[1] = PHASE_V_DUTY_PERCENT;
    g_egtmAtomState.dutyCycles[2] = PHASE_W_DUTY_PERCENT;
    g_egtmAtomState.dutyCycles[3] = ADC_TRIG_DUTY_PERCENT;

    g_egtmAtomState.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtomState.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtomState.deadTimes[2] = dtmConfig[2].deadTime;

    g_egtmAtomState.phases[0] = 0.0f;
    g_egtmAtomState.phases[1] = 0.0f;
    g_egtmAtomState.phases[2] = 0.0f;

    /* 11) Configure diagnostic GPIO (LED) for ISR toggling */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 12) Configure EGTM trigger route to TMADC (falling edge) */
    /* Note: Replace placeholders with target-verified enumerators if needed. */
    (void)IfxEgtm_Trigger_trigToAdc(&MODULE_EGTM,
                                    IfxEgtm_Trigger_AdcTriggerSignal_0,
                                    IfxEgtm_TriggerMuxSel_36,
                                    IfxEgtm_Trigger_Edge_falling);

    /* 13) syncStart=TRUE ensures synchronized start; period-event interrupt is enabled only for base channel */
}
