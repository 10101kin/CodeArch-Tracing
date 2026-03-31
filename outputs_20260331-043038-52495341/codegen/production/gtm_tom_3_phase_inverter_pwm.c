/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: 3-phase complementary PWM using IfxGtm_Pwm on TOM
 *
 * Initialization strictly follows iLLD patterns provided in the prompt.
 * No watchdog handling here (must remain in CpuX_Main.c only).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= User-confirmed migration values ========================= */
#define NUM_OF_CHANNELS                (3u)
#define PWM_FREQUENCY_HZ               (20000.0f)      /* 20 kHz */
#define PHASE_U_DUTY_INIT_PERCENT      (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT      (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT      (75.0f)
#define PHASE_DUTY_STEP_PERCENT        (10.0f)
#define DUTY_MIN_PERCENT               (10.0f)
#define DUTY_MAX_PERCENT               (90.0f)
#define PWM_DEAD_TIME_SECONDS          (1.0e-6f)       /* 1.0 us (user-confirmed) */
#define PWM_MIN_PULSE_TIME_SECONDS     (1.0e-6f)       /* 1.0 us */

/* ISR priority macro (used by IFX_INTERRUPT and InterruptConfig.priority) */
#define ISR_PRIORITY_ATOM              (3u)

/* LED/debug pin (compound macro: port, pin) */
#define LED                            &MODULE_P13, 0

/* ========================= TOM TOUT pin assignments (user-specified) ========================= */
/* Use the exact TOUT symbols from the hardware context/pinmap */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)  /* Phase U high-side */
#define PHASE_U_LS   (&IfxGtm_TOM1_5_TOUT11_P00_2_OUT)  /* Phase U low-side  */
#define PHASE_V_HS   (&IfxGtm_TOM1_1N_TOUT14_P00_5_OUT) /* Phase V high-side */
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)  /* Phase V low-side  */
#define PHASE_W_HS   (&IfxGtm_TOM1_3N_TOUT16_P00_7_OUT) /* Phase W high-side */
#define PHASE_W_LS   (&IfxGtm_TOM1_2N_TOUT15_P00_6_OUT) /* Phase W low-side  */

/* ========================= Module state ========================= */
typedef struct
{
    IfxGtm_Pwm                pwm;                              /* Driver handle */
    IfxGtm_Pwm_Channel        channels[NUM_OF_CHANNELS];        /* Persistent channels array */
    float32                   dutyCycles[NUM_OF_CHANNELS];      /* Duty in percent */
    float32                   phases[NUM_OF_CHANNELS];          /* Phase in percent/deg-equivalent */
    IfxGtm_Pwm_DeadTime       deadTimes[NUM_OF_CHANNELS];       /* Dead-time per pair */
} GtmTom3Phase_State;

IFX_STATIC GtmTom3Phase_State g_gtmTom3phState;                 /* Persistent module state */

/* ========================= ISR and callback declarations ========================= */
/* ISR routed by the high-level PWM driver via TOM/ATOM interrupt line */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/* Period event callback (assigned in InterruptConfig) */
void IfxGtm_periodEventFunction(void *data);

/* ========================= ISR and callback implementations ========================= */
void interruptGtmAtom(void)
{
    /* Minimal ISR body: toggle LED/debug pin */
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty callback */
}

/* ========================= Public API implementations ========================= */
/**
 * Initialize a persistent 3-channel complementary PWM on TOM using IfxGtm_Pwm.
 * Follows the mandatory initialization patterns and clock enable guard.
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Populate defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Per-channel output configuration (complementary pairs) */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;  /* High-side */
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;  /* Low-side  */
    output[0].polarity                = Ifx_ActiveState_high;              /* HS active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;               /* LS active low  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Per-channel DTM settings and channel configuration */
    /* Common dead-time (rising/falling) per complementary pair */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_SECONDS;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_SECONDS;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_SECONDS;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_SECONDS;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_SECONDS;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_SECONDS;

    /* Interrupt only on base channel (index 0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Logical timer channel indices: 0, 1, 2 (base = 0) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;    /* base period event */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;            /* no ISR on this channel */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;            /* no ISR on this channel */

    /* 6) Complete main configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;   /* center-aligned */
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;            /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 7) Enable-guard: enable GTM and configure CMU clocks only if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phState.pwm, &g_gtmTom3phState.channels[0], &config);

    /* 9) Store initial duties, phases, and dead-times into module state */
    g_gtmTom3phState.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phState.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phState.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phState.phases[0] = channelConfig[0].phase;
    g_gtmTom3phState.phases[1] = channelConfig[1].phase;
    g_gtmTom3phState.phases[2] = channelConfig[2].phase;

    g_gtmTom3phState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure LED/debug GPIO as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duties and apply immediately.
 * Adds PHASE_DUTY_STEP_PERCENT to each duty and wraps to DUTY_MIN_PERCENT when >= DUTY_MAX_PERCENT.
 */
void GTM_TOM_3_Phase_Inverter_PWM_updateDuties(void)
{
    /* 1..4) Update persistent duty cycles with wrap */
    g_gtmTom3phState.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    if (g_gtmTom3phState.dutyCycles[0] >= DUTY_MAX_PERCENT)
    {
        g_gtmTom3phState.dutyCycles[0] = DUTY_MIN_PERCENT;
    }

    g_gtmTom3phState.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    if (g_gtmTom3phState.dutyCycles[1] >= DUTY_MAX_PERCENT)
    {
        g_gtmTom3phState.dutyCycles[1] = DUTY_MIN_PERCENT;
    }

    g_gtmTom3phState.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;
    if (g_gtmTom3phState.dutyCycles[2] >= DUTY_MAX_PERCENT)
    {
        g_gtmTom3phState.dutyCycles[2] = DUTY_MIN_PERCENT;
    }

    /* 5) Apply immediately to all complementary pairs */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phState.pwm, (float32 *)g_gtmTom3phState.dutyCycles);
}
