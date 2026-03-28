/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief 3-phase complementary center-aligned PWM driver on GTM TOM using unified IfxGtm_Pwm.
 *
 * Implementation notes:
 * - Uses TOM backend in Cluster_1, 20 kHz, center-aligned, complementary outputs with 1 us dead-time.
 * - Synchronized start and synchronized shadow updates enabled.
 * - Initial duties: U=25%, V=50%, W=75%.
 * - Immediate duty update API is used for runtime changes.
 * - GTM enable/CMU clock configuration is guarded to run only if GTM is not enabled yet.
 * - Provides a no-op period-event callback and a minimal ISR toggling LED P13.0.
 * - Watchdog control is NOT handled here (must be done only in CpuX_Main.c as per AURIX standard).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define NUM_OF_CHANNELS             (3u)
#define PWM_FREQUENCY_HZ            (20000.0f)
#define ISR_PRIORITY_ATOM           (20)

/* Initial duty cycle percentages (0..100) */
#define PHASE_U_DUTY_INIT           (25.0f)
#define PHASE_V_DUTY_INIT           (50.0f)
#define PHASE_W_DUTY_INIT           (75.0f)

/* Duty cycle update step in percent */
#define PHASE_DUTY_STEP             (10.0f)

/* Dead-time in seconds (1 us) */
#define DTM_DEADTIME_RISING_S       (1.0e-6f)
#define DTM_DEADTIME_FALLING_S      (1.0e-6f)

/* LED: P13.0 (used by ISR to toggle) */
#define LED                         &MODULE_P13, 0

/* =============================
 * PWM Pin Mapping Macros
 * =============================
 * User-requested pins (KIT_A2G_TC387_5V_TFT):
 *  - U: P02.0 (HS), P02.7 (LS)
 *  - V: P02.1 (HS), P02.4 (LS)
 *  - W: P02.2 (HS), P02.5 (LS)
 *
 * No validated TOUT symbols were provided in the context for these pads.
 * Define as NULL_PTR placeholders to be replaced during integration with the
 * appropriate IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols from the device PinMap.
 */
#define PHASE_U_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_<ch>_TOUT<PIN>_P02_0_OUT */
#define PHASE_U_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_<chN>_TOUT<PIN>_P02_7_OUT */
#define PHASE_V_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_<ch>_TOUT<PIN>_P02_1_OUT */
#define PHASE_V_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_<chN>_TOUT<PIN>_P02_4_OUT */
#define PHASE_W_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_<ch>_TOUT<PIN>_P02_2_OUT */
#define PHASE_W_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_<chN>_TOUT<PIN>_P02_5_OUT */

/* =============================
 * Module State
 * ============================= */
typedef struct
{
    IfxGtm_Pwm             pwm;                               /* PWM driver handle */
    IfxGtm_Pwm_Channel     channels[NUM_OF_CHANNELS];         /* Persistent channels array */
    float32                dutyCycles[NUM_OF_CHANNELS];       /* Duty values in percent (0..100) */
    float32                phases[NUM_OF_CHANNELS];           /* Phase offsets in percent/deg (SW-owned) */
    IfxGtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];        /* Dead-time values per channel */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_tom3phState; /* Persistent module state */

/* =============================
 * ISR and Callback Declarations
 * ============================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/**
 * @brief GTM ATOM ISR (rate TBD by user). Body intentionally minimal.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/**
 * @brief Period event callback for IfxGtm_Pwm (no-op as required).
 * @param data user data pointer (unused)
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Public API Implementations
 * ============================= */

/**
 * @brief Initialize a 3-channel complementary, center-aligned PWM on TOM1, Cluster_1 at 20 kHz.
 *
 * Behavior:
 * - Declares all configuration structures locally (main config, interrupt config, channels, outputs, DTM).
 * - Initializes config with defaults, sets TOM backend, Cluster_1, 20 kHz, center-aligned, complementary outputs.
 * - Enables synchronized start and synchronized shadow updates.
 * - Sets rising/falling dead-times to 1 us for each complementary pair.
 * - Assigns output pins for U, V, W (HS/LS) and initial duties 25/50/75%.
 * - InterruptConfig (period event) assigned only to base channel (index 0): CPU0 provider, priority 20.
 * - Applies enable guard: if GTM is not enabled, enable GTM and configure CMU clocks (GCLK and CLK0), then enable FXCLK and CLK0.
 * - Calls IfxGtm_Pwm_init once with persistent channels array from module state.
 * - Copies initial duty and dead-time values into persistent module state.
 * - Configures LED GPIO pin (P13.0) as output for ISR toggling.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare configuration structures locally */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   interruptConfig;
    IfxGtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: HS/LS pins, polarity, drive */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;      /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;       /* LS active LOW  */
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

    /* 4) Dead-time configuration: 1 us for rising and falling per channel */
    dtmConfig[0].deadTime.rising = DTM_DEADTIME_RISING_S;
    dtmConfig[0].deadTime.falling = DTM_DEADTIME_FALLING_S;
    dtmConfig[1].deadTime.rising = DTM_DEADTIME_RISING_S;
    dtmConfig[1].deadTime.falling = DTM_DEADTIME_FALLING_S;
    dtmConfig[2].deadTime.rising = DTM_DEADTIME_RISING_S;
    dtmConfig[2].deadTime.falling = DTM_DEADTIME_FALLING_S;

    /* 5) Interrupt configuration: period event on base channel only */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;  /* Period ISR mode */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;             /* CPU0 */
    interruptConfig.priority    = ISR_PRIORITY_ATOM;           /* Priority 20 */
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;  /* Callback */
    interruptConfig.dutyEvent   = NULL_PTR;                    /* Not used */

    /* 6) Channel configurations (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;            /* Base channel interrupt */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;                    /* No separate IRQ */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;                    /* No separate IRQ */

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                        /* Cluster 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;                /* TOM backend */
    config.alignment            = IfxGtm_Pwm_Alignment_center;             /* Center-aligned */
    config.syncStart            = TRUE;                                    /* Synchronous start */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                         /* 20 kHz */
    config.clockSource.atom     = (uint32)IfxGtm_Cmu_Fxclk_0;               /* CMU FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;         /* DTM clock */
    config.syncUpdateEnabled    = TRUE;                                     /* Shadow updates synchronized */

    /* 8) Enable guard: enable GTM and CMU clocks only if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent channels array */
    IfxGtm_Pwm_init(&g_tom3phState.pwm, &g_tom3phState.channels[0], &config);

    /* 10) Store initial duties, phases, and dead-times into module state */
    g_tom3phState.dutyCycles[0] = channelConfig[0].duty;
    g_tom3phState.dutyCycles[1] = channelConfig[1].duty;
    g_tom3phState.dutyCycles[2] = channelConfig[2].duty;

    g_tom3phState.phases[0] = channelConfig[0].phase;
    g_tom3phState.phases[1] = channelConfig[1].phase;
    g_tom3phState.phases[2] = channelConfig[2].phase;

    g_tom3phState.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phState.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO (P13.0) as output for ISR toggling */
    IfxPort_setPinModeOutput(&MODULE_P13, 0, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Update the three phase duties by +10% with wrap-to-0-then-add-step, then apply immediately.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap-to-0-then-add-step behavior (explicit per-channel, no loop) */
    if ((g_tom3phState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phState.dutyCycles[0] = 0.0f; }
    if ((g_tom3phState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phState.dutyCycles[1] = 0.0f; }
    if ((g_tom3phState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phState.dutyCycles[2] = 0.0f; }

    g_tom3phState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply updates immediately to all three complementary pairs atomically */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phState.pwm, (float32 *)g_tom3phState.dutyCycles);
}
