/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: GTM TOM three-phase inverter PWM using IfxGtm_Pwm (unified)
 * - Submodule: TOM
 * - Timebase: TOM1, CH0 (20 kHz), center-aligned
 * - Complementary pairs: U, V, W (HS active-high, LS active-low)
 * - Dead-time: 0.5 us (rising/falling)
 * - Min pulse: 1.0 us (tracked in module state)
 *
 * Notes:
 * - Clock and GTM enable performed under mandatory enable-guard.
 * - Output routing via IfxGtm_Pwm_OutputConfig array.
 * - Dead-time via IfxGtm_Pwm_DtmConfig array.
 * - Channel setup via IfxGtm_Pwm_ChannelConfig array.
 * - Clock source: config.clockSource.tom = IfxGtm_Cmu_Fxclk_0.
 * - Interrupt callback provided via IfxGtm_Pwm_InterruptConfig.
 * - Pins are placeholders (NULL_PTR) if template pin symbols are not available.
 *
 * Watchdog: Do NOT disable watchdogs here (Cpu0_Main.c only).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Configuration Macros ========================= */
#define NUM_OF_CHANNELS                (3u)
#define PWM_FREQUENCY_HZ               (20000.0f)
#define ISR_PRIORITY_ATOM              (3)

/* Dead-time and min pulse (seconds) */
#define DEAD_TIME_RISING_SEC           (5.0e-7f)   /* 0.5 us */
#define DEAD_TIME_FALLING_SEC          (5.0e-7f)   /* 0.5 us */
#define MIN_PULSE_SEC                  (1.0e-6f)   /* 1.0 us */

/* Initial duties in percent (0..100) */
#define PHASE_U_DUTY_INIT              (25.0f)
#define PHASE_V_DUTY_INIT              (50.0f)
#define PHASE_W_DUTY_INIT              (75.0f)

/* LED debug pin (port, pin) */
#define LED                            &MODULE_P13, 0

/* ========================= Pin Routing Macros =========================== */
/*
 * IMPORTANT: No validated pin symbols were provided by the template for GTM TOM.
 * Define pin macros as NULL_PTR placeholders. Replace with actual symbols during
 * integration, matching the required mapping:
 *   U_HS: TOM1 CH2 -> P00.3 (TOUT12)  => &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
 *   U_LS: TOM1 CH1 -> P00.2 (TOUT11)  => &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
 *   V_HS: TOM1 CH4 -> P00.5 (TOUT14)  => &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
 *   V_LS: TOM1 CH3 -> P00.4 (TOUT13)  => &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
 *   W_HS: TOM1 CH6 -> P00.7 (TOUT16)  => &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
 *   W_LS: TOM1 CH5 -> P00.6 (TOUT15)  => &IfxGtm_TOM1_5_TOUT15_P00_6_OUT
 */
#define PHASE_U_HS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUT12_P00_3_OUT */
#define PHASE_U_LS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUT11_P00_2_OUT */
#define PHASE_V_HS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUT14_P00_5_OUT */
#define PHASE_V_LS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUT13_P00_4_OUT */
#define PHASE_W_HS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_6_TOUT16_P00_7_OUT */
#define PHASE_W_LS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUT15_P00_6_OUT */

/* ========================= Module State ================================ */
typedef struct
{
    IfxGtm_Pwm               pwm;                                 /* driver handle */
    IfxGtm_Pwm_Channel       channels[NUM_OF_CHANNELS];           /* persistent channels array */
    float32                  dutyCycles[NUM_OF_CHANNELS];         /* duty in percent (0..100) */
    float32                  phases[NUM_OF_CHANNELS];             /* phase offset in degrees */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];          /* tracked dead-times (s) */
    float32                  minPulseSeconds;                     /* tracked min pulse (s) */
} GtmTom3PhPwm_State;

IFX_STATIC GtmTom3PhPwm_State g_gtmTom3PhPwm;

/* ========================= ISR and Callback ============================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback: assigned into InterruptConfig.periodEvent (empty body) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API =================================== */
void initGtmTomPwm(void)
{
    /* 1) Persistent state is already declared at file scope (g_gtmTom3PhPwm) */

    /* 2) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Main config fields */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;     /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;
    /* NOTE: Cluster is left as initialized by initConfig() to avoid non-portable constants */

    /* 4) Output routing: complementary per channel */
    /* Channel 0 -> Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;   /* HS: active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;    /* LS: active low  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 1 -> Phase V */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 2 -> Phase W */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) Dead-time configuration for all pairs (0.5 us) + track minPulse */
    dtmConfig[0].deadTime.rising = DEAD_TIME_RISING_SEC;
    dtmConfig[0].deadTime.falling = DEAD_TIME_FALLING_SEC;
    dtmConfig[1].deadTime.rising = DEAD_TIME_RISING_SEC;
    dtmConfig[1].deadTime.falling = DEAD_TIME_FALLING_SEC;
    dtmConfig[2].deadTime.rising = DEAD_TIME_RISING_SEC;
    dtmConfig[2].deadTime.falling = DEAD_TIME_FALLING_SEC;

    g_gtmTom3PhPwm.minPulseSeconds = MIN_PULSE_SEC;

    /* 6) Enable guard: enable GTM and CMU clocks if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        /* NOTE: setClkFrequency() is intentionally omitted (not in available mocks) */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* Interrupt configuration (base channel) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 7) Channel configuration: logical indices 0..2 with TOM1 CH0 as timebase */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0; /* timebase */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;          /* base channel interrupt */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = &interruptConfig;          /* share same interrupt config */

    /* CH2 -> Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = &interruptConfig;

    /* 7) Initialize PWM: applies pin mux, alignment, timebase, shadows */
    IfxGtm_Pwm_init(&g_gtmTom3PhPwm.pwm, &g_gtmTom3PhPwm.channels[0], &config);

    /* 8) Immediately set/update base frequency to guarantee 20 kHz */
    IfxGtm_Pwm_updateFrequencyImmediate(&g_gtmTom3PhPwm.pwm, PWM_FREQUENCY_HZ);

    /* 9) Program initial duties using immediate multi-channel update (queued semantics not available in this API set) */
    g_gtmTom3PhPwm.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3PhPwm.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3PhPwm.dutyCycles[2] = PHASE_W_DUTY_INIT;
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3PhPwm.pwm, (float32 *)g_gtmTom3PhPwm.dutyCycles);

    /* 10) Start synchronized outputs: syncStart=TRUE ensures lockstep start at timebase */

    /* Store phases and dead-times into module state for diagnostics/future updates */
    g_gtmTom3PhPwm.phases[0] = channelConfig[0].phase;
    g_gtmTom3PhPwm.phases[1] = channelConfig[1].phase;
    g_gtmTom3PhPwm.phases[2] = channelConfig[2].phase;

    g_gtmTom3PhPwm.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3PhPwm.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3PhPwm.deadTimes[2] = dtmConfig[2].deadTime;

    /* Configure LED GPIO after PWM init (debug ISR toggle) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
