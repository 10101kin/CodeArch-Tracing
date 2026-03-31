/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM
 * Submodule: TOM, Center-aligned, Sync start/update, 20 kHz
 * API: IfxGtm_Pwm (unified high-level PWM)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define GTM_TOM3PH_NUM_CHANNELS         (3u)
#define GTM_TOM3PH_PWM_FREQUENCY_HZ     (20000.0f)
#define PHASE_U_INIT_DUTY               (25.0f)    /* percent */
#define PHASE_V_INIT_DUTY               (50.0f)    /* percent */
#define PHASE_W_INIT_DUTY               (75.0f)    /* percent */
#define PHASE_DUTY_STEP                 (10.0f)    /* percent */
#define PHASE_DUTY_MIN                  (10.0f)    /* percent */
#define PHASE_DUTY_MAX                  (90.0f)    /* percent */
#define PWM_DEAD_TIME_S                 (1.0e-6f)  /* seconds (migration value, applied even if complementary disabled) */

/*
 * User-requested pin assignments (preserved TOM1 pins)
 * Note: Single-ended: use only high-side pins for output[].pin; complementaryPin remains NULL_PTR.
 */
#define PHASE_U_HS                      (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)  /* U high-side: P00.3 (TOM1 CH2) */
#define PHASE_U_LS                      (&IfxGtm_TOM1_5_TOUT11_P00_2_OUT)  /* U low-side : P00.2 (TOM1 CH1) */
#define PHASE_V_HS                      (&IfxGtm_TOM1_1N_TOUT14_P00_5_OUT) /* V high-side: P00.5 (mapped option for TOM1) */
#define PHASE_V_LS                      (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)  /* V low-side : P00.4 (TOM1 CH3) */
#define PHASE_W_HS                      (&IfxGtm_TOM1_3N_TOUT16_P00_7_OUT) /* W high-side: P00.7 (mapped option for TOM1) */
#define PHASE_W_LS                      (&IfxGtm_TOM1_2N_TOUT15_P00_6_OUT) /* W low-side : P00.6 (TOM1 CH5) */

/* =============================
 * Module State
 * ============================= */

typedef struct
{
    IfxGtm_Pwm               pwm;                                           /* PWM driver handle */
    IfxGtm_Pwm_Channel       channels[GTM_TOM3PH_NUM_CHANNELS];            /* persistent channel runtime storage */
    float32                  dutyCycles[GTM_TOM3PH_NUM_CHANNELS];          /* percent */
    float32                  phases[GTM_TOM3PH_NUM_CHANNELS];              /* seconds or degrees (here: 0.0f) */
    IfxGtm_Pwm_DeadTime      deadTimes[GTM_TOM3PH_NUM_CHANNELS];           /* store configured dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInvState;

/* =============================
 * Public API
 * ============================= */

/*
 * Initialize 3-phase center-aligned PWM on TOM with three logical channels (U,V,W).
 * - Single-ended pins (HS only), complementaryPin = NULL_PTR
 * - Sync start and sync update enabled, 20 kHz
 * - Initial duties: 25%, 50%, 75%
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM3PH_NUM_CHANNELS];

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output and per-channel configuration */
    /* Phase U → channel index 0 */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin        = NULL_PTR; /* single-ended */
    output[0].polarity                = Ifx_ActiveState_high;
    output[0].complementaryPolarity   = Ifx_ActiveState_low;
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V → channel index 1 */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin        = NULL_PTR;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W → channel index 2 */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin        = NULL_PTR;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: apply migration value even for single-ended (kept in state) */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_S;

    /* Channel logical indices must be ordinal (Ch_0..Ch_2) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;  /* Phase U */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = NULL_PTR;                   /* no ISR used */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;  /* Phase V */
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;                   /* no ISR used */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;  /* Phase W */
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;                   /* no ISR used */

    /* 4) Main configuration */
    config.cluster              = IfxGtm_Cluster_0;                   /* cluster selection */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;           /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;        /* center-aligned */
    config.syncStart            = TRUE;                                /* start all in sync */
    config.syncUpdateEnabled    = TRUE;                                /* coherent shadow transfers */
    config.numChannels          = (uint8)GTM_TOM3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM3PH_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                  /* CLOCK SOURCE UNION: TOM → Fxclk_0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;  /* DTM clock source */

    /* 5) GTM enable guard and CMU clocks (follow mandatory pattern) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent state handle and channels array */
    IfxGtm_Pwm_init(&g_tom3phInvState.pwm, &g_tom3phInvState.channels[0], &config);

    /* 7) Persist initial state for runtime updates */
    g_tom3phInvState.dutyCycles[0] = PHASE_U_INIT_DUTY;
    g_tom3phInvState.dutyCycles[1] = PHASE_V_INIT_DUTY;
    g_tom3phInvState.dutyCycles[2] = PHASE_W_INIT_DUTY;

    g_tom3phInvState.phases[0] = 0.0f;
    g_tom3phInvState.phases[1] = 0.0f;
    g_tom3phInvState.phases[2] = 0.0f;

    g_tom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;
}

/*
 * Update the three phase duty cycles in percent and apply them synchronously.
 * Algorithm: add +10.0 to each duty; if a value becomes >= 90.0, wrap it to 10.0.
 * Then call the immediate multi-channel update API for coherent shadow transfer.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Add step and wrap within [10, 90) per phase (no loop per design guideline) */
    g_tom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    if (g_tom3phInvState.dutyCycles[0] >= PHASE_DUTY_MAX)
    {
        g_tom3phInvState.dutyCycles[0] = PHASE_DUTY_MIN;
    }

    g_tom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    if (g_tom3phInvState.dutyCycles[1] >= PHASE_DUTY_MAX)
    {
        g_tom3phInvState.dutyCycles[1] = PHASE_DUTY_MIN;
    }

    g_tom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;
    if (g_tom3phInvState.dutyCycles[2] >= PHASE_DUTY_MAX)
    {
        g_tom3phInvState.dutyCycles[2] = PHASE_DUTY_MIN;
    }

    /* Apply all three updates immediately and synchronously */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInvState.pwm, (float32*)g_tom3phInvState.dutyCycles);
}
