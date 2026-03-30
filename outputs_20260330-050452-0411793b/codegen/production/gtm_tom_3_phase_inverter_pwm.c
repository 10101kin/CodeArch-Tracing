/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for 3-phase complementary center-aligned PWM using IfxGtm_Pwm (TOM submodule)
 * Migration target: Replace IfxGtm_Tom_PwmHl with IfxGtm_Pwm
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Configuration Macros ========================= */
#define GTM_TOM_NUM_CHANNELS           (3u)
#define GTM_TOM_PWM_FREQUENCY_HZ       (20000.0f)    /* 20 kHz */
#define GTM_TOM_DUTY_INIT_U            (25.0f)       /* percent */
#define GTM_TOM_DUTY_INIT_V            (50.0f)       /* percent */
#define GTM_TOM_DUTY_INIT_W            (75.0f)       /* percent */
#define GTM_TOM_DUTY_STEP              (10.0f)       /* percent */

/* Interrupt priority for PWM period event (handled by HLD routing) */
#define ISR_PRIORITY_ATOM              (20)

/* Debug LED pin (port, pin) compound macro */
#define LED                            (&MODULE_P13), (0u)

/*
 * Pin routing placeholders: replace NULL_PTR with valid TOUT mappings for TOM1 on TC38x LFBGA516
 * User-requested pins:
 *  - Phase U: P02.0 (HS) / P02.7 (LS)
 *  - Phase V: P02.1 (HS) / P02.4 (LS)
 *  - Phase W: P02.2 (HS) / P02.5 (LS)
 * Example (to be validated on the target board & PinMap):
 *   #define PHASE_U_HS  (&IfxGtm_TOM1_0_TOUTxx_P02_0_OUT)
 */
#define PHASE_U_HS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTx_P02_0_OUT */
#define PHASE_U_LS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTx_P02_7_OUT */
#define PHASE_V_HS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTx_P02_1_OUT */
#define PHASE_V_LS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTx_P02_4_OUT */
#define PHASE_W_HS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTx_P02_2_OUT */
#define PHASE_W_LS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTx_P02_5_OUT */

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm               pwm;                               /* PWM driver handle */
    IfxGtm_Pwm_Channel       channels[GTM_TOM_NUM_CHANNELS];    /* Channel handles (persistent) */
    float32                  dutyCycles[GTM_TOM_NUM_CHANNELS];  /* Duty in percent [U,V,W] */
    float32                  phases[GTM_TOM_NUM_CHANNELS];      /* Phase offset in percent */
    IfxGtm_Pwm_DeadTime      deadTimes[GTM_TOM_NUM_CHANNELS];   /* Dead-time per phase (s) */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;

/* ========================= ISR and Callback ========================= */

/*
 * ISR: Minimal body per best practices. Priority must match InterruptConfig.priority.
 * The high-level driver routes the period event; we only toggle a debug pin here.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Period-event callback hook assigned via IfxGtm_Pwm_InterruptConfig.
 * Must have empty body; do not perform any processing here.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */

/**
 * Initialize a 3-phase, center-aligned complementary PWM using GTM TOM submodule.
 * - Frequency: 20 kHz
 * - Dead-time: 1 us on both edges
 * - Phases: U/V/W complementary pairs
 * - Sync start and sync update enabled
 * - Clock source: FXCLK0 for TOM
 * - Period-event callback assigned via InterruptConfig (attached to base channel index 0)
 * - GTM enable guard applied (enable + CMU setup only if module is not yet enabled)
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_NUM_CHANNELS];
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Initialize config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: three complementary pairs (U,V,W) */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;   /* HS active-high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;    /* LS active-low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: dead-time = 1 us on both edges */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Base channel interrupt configuration (period callback) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical channel indices 0..2) */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = GTM_TOM_DUTY_INIT_U;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* attach only to base channel */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = GTM_TOM_DUTY_INIT_V;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;         /* only base channel has interrupt */

    /* CH2 -> Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = GTM_TOM_DUTY_INIT_W;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                 /* TOM1 cluster index 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;         /* Use TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;      /* Center-aligned */
    config.syncStart            = TRUE;                             /* Synchronized start */
    config.numChannels          = (uint8)GTM_TOM_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_PWM_FREQUENCY_HZ;         /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;               /* TOM -> FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock */
    config.syncUpdateEnabled    = TRUE;                             /* Sync update */

    /* 8) Enable guard: enable and set CMU clocks only if GTM is not enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM driver (channels handle array must be persistent) */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 10) Store initial state into persistent module state */
    g_gtmTom3phInv.dutyCycles[0] = GTM_TOM_DUTY_INIT_U;
    g_gtmTom3phInv.dutyCycles[1] = GTM_TOM_DUTY_INIT_V;
    g_gtmTom3phInv.dutyCycles[2] = GTM_TOM_DUTY_INIT_W;

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED/debug GPIO as output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duties (U,V,W) in percent using a step-and-wrap pattern, then
 * apply immediately via the PWM driver. No delays or scheduling here.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap-to-0 then add step (no for-loop; explicit per-channel operations) */
    if ((g_gtmTom3phInv.dutyCycles[0] + GTM_TOM_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + GTM_TOM_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + GTM_TOM_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += GTM_TOM_DUTY_STEP; /* U */
    g_gtmTom3phInv.dutyCycles[1] += GTM_TOM_DUTY_STEP; /* V */
    g_gtmTom3phInv.dutyCycles[2] += GTM_TOM_DUTY_STEP; /* W */

    /* Apply updates immediately in percent */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);
}
