/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm
 * - TC3xx family
 * - Submodule: TOM
 * - Center-aligned at 20 kHz
 * - Single-ended HS pins (U/V/W = P00.3 / P00.5 / P00.7)
 * - Synchronous start and synchronous updates enabled
 *
 * Notes:
 *  - Watchdogs are NOT touched here (Cpu0_Main.c only per project standard)
 *  - No ISR-based driver callbacks are configured (per behavioral spec)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ========================= Macros and Configuration Constants ========================= */

/* Number of logical PWM channels (U, V, W) */
#define GTM_TOM_3PH_NUM_CHANNELS         (3u)

/* PWM switching frequency (Hz) */
#define GTM_TOM_PWM_FREQUENCY_HZ         (20000.0f)

/* Interrupt priority placeholder (ISR not wired to driver in this module) */
#define ISR_PRIORITY_ATOM                (3)

/* Phase initial duties (percent) */
#define PHASE_U_DUTY_INIT                (25.0f)
#define PHASE_V_DUTY_INIT                (50.0f)
#define PHASE_W_DUTY_INIT                (75.0f)

/* Duty modulation step and bounds (percent) */
#define PHASE_DUTY_STEP                  (10.0f)
#define PHASE_DUTY_MIN                   (10.0f)
#define PHASE_DUTY_MAX                   (90.0f)

/* Dead-time and minimum pulse time (seconds) from migration values */
#define GTM_TOM_PWM_DEAD_TIME_S          (1.0e-6f)
#define GTM_TOM_PWM_MIN_PULSE_TIME_S     (1.0e-6f)

/* LED debug pin: port and pin as a compound macro */
#define LED                               &MODULE_P13, 0

/* Validated TOM1 HS pins (single-ended) */
#define PHASE_U_HS                       &IfxGtm_TOM1_2_TOUT12_P00_3_OUT  /* U: TOM1 CH2 → P00.3 */
#define PHASE_V_HS                       &IfxGtm_TOM1_4_TOUT14_P00_5_OUT  /* V: TOM1 CH4 → P00.5 */
#define PHASE_W_HS                       &IfxGtm_TOM1_6_TOUT16_P00_7_OUT  /* W: TOM1 CH6 → P00.7 */

/* ========================= Internal ISR and Callback Declarations ========================= */

/* ISR declaration (not linked to driver interrupts in this module; kept for debug visibility) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/* Period-event callback symbol (empty body as required) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR implementation: minimal body per guideline */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                            /* PWM driver handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM_3PH_NUM_CHANNELS];             /* Persistent channels storage */
    float32                 dutyCycles[GTM_TOM_3PH_NUM_CHANNELS];           /* Duty cycles in percent */
    float32                 phases[GTM_TOM_3PH_NUM_CHANNELS];               /* Phase offsets in percent */
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM_3PH_NUM_CHANNELS];            /* Dead-time values (s) */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInv;

/* ========================= Public API Implementations ========================= */

/*
 * Initialize 3-phase center-aligned PWM on TOM1 using IfxGtm_Pwm.
 * - Localize all configs, then apply hardware settings within GTM enable-guard.
 * - Single-ended outputs on TOM1 HS pins (U/V/W).
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_3PH_NUM_CHANNELS];

    /* 2) Load defaults into main config */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Per-channel output binding and base attributes (single-ended HS only) */
    /* U phase (index 0) */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = NULL_PTR;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase (index 1) */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = NULL_PTR;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase (index 2) */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = NULL_PTR;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM configuration: use migration-provided dead-time even in single-ended mode */
    dtmConfig[0].deadTime.rising = GTM_TOM_PWM_DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = GTM_TOM_PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.rising = GTM_TOM_PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = GTM_TOM_PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.rising = GTM_TOM_PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = GTM_TOM_PWM_DEAD_TIME_S;

    /* 6) Channel configuration (logical ordinal indices 0..2) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = NULL_PTR;  /* No ISR used per behavior */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;  /* No ISR used per behavior */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;  /* No ISR used per behavior */

    /* 4) Main configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM_3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0; /* TOM uses FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;

    /* 5) GTM enable-guard and clock setup (MANDATORY pattern) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent handle and channels */
    IfxGtm_Pwm_init(&g_tom3phInv.pwm, &g_tom3phInv.channels[0], &config);

    /* 7) Initialize persistent runtime state from channel configuration */
    g_tom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_tom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_tom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_tom3phInv.phases[0] = channelConfig[0].phase;
    g_tom3phInv.phases[1] = channelConfig[1].phase;
    g_tom3phInv.phases[2] = channelConfig[2].phase;

    g_tom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* Configure LED pin for debug (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update the three phase duty cycles and apply them synchronously.
 * Algorithm per design:
 *  - Add +10.0 percent to each phase duty
 *  - If a value becomes >= 90.0, wrap it to 10.0
 *  - Apply with immediate multi-channel update for coherent shadow transfer
 */
void updateGtmTom3phInvDuty(void)
{
    /* Update U phase */
    g_tom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    if (g_tom3phInv.dutyCycles[0] >= PHASE_DUTY_MAX)
    {
        g_tom3phInv.dutyCycles[0] = PHASE_DUTY_MIN;
    }

    /* Update V phase */
    g_tom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    if (g_tom3phInv.dutyCycles[1] >= PHASE_DUTY_MAX)
    {
        g_tom3phInv.dutyCycles[1] = PHASE_DUTY_MIN;
    }

    /* Update W phase */
    g_tom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;
    if (g_tom3phInv.dutyCycles[2] >= PHASE_DUTY_MAX)
    {
        g_tom3phInv.dutyCycles[2] = PHASE_DUTY_MIN;
    }

    /* Apply synchronous update (percent units) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInv.pwm, (float32 *)g_tom3phInv.dutyCycles);
}
