/*
 * gtm_tom_3_phase_inverter_pwm.c
 * TC3xx (TC387) - 3-Phase Inverter PWM using GTM TOM unified high-level driver (IfxGtm_Pwm)
 *
 * Implements:
 *   - void initGtmTomPwm(void)
 *   - void updateGtmTomPwmDutyCycles(void)
 *
 * Conforms to the SW Detailed Design behavior_description:
 *   - 20 kHz, center-aligned, synchronized updates
 *   - Six single-output TOM channels under a shared time base
 *   - High-side duties initialized to 25/50/75 percent
 *   - Low-sides use inverted polarity (complementary behavior), same numeric duty
 *   - Update function increments HS duties by +10% with wrap to 0% and applies all six
 *     duty values atomically using timer disable/apply shadow sequence
 *
 * CRITICAL RULES APPLIED:
 *   - Use IfxGtm_Pwm high-level unified driver with OutputConfig/ChannelConfig arrays
 *   - No explicit PinMap_setTomTout calls; pin routing is handled via OutputConfig
 *   - GTM/CMU clocks enabled before initialization
 *   - Do not include watchdog disables here
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD includes (generic headers only) */
#include "IfxGtm_Pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxPort.h"

/* ========================================================================== */
/* Requirements-derived configuration values                                   */
/* ========================================================================== */
#define TIMING_FREQUENCY_HZ                 (20000.0f)   /* 20 kHz */
#define TIMING_FXCLK_FREQUENCY_MHZ          (100.0f)
#define TIMING_PRESCALER                    (1u)
#define TIMING_TIMER_BASE_PERIOD_TICKS      (2500u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ      (300.0f)
#define CLOCK_GTM_FXCLK_MHZ                 (100.0f)

#define INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U   (25.0f)
#define INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V   (50.0f)
#define INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W   (75.0f)

/* Runtime duty behavior */
#define DUTY_STEP                            (10.0f)
#define DUTY_WRAP_THRESHOLD                  (100.0f)

/* Channel count: six single-output TOM channels */
#define NUM_OF_CHANNELS                      (6u)

/* ========================================================================== */
/* Pin assignments (validated examples; use generic PinMap header)            */
/* The exact TOM channels are selected per available GTM_TOUT routing.        */
/* Low-sides are modeled as independent single-output channels with inverted   */
/* polarity (no hardware dead-time).                                           */
/* ========================================================================== */
/* U phase: HS + LS */
#define PHASE_U_HS   (&IfxGtm_TOM0_10_TOUT12_P00_3_OUT)  /* TOM0 Ch10 -> TOUT12 P00.3 */
#define PHASE_U_LS   (&IfxGtm_TOM0_11_TOUT13_P00_4_OUT)  /* TOM0 Ch11 -> TOUT13 P00.4 */

/* V phase: HS + LS */
#define PHASE_V_HS   (&IfxGtm_TOM0_12_TOUT50_P22_3_OUT)  /* TOM0 Ch12 -> TOUT50 P22.3 */
#define PHASE_V_LS   (&IfxGtm_TOM0_6_TOUT48_P22_1_OUT)   /* TOM0 Ch6  -> TOUT48 P22.1 */

/* W phase: HS + LS */
#define PHASE_W_HS   (&IfxGtm_TOM0_0_TOUT26_P33_4_OUT)   /* TOM0 Ch0  -> TOUT26 P33.4 */
#define PHASE_W_LS   (&IfxGtm_TOM0_1_TOUT54_P21_3_OUT)   /* TOM0 Ch1  -> TOUT54 P21.3 */

/* Timer TOM selection: shared time base on TOM0 channel 0 (example) */
#define GTM_TOM_MASTER             (IfxGtm_Tom_0)
#define GTM_TOM_MASTER_TIMER_CH    (IfxGtm_Tom_Ch_0)

/* ========================================================================== */
/* Driver state                                                               */
/* ========================================================================== */

typedef struct
{
    IfxGtm_Pwm           pwm;                          /* Unified PWM driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];    /* Channel data returned by init */
    float32              dutyCycles[NUM_OF_CHANNELS];  /* Six-channel duty array (percent) */
    float32              hsDuty[3];                    /* High-side duty (U,V,W) percent */
} GtmTom3phPwmDriver;

static GtmTom3phPwmDriver s_pwmDrv;        /* Module-static driver */
static IfxGtm_Tom_Timer   s_tomTimer;      /* Shared TOM time base for shadow gating */
static boolean            s_initialized = FALSE;

/* ========================================================================== */
/* Private helpers                                                            */
/* ========================================================================== */
/**
 * Compute next high-side duties by +10% with wrap, then propagate into the
 * six-channel output array in channel order. Low-sides use the same numeric
 * duty values; their inversion is realized by output polarity.
 *
 * Channel order used by this module:
 *   [0] U_HS, [1] U_LS, [2] V_HS, [3] V_LS, [4] W_HS, [5] W_LS
 */
static void computeNextDuties(float32 *hsDutyInOut, float32 *sixChannelDutyOut)
{
    /* Increment U, V, W by +10% with wrap to 0% when exceeding 100% */
    for (uint8 i = 0u; i < 3u; i++)
    {
        float32 d = hsDutyInOut[i] + DUTY_STEP;
        if (d > DUTY_WRAP_THRESHOLD)
        {
            d = 0.0f;
        }
        hsDutyInOut[i] = d;
    }

    /* Assemble six-channel array in configured order */
    sixChannelDutyOut[0] = hsDutyInOut[0]; /* U_HS */
    sixChannelDutyOut[1] = hsDutyInOut[0]; /* U_LS (inverted by polarity) */
    sixChannelDutyOut[2] = hsDutyInOut[1]; /* V_HS */
    sixChannelDutyOut[3] = hsDutyInOut[1]; /* V_LS */
    sixChannelDutyOut[4] = hsDutyInOut[2]; /* W_HS */
    sixChannelDutyOut[5] = hsDutyInOut[2]; /* W_LS */
}

/* ========================================================================== */
/* Public API                                                                 */
/* ========================================================================== */
/**
 * Initialize GTM clocks, TOM time base, and six single-output PWM channels using IfxGtm_Pwm.
 * - 20 kHz, center-aligned, synchronized updates enabled
 * - High-sides active HIGH; Low-sides active LOW (inverted), no hardware dead-time
 * - Pins routed via OutputConfig (no explicit PinMap_setTomTout calls)
 * - After init, stage and apply an initial shadow update of all six duties
 */
void initGtmTomPwm(void)
{
    /* 1) Enable GTM and configure CMU clocks (GCLK, CLK0, FXCLK) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        float32 moduleFreq = 0.0f;
        IfxGtm_enable(&MODULE_GTM);
        moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFreq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0);
    }

    /* 2) Initialize shared TOM time base (for shadow gating) */
    {
        IfxGtm_Tom_Timer_Config timerConfig;
        IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
        /* Configure timer base per requirements */
        timerConfig.base.frequency  = TIMING_FREQUENCY_HZ;          /* 20 kHz */
        timerConfig.base.isrPriority = 0;                           /* No ISR here */
        timerConfig.base.triggerOut  = NULL_PTR;                    /* Not used */
        timerConfig.base.minResolution = 0.0f;                      /* default */
        timerConfig.base.start       = TRUE;                        /* start request */
        timerConfig.base.alignment   = Ifx_Tim_Mode_centerAligned;  /* center-aligned base */
        timerConfig.clock            = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0; /* FXCLK0 */
        timerConfig.tom              = GTM_TOM_MASTER;              /* TOM0 */
        timerConfig.timerChannel     = GTM_TOM_MASTER_TIMER_CH;     /* Ch0 as time base */

        if (IfxGtm_Tom_Timer_init(&s_tomTimer, &timerConfig) == FALSE)
        {
            /* Early-exit on failure; leave module uninitialized */
            s_initialized = FALSE;
            return;
        }
        IfxGtm_Tom_Timer_updateInputFrequency(&s_tomTimer);
    }

    /* 3) Configure unified PWM driver with six single-output channels */
    {
        IfxGtm_Pwm_Config           config;
        IfxGtm_Pwm_ChannelConfig    channelCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig     outputCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_DtmConfig        dtmCfg[NUM_OF_CHANNELS];

        /* Zero DTMs (no dead-time); link anyway to conform with unified driver topology */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            dtmCfg[i].deadTime.rising  = 0.0f;
            dtmCfg[i].deadTime.falling = 0.0f;
        }

        /* Initialize config defaults */
        IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

        /* Output pin routing via OutputConfig array (no PinMap API calls) */
        /* Order: [0] U_HS, [1] U_LS, [2] V_HS, [3] V_LS, [4] W_HS, [5] W_LS */
        outputCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        outputCfg[0].complementaryPin      = NULL_PTR;
        outputCfg[0].polarity              = Ifx_ActiveState_high; /* HS active high */
        outputCfg[0].complementaryPolarity = Ifx_ActiveState_low;
        outputCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
        outputCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outputCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        outputCfg[1].complementaryPin      = NULL_PTR;
        outputCfg[1].polarity              = Ifx_ActiveState_low;  /* LS inverted */
        outputCfg[1].complementaryPolarity = Ifx_ActiveState_high;
        outputCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
        outputCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outputCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        outputCfg[2].complementaryPin      = NULL_PTR;
        outputCfg[2].polarity              = Ifx_ActiveState_high;
        outputCfg[2].complementaryPolarity = Ifx_ActiveState_low;
        outputCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
        outputCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outputCfg[3].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        outputCfg[3].complementaryPin      = NULL_PTR;
        outputCfg[3].polarity              = Ifx_ActiveState_low;
        outputCfg[3].complementaryPolarity = Ifx_ActiveState_high;
        outputCfg[3].outputMode            = IfxPort_OutputMode_pushPull;
        outputCfg[3].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outputCfg[4].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        outputCfg[4].complementaryPin      = NULL_PTR;
        outputCfg[4].polarity              = Ifx_ActiveState_high;
        outputCfg[4].complementaryPolarity = Ifx_ActiveState_low;
        outputCfg[4].outputMode            = IfxPort_OutputMode_pushPull;
        outputCfg[4].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outputCfg[5].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        outputCfg[5].complementaryPin      = NULL_PTR;
        outputCfg[5].polarity              = Ifx_ActiveState_low;
        outputCfg[5].complementaryPolarity = Ifx_ActiveState_high;
        outputCfg[5].outputMode            = IfxPort_OutputMode_pushPull;
        outputCfg[5].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* Channel configuration: bind TOM channels and initial duties */
        /* Map to TOM0 channels consistent with selected pins */
        /* U_HS: ch10, U_LS: ch11, V_HS: ch12, V_LS: ch6, W_HS: ch0, W_LS: ch1 */
        /* Initial duties: HS = 25/50/75, LS = same numeric value (inversion by polarity) */
        /* Channel 0: U_HS (Ch10) */
        channelCfg[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_10;
        channelCfg[0].phase     = 0.0f;
        channelCfg[0].duty      = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U;
        channelCfg[0].dtm       = &dtmCfg[0];
        channelCfg[0].output    = &outputCfg[0];
        channelCfg[0].mscOut    = NULL_PTR;
        channelCfg[0].interrupt = NULL_PTR;

        /* Channel 1: U_LS (Ch11) */
        channelCfg[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_11;
        channelCfg[1].phase     = 0.0f;
        channelCfg[1].duty      = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U;
        channelCfg[1].dtm       = &dtmCfg[1];
        channelCfg[1].output    = &outputCfg[1];
        channelCfg[1].mscOut    = NULL_PTR;
        channelCfg[1].interrupt = NULL_PTR;

        /* Channel 2: V_HS (Ch12) */
        channelCfg[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_12;
        channelCfg[2].phase     = 0.0f;
        channelCfg[2].duty      = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V;
        channelCfg[2].dtm       = &dtmCfg[2];
        channelCfg[2].output    = &outputCfg[2];
        channelCfg[2].mscOut    = NULL_PTR;
        channelCfg[2].interrupt = NULL_PTR;

        /* Channel 3: V_LS (Ch6) */
        channelCfg[3].timerCh   = IfxGtm_Pwm_SubModule_Ch_6;
        channelCfg[3].phase     = 0.0f;
        channelCfg[3].duty      = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V;
        channelCfg[3].dtm       = &dtmCfg[3];
        channelCfg[3].output    = &outputCfg[3];
        channelCfg[3].mscOut    = NULL_PTR;
        channelCfg[3].interrupt = NULL_PTR;

        /* Channel 4: W_HS (Ch0) */
        channelCfg[4].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
        channelCfg[4].phase     = 0.0f;
        channelCfg[4].duty      = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W;
        channelCfg[4].dtm       = &dtmCfg[4];
        channelCfg[4].output    = &outputCfg[4];
        channelCfg[4].mscOut    = NULL_PTR;
        channelCfg[4].interrupt = NULL_PTR;

        /* Channel 5: W_LS (Ch1) */
        channelCfg[5].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
        channelCfg[5].phase     = 0.0f;
        channelCfg[5].duty      = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W;
        channelCfg[5].dtm       = &dtmCfg[5];
        channelCfg[5].output    = &outputCfg[5];
        channelCfg[5].mscOut    = NULL_PTR;
        channelCfg[5].interrupt = NULL_PTR;

        /* Top-level PWM configuration */
        config.cluster            = IfxGtm_Cluster_0;
        config.subModule          = IfxGtm_Pwm_SubModule_tom;
        config.alignment          = IfxGtm_Pwm_Alignment_center;  /* center-aligned */
        config.syncStart          = TRUE;                         /* auto-start after init */
        config.syncUpdateEnabled  = TRUE;                         /* shadow updates */
        config.numChannels        = NUM_OF_CHANNELS;
        config.channels           = &channelCfg[0];
        config.frequency          = TIMING_FREQUENCY_HZ;
        config.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;           /* TOM uses Fxclk enum */
        config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;

        /* Initialize PWM driver (no return value) */
        IfxGtm_Pwm_init(&s_pwmDrv.pwm, &s_pwmDrv.channels[0], &config);

        /* Cache initial duties for runtime updates (HS and 6-channel array) */
        s_pwmDrv.hsDuty[0] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U;
        s_pwmDrv.hsDuty[1] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V;
        s_pwmDrv.hsDuty[2] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W;

        s_pwmDrv.dutyCycles[0] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U; /* U_HS */
        s_pwmDrv.dutyCycles[1] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U; /* U_LS */
        s_pwmDrv.dutyCycles[2] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V; /* V_HS */
        s_pwmDrv.dutyCycles[3] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V; /* V_LS */
        s_pwmDrv.dutyCycles[4] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W; /* W_HS */
        s_pwmDrv.dutyCycles[5] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W; /* W_LS */
    }

    /* 4) Start the TOM time base and push initial shadowed update for sync start */
    IfxGtm_Tom_Timer_run(&s_tomTimer);
    IfxGtm_Tom_Timer_disableUpdate(&s_tomTimer);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwmDrv.pwm, &s_pwmDrv.dutyCycles[0]);
    IfxGtm_Tom_Timer_applyUpdate(&s_tomTimer);

    s_initialized = TRUE;
}

/**
 * Update function: increments high-side duties by +10% with wrap and applies
 * all six channel duties atomically using the TOM timer's shadow update sequence.
 * The caller is responsible for call rate control (e.g., every 500 ms).
 */
void updateGtmTomPwmDutyCycles(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if init failed or not performed */
    }

    computeNextDuties(&s_pwmDrv.hsDuty[0], &s_pwmDrv.dutyCycles[0]);

    /* Shadowed multi-channel update */
    IfxGtm_Tom_Timer_disableUpdate(&s_tomTimer);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwmDrv.pwm, &s_pwmDrv.dutyCycles[0]);
    IfxGtm_Tom_Timer_applyUpdate(&s_tomTimer);
}
