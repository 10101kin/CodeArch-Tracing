/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for 3-phase complementary center-aligned PWM using IfxGtm_Pwm on TOM backend (TC3xx).
 *
 * Requirements implemented:
 *  - TOM1, Cluster 1, 20 kHz, CMU FXCLK0
 *  - Complementary outputs with 1 us rising/falling dead-time
 *  - Center-aligned PWM, synchronized start and synchronized shadow updates
 *  - Initial duties: 25%, 50%, 75%
 *  - Update API: +10% step with wrap-to-0-then-add-step, applied immediately
 *  - Period-event callback (no-op) and ATOM ISR toggling LED (P13.0)
 *
 * Notes:
 *  - Watchdog disable must not be placed here (only in CpuX main files).
 *  - Pin TOUT symbols for the requested P02.x pads are not provided by the validated list in this context.
 *    The pin macros below are therefore NULL_PTR placeholders and must be replaced during integration
 *    with the correct &IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols matching the board/package.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ============================= Macros and constants ============================= */

/* Channel count */
#define PWM3PH_NUM_CHANNELS             (3u)

/* PWM timing */
#define PWM3PH_FREQUENCY_HZ             (20000.0f)

/* ISR priority for GTM interrupt routing (period event) and ISR declaration */
#define ISR_PRIORITY_ATOM               (20u)

/* GTM cluster selection (Cluster 1) */
#define GTM_CLUSTER                     (1u)

/* LED pin (P13.0): compound macro expands to two arguments (port, pin) */
#define LED                              &MODULE_P13, 0u

/* Phase duty initial percentages and update step (percent units) */
#define PHASE_U_DUTY_INIT               (25.0f)
#define PHASE_V_DUTY_INIT               (50.0f)
#define PHASE_W_DUTY_INIT               (75.0f)
#define PHASE_DUTY_STEP                 (10.0f)

/*
 * Pin routing macros: placeholders because validated TOUT symbols are not provided in this context.
 * Replace NULL_PTR with the proper &IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols that map to:
 *   U: P02.0 (high), P02.7 (low)
 *   V: P02.1 (high), P02.4 (low)
 *   W: P02.2 (high), P02.5 (low)
 */
#define PHASE_U_HS                       (NULL_PTR) /* Replace with &IfxGtm_TOM1_<ch>_TOUT<PAD>_P02_0_OUT */
#define PHASE_U_LS                       (NULL_PTR) /* Replace with &IfxGtm_TOM1_<chN>_TOUT<PAD>_P02_7_OUT */
#define PHASE_V_HS                       (NULL_PTR) /* Replace with &IfxGtm_TOM1_<ch>_TOUT<PAD>_P02_1_OUT */
#define PHASE_V_LS                       (NULL_PTR) /* Replace with &IfxGtm_TOM1_<chN>_TOUT<PAD>_P02_4_OUT */
#define PHASE_W_HS                       (NULL_PTR) /* Replace with &IfxGtm_TOM1_<ch>_TOUT<PAD>_P02_2_OUT */
#define PHASE_W_LS                       (NULL_PTR) /* Replace with &IfxGtm_TOM1_<chN>_TOUT<PAD>_P02_5_OUT */

/* ============================= Module state ============================= */

/**
 * Internal driver state for 3-phase inverter PWM.
 */
typedef struct
{
    IfxGtm_Pwm              pwm;                                 /* High-level PWM driver handle */
    IfxGtm_Pwm_Channel      channels[PWM3PH_NUM_CHANNELS];       /* Persistent channel instances (driver stores pointer) */
    float32                 dutyCycles[PWM3PH_NUM_CHANNELS];     /* Duty in percent [0..100] per logical channel */
    float32                 phases[PWM3PH_NUM_CHANNELS];         /* Phase in percent/deg if used (initialized to 0) */
    IfxGtm_Pwm_DeadTime     deadTimes[PWM3PH_NUM_CHANNELS];      /* Dead-time settings per channel */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_3phState;

/* ============================= ISR and callback ============================= */

/*
 * ATOM/TOM ISR declaration and implementation.
 * The body must only toggle the debug LED; the high-level driver handles routing.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Period event callback used by the high-level driver. Must be a no-op.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================= Public API ============================= */

/**
 * Initialize a 3-channel complementary, center-aligned PWM using the unified IfxGtm_Pwm driver on the TOM backend.
 *
 * Behavior:
 *  - Prepares all configuration structures locally (config, outputs, DTM, interrupt, channelConfig)
 *  - Uses TOM backend on Cluster 1, 20 kHz, center-aligned, complementary outputs
 *  - Synchronized start and synchronized shadow updates enabled
 *  - Dead-time: 1 us rising, 1 us falling on each complementary pair
 *  - Channels 0..2 map to U, V, W with initial duties 25%, 50%, 75%
 *  - InterruptConfig populated and assigned to base channel only; others have NULL_PTR
 *  - Enable guard: if GTM not enabled, enable and configure CMU clocks inside the guard
 *  - Calls high-level IfxGtm_Pwm_init once, passing persistent channels array from module state
 *  - Stores initial duties and dead-times into module state for future updates
 *  - Configures LED GPIO (P13.0) as output for ISR toggle
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[PWM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[PWM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[PWM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Initialize main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for complementary pairs (high-side + low-side) */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;            /* HS active-high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;             /* LS active-low  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (1 us rising/falling) */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 5) Interrupt configuration (period event, CPU0 provider, priority 20) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2) */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* base channel carries interrupt */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;          /* only base channel has interrupt */

    /* CH2 -> Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = (IfxGtm_Cluster)GTM_CLUSTER;        /* Cluster 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;           /* TOM backend */
    config.alignment            = IfxGtm_Pwm_Alignment_center;        /* Center-aligned */
    config.syncStart            = TRUE;                                /* Synchronized start */
    config.numChannels          = (uint8)PWM3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM3PH_FREQUENCY_HZ;                 /* 20 kHz */
    config.clockSource.atom     = (uint32)IfxGtm_Cmu_Fxclk_0;          /* Use FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;    /* DTM clock */
    config.syncUpdateEnabled    = TRUE;                                /* Synchronized shadow update */

    /* 8) Enable guard: enable GTM and CMU clocks only if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the high-level PWM driver (passes persistent channels array from module state) */
    IfxGtm_Pwm_init(&g_3phState.pwm, &g_3phState.channels[0], &config);

    /* 10) Store initial duties, phases, and dead-times into persistent state */
    g_3phState.dutyCycles[0] = channelConfig[0].duty;
    g_3phState.dutyCycles[1] = channelConfig[1].duty;
    g_3phState.dutyCycles[2] = channelConfig[2].duty;

    g_3phState.phases[0] = channelConfig[0].phase;
    g_3phState.phases[1] = channelConfig[1].phase;
    g_3phState.phases[2] = channelConfig[2].phase;

    g_3phState.deadTimes[0] = dtmConfig[0].deadTime;
    g_3phState.deadTimes[1] = dtmConfig[1].deadTime;
    g_3phState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as output for ISR toggling */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duties in percent using a fixed +10% step with wrap-to-0-then-add-step behavior.
 * Applies the change immediately via the high-level PWM driver (atomic across channels).
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: check then unconditional add (three separate if-blocks, no loops) */
    if ((g_3phState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_3phState.dutyCycles[0] = 0.0f; }
    if ((g_3phState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_3phState.dutyCycles[1] = 0.0f; }
    if ((g_3phState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_3phState.dutyCycles[2] = 0.0f; }

    g_3phState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_3phState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_3phState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately (percent 0..100) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_3phState.pwm, (float32*)g_3phState.dutyCycles);
}
