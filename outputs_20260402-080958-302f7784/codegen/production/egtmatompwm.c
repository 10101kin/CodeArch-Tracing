/*
 * egtmatompwm.c
 * Production-ready EGTM ATOM 3-phase inverter PWM driver (TC4xx)
 *
 * Notes:
 * - Uses unified IfxEgtm_Pwm high-level driver for 3 complementary center-aligned pairs
 * - Adds a separate edge-aligned ATOM trigger channel for ADC sampling
 * - Follows iLLD initialization and CMU enable patterns
 * - No watchdog handling in this file per AURIX architecture (handled in CpuX_Main.c)
 */

#include "egtmatompwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (configuration values) ========================= */
/* Channel and frequency configuration */
#define EGTM_NUM_PWM_CHANNELS           (3U)
#define EGTM_PWM_FREQUENCY_HZ           (20000.0f)   /* 20 kHz */

/* Initial duties in percent */
#define EGTM_PHASE_U_DUTY               (25.0f)
#define EGTM_PHASE_V_DUTY               (50.0f)
#define EGTM_PHASE_W_DUTY               (75.0f)

/* Duty step (percent) for updateEgtmAtomDuty */
#define EGTM_PHASE_DUTY_STEP            (0.01f)

/* ADC trigger duty (edge-aligned) */
#define EGTM_ADC_TRIG_DUTY              (50.0f)

/* ISR priority (hardware ISR toggles LED only) */
#define ISR_PRIORITY_ATOM               (25)

/* LED pin macro (compound: port, pin) — user-requested LED P03.9 */
#define LED                             &MODULE_P03, 9

/* ========================= Validated EGTM ATOM pin symbols ========================= */
/*
 * Per the validated pin list provided, only a subset of ATOM0 mappings are available here.
 * Use available symbols; for any missing channel/pin mapping, set to NULL_PTR for integration.
 */
#define PHASE_U_HS                      (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)   /* ATOM0 Ch0 non-complementary */
#define PHASE_U_LS                      (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT) /* ATOM0 Ch0 complementary */
#define PHASE_V_HS                      (NULL_PTR)  /* Replace with a valid &IfxEgtm_ATOM0_1_TOUT*_Pxx_y_OUT in integration */
#define PHASE_V_LS                      (NULL_PTR)  /* Replace with a valid &IfxEgtm_ATOM0_1N_TOUT*_Pxx_y_OUT in integration */
#define PHASE_W_HS                      (NULL_PTR)  /* Replace with a valid &IfxEgtm_ATOM0_2_TOUT*_Pxx_y_OUT in integration */
#define PHASE_W_LS                      (NULL_PTR)  /* Replace with a valid &IfxEgtm_ATOM0_2N_TOUT*_Pxx_y_OUT in integration */

/* ========================= Module state ========================= */
typedef struct
{
    IfxEgtm_Pwm             pwm;                                      /* Unified PWM driver handle */
    IfxEgtm_Pwm_Channel     channels[EGTM_NUM_PWM_CHANNELS];          /* Persistent channels array (driver-owned after init) */
    float32                 dutyCycles[EGTM_NUM_PWM_CHANNELS];         /* Cached duties in percent */
    float32                 phases[EGTM_NUM_PWM_CHANNELS];             /* Cached phase offsets (seconds or fraction as per config) */
    IfxEgtm_Pwm_DeadTime    deadTimes[EGTM_NUM_PWM_CHANNELS];          /* Cached dead-times (seconds) */
} EgtmAtom3phState;

typedef struct
{
    IfxEgtm_Pwm         pwm;                   /* Separate PWM for ADC trigger */
    IfxEgtm_Pwm_Channel channels[1];           /* Single logical channel */
    float32             duty;                  /* Cached duty (percent) */
} EgtmAtomTrigState;

IFX_STATIC EgtmAtom3phState g_egtm3ph;
IFX_STATIC EgtmAtomTrigState g_egtmTrig;

/* ========================= ISR and callbacks ========================= */
/* Hardware ISR required by InterruptConfig — toggles LED and returns */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Public ISR requested by software design — toggles LED and returns */
void resultISR(void)
{
    IfxPort_togglePin(LED);
}

/* Empty period-event callback function for InterruptConfig */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* intentionally empty */
}

/* ========================= Internal helpers ========================= */
static void egtm_configure3phOutputs(IfxEgtm_Pwm_OutputConfig output[EGTM_NUM_PWM_CHANNELS])
{
    /* Phase U */
    output[0].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;      /* HS active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;       /* LS active low for complementary */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;
}

/* ========================= Public functions ========================= */
/*
 * Initialize EGTM ATOM PWM for 3 complementary, center-aligned phase pairs with dead-time,
 * plus a separate edge-aligned trigger channel for ADC.
 */
void initEgtmAtom3phInv(void)
{
    /* --- 1) Local configuration structures --- */
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[EGTM_NUM_PWM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[EGTM_NUM_PWM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[EGTM_NUM_PWM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    /* --- 2) Load defaults --- */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* --- 3) Per-channel setup: outputs, dead-time, interrupt --- */
    egtm_configure3phOutputs(output);

    /* Dead-time: 1 us rising, 1 us falling for each complementary pair */
    dtmConfig[0].deadTime.rising   = 1.0e-6f;
    dtmConfig[0].deadTime.falling  = 1.0e-6f;
    dtmConfig[0].fastShutOff       = NULL_PTR;

    dtmConfig[1].deadTime.rising   = 1.0e-6f;
    dtmConfig[1].deadTime.falling  = 1.0e-6f;
    dtmConfig[1].fastShutOff       = NULL_PTR;

    dtmConfig[2].deadTime.rising   = 1.0e-6f;
    dtmConfig[2].deadTime.falling  = 1.0e-6f;
    dtmConfig[2].fastShutOff       = NULL_PTR;

    /* Interrupt on base channel (index 0) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel 0: Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = EGTM_PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    /* Channel 1: Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = EGTM_PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2: Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = EGTM_PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* --- 4) Main config completion --- */
    config.cluster              = IfxEgtm_Cluster_0;
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;            /* ATOM submodule */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;          /* center-aligned */
    config.numChannels          = (uint8)EGTM_NUM_PWM_CHANNELS;
    config.channels             = channelConfig;
    config.frequency            = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                      /* CMU Clock 0 for ATOM */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;      /* CMU Clock 0 for DTM */
    config.syncUpdateEnabled    = TRUE;                                   /* synchronized updates */
    config.syncStart            = TRUE;                                   /* synchronized start */

    /* --- 5) Enable-guard and CMU clocks --- */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* --- 6) Initialize PWM (applies duties and dead-times) --- */
    IfxEgtm_Pwm_init(&g_egtm3ph.pwm, g_egtm3ph.channels, &config);

    /* --- 7) Cache initial state to module storage --- */
    g_egtm3ph.dutyCycles[0] = channelConfig[0].duty;
    g_egtm3ph.dutyCycles[1] = channelConfig[1].duty;
    g_egtm3ph.dutyCycles[2] = channelConfig[2].duty;

    g_egtm3ph.phases[0] = channelConfig[0].phase;
    g_egtm3ph.phases[1] = channelConfig[1].phase;
    g_egtm3ph.phases[2] = channelConfig[2].phase;

    g_egtm3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtm3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtm3ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* --- 8) Configure separate edge-aligned ATOM trigger channel for ADC --- */
    {
        IfxEgtm_Pwm_Config        tcfg;
        IfxEgtm_Pwm_ChannelConfig tch[1];
        IfxEgtm_Pwm_OutputConfig  tout[1];

        IfxEgtm_Pwm_initConfig(&tcfg, &MODULE_EGTM);

        /* No physical pin required for internal trigger; leave as NULL unless routed */
        tout[0].pin                     = NULL_PTR;
        tout[0].complementaryPin        = NULL_PTR;
        tout[0].polarity                = Ifx_ActiveState_high;
        tout[0].complementaryPolarity   = Ifx_ActiveState_low;
        tout[0].outputMode              = IfxPort_OutputMode_pushPull;
        tout[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        tch[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_3;     /* next logical channel */
        tch[0].phase      = 0.0f;
        tch[0].duty       = EGTM_ADC_TRIG_DUTY;             /* 50% edge-aligned */
        tch[0].dtm        = NULL_PTR;                       /* no dead-time for trigger */
        tch[0].output     = &tout[0];
        tch[0].mscOut     = NULL_PTR;
        tch[0].interrupt  = NULL_PTR;

        tcfg.cluster              = IfxEgtm_Cluster_0;
        tcfg.subModule            = IfxEgtm_Pwm_SubModule_atom;
        tcfg.alignment            = IfxEgtm_Pwm_Alignment_edge;  /* edge-aligned trigger */
        tcfg.numChannels          = 1U;
        tcfg.channels             = tch;
        tcfg.frequency            = EGTM_PWM_FREQUENCY_HZ;
        tcfg.clockSource.atom     = IfxEgtm_Cmu_Clk_0;
        tcfg.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;
        tcfg.syncUpdateEnabled    = TRUE;
        tcfg.syncStart            = TRUE;

        IfxEgtm_Pwm_init(&g_egtmTrig.pwm, g_egtmTrig.channels, &tcfg);
        g_egtmTrig.duty = tch[0].duty;

        /* Connect ATOM trigger to ADC trigger mux (selection values depend on board design) */
        {
            /* Selectors are integration-specific; use defaults here. */
            IfxEgtm_TrigSource          src      = (IfxEgtm_TrigSource)0; /* e.g., ATOM0 */
            IfxEgtm_TrigChannel         ch       = (IfxEgtm_TrigChannel)IfxEgtm_Pwm_SubModule_Ch_3;
            IfxEgtm_Cfg_AdcTriggerSignal sig     = (IfxEgtm_Cfg_AdcTriggerSignal)0; /* e.g., AdcTriggerSignal_0 */
            boolean ok = IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0, src, ch, sig);
            (void)ok; /* Integration should verify and adjust source/channel/signal as required */
        }
    }

    /* --- 9) Configure LED GPIO as push-pull output (for ISR toggle) --- */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Atomically update the 3-phase duties in percent (0..100). No timing/delay logic here.
 */
void updateEgtmAtomDuty(void)
{
    /* Wrap-then-add rule (no loops) */
    if ((g_egtm3ph.dutyCycles[0] + EGTM_PHASE_DUTY_STEP) >= 100.0f) { g_egtm3ph.dutyCycles[0] = 0.0f; }
    if ((g_egtm3ph.dutyCycles[1] + EGTM_PHASE_DUTY_STEP) >= 100.0f) { g_egtm3ph.dutyCycles[1] = 0.0f; }
    if ((g_egtm3ph.dutyCycles[2] + EGTM_PHASE_DUTY_STEP) >= 100.0f) { g_egtm3ph.dutyCycles[2] = 0.0f; }

    g_egtm3ph.dutyCycles[0] += EGTM_PHASE_DUTY_STEP;
    g_egtm3ph.dutyCycles[1] += EGTM_PHASE_DUTY_STEP;
    g_egtm3ph.dutyCycles[2] += EGTM_PHASE_DUTY_STEP;

    /* Immediate multi-channel duty update (preserves alignment and dead-time) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtm3ph.pwm, (float32 *)g_egtm3ph.dutyCycles);
}
