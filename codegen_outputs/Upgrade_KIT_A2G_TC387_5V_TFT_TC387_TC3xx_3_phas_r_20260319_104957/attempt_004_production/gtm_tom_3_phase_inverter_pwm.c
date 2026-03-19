/*
 * gtm_tom_3_phase_inverter_pwm.c
 * GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm driver
 * Target: TC3xx (TC387)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD includes required by SW Detailed Design driver_calls */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ===================== Internal module state ===================== */
static IfxGtm_Tom_Timer     s_timebase;                 /* Common TOM timebase */
static IfxGtm_Pwm           s_pwm;                      /* Unified PWM driver handle */
static IfxGtm_Pwm_Channel   s_channels[NUM_OF_CHANNELS];/* Channels returned by init */
static boolean              s_initialized = FALSE;      /* Init status */

/* High-side duty accumulators (percent, 0..100) for U/V/W */
static float32 s_hsDutyPct[3] = { INITIAL_DUTY_PERCENT_U, INITIAL_DUTY_PERCENT_V, INITIAL_DUTY_PERCENT_W };

/* Channel order indices for duty array assembly (must match configuration order) */
enum {
    CH_IDX_U_HS = 0,
    CH_IDX_U_LS = 1,
    CH_IDX_V_HS = 2,
    CH_IDX_V_LS = 3,
    CH_IDX_W_HS = 4,
    CH_IDX_W_LS = 5
};

/* ===================== Local helpers ===================== */
/** Clamp a float32 value to [minVal, maxVal] */
static float32 clampFloat32(float32 value, float32 minVal, float32 maxVal)
{
    float32 v = value;
    if (v < minVal) { v = minVal; }
    if (v > maxVal) { v = maxVal; }
    return v;
}

/* ===================== Public API implementations ===================== */
/**
 * Initialize the GTM-based three-phase PWM inverter per SW Detailed Design.
 * Steps:
 * 1) Enable GTM and FXCLK clock domain
 * 2) Configure TOM1 common timebase at 20 kHz, center-aligned, clocked by Fxclk0
 * 3) Configure six PWM outputs on TOM1 CH1..CH6 and map them to pins
 * 4) Initialize unified IfxGtm_Pwm driver with six channels
 * 5) Start TOM timer and start synced PWM channels
 * 6) Compute initial HS duties (U,V,W) and LS as complement minus SW dead-time (clamped)
 * 7) Apply all six initial duties atomically (disable/apply update)
 */
void initGtmTomPwm(void)
{
    /* 1) Enable GTM module and FXCLK clock domain */
    IfxGtm_enable(&MODULE_GTM);
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* 2) Configure TOM1 common timebase (20 kHz, Fxclk0) */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
        timerCfg.base.frequency = TIMEBASE_FREQUENCY_HZ;             /* 20 kHz */
        timerCfg.clock          = TIMEBASE_CLOCK_SOURCE;             /* IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0 */
        timerCfg.tom            = TIMEBASE_MODULE;                   /* TOM1 */
        timerCfg.timerChannel   = IfxGtm_Tom_Ch_0;                   /* Use CH0 as master timebase */
        if (IfxGtm_Tom_Timer_init(&s_timebase, &timerCfg) == FALSE)
        {
            /* Error handling: early return on failure (do not set s_initialized) */
            return;
        }
        IfxGtm_Tom_Timer_updateInputFrequency(&s_timebase);
    }

    /* 3) Map six PWM output pins (explicit PinMap per SW Detailed Design) */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) Initialize unified multi-channel PWM driver (six independent TOM channels) */
    {
        IfxGtm_Pwm_Config           cfg;
        IfxGtm_Pwm_ChannelConfig    chCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig     outCfg[NUM_OF_CHANNELS];

        IfxGtm_Pwm_initConfig(&cfg, &MODULE_GTM);

        /* Common PWM configuration */
        cfg.cluster            = IfxGtm_Cluster_0;
        cfg.subModule          = IfxGtm_Pwm_SubModule_tom;      /* TOM */
        cfg.alignment          = IfxGtm_Pwm_Alignment_center;   /* center-aligned */
        cfg.syncStart          = TRUE;                          /* start channels after init */
        cfg.syncUpdateEnabled  = TRUE;                          /* shadow updates */
        cfg.numChannels        = NUM_OF_CHANNELS;               /* 6 channels */
        cfg.frequency          = TIMEBASE_FREQUENCY_HZ;         /* 20 kHz */
        cfg.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;            /* Fxclk0 */

        /* Per-channel configuration (order must match enum indices above) */
        /* CH_IDX_U_HS -> TOM1_2 */
        IfxGtm_Pwm_initChannelConfig(&chCfg[CH_IDX_U_HS]);
        outCfg[CH_IDX_U_HS].pin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        outCfg[CH_IDX_U_HS].polarity = Ifx_ActiveState_high;
        outCfg[CH_IDX_U_HS].outputMode = IfxPort_OutputMode_pushPull;
        outCfg[CH_IDX_U_HS].padDriver  = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        chCfg[CH_IDX_U_HS].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
        chCfg[CH_IDX_U_HS].phase     = 0.0f;
        chCfg[CH_IDX_U_HS].duty      = 0.0f; /* will be set after run */
        chCfg[CH_IDX_U_HS].output    = &outCfg[CH_IDX_U_HS];
        chCfg[CH_IDX_U_HS].dtm       = NULL_PTR;
        chCfg[CH_IDX_U_HS].mscOut    = NULL_PTR;
        chCfg[CH_IDX_U_HS].interrupt = NULL_PTR;

        /* CH_IDX_U_LS -> TOM1_1 */
        IfxGtm_Pwm_initChannelConfig(&chCfg[CH_IDX_U_LS]);
        outCfg[CH_IDX_U_LS].pin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        outCfg[CH_IDX_U_LS].polarity = Ifx_ActiveState_high;
        outCfg[CH_IDX_U_LS].outputMode = IfxPort_OutputMode_pushPull;
        outCfg[CH_IDX_U_LS].padDriver  = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        chCfg[CH_IDX_U_LS].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
        chCfg[CH_IDX_U_LS].phase     = 0.0f;
        chCfg[CH_IDX_U_LS].duty      = 0.0f;
        chCfg[CH_IDX_U_LS].output    = &outCfg[CH_IDX_U_LS];
        chCfg[CH_IDX_U_LS].dtm       = NULL_PTR;
        chCfg[CH_IDX_U_LS].mscOut    = NULL_PTR;
        chCfg[CH_IDX_U_LS].interrupt = NULL_PTR;

        /* CH_IDX_V_HS -> TOM1_4 */
        IfxGtm_Pwm_initChannelConfig(&chCfg[CH_IDX_V_HS]);
        outCfg[CH_IDX_V_HS].pin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        outCfg[CH_IDX_V_HS].polarity = Ifx_ActiveState_high;
        outCfg[CH_IDX_V_HS].outputMode = IfxPort_OutputMode_pushPull;
        outCfg[CH_IDX_V_HS].padDriver  = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        chCfg[CH_IDX_V_HS].timerCh   = IfxGtm_Pwm_SubModule_Ch_4;
        chCfg[CH_IDX_V_HS].phase     = 0.0f;
        chCfg[CH_IDX_V_HS].duty      = 0.0f;
        chCfg[CH_IDX_V_HS].output    = &outCfg[CH_IDX_V_HS];
        chCfg[CH_IDX_V_HS].dtm       = NULL_PTR;
        chCfg[CH_IDX_V_HS].mscOut    = NULL_PTR;
        chCfg[CH_IDX_V_HS].interrupt = NULL_PTR;

        /* CH_IDX_V_LS -> TOM1_3 */
        IfxGtm_Pwm_initChannelConfig(&chCfg[CH_IDX_V_LS]);
        outCfg[CH_IDX_V_LS].pin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        outCfg[CH_IDX_V_LS].polarity = Ifx_ActiveState_high;
        outCfg[CH_IDX_V_LS].outputMode = IfxPort_OutputMode_pushPull;
        outCfg[CH_IDX_V_LS].padDriver  = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        chCfg[CH_IDX_V_LS].timerCh   = IfxGtm_Pwm_SubModule_Ch_3;
        chCfg[CH_IDX_V_LS].phase     = 0.0f;
        chCfg[CH_IDX_V_LS].duty      = 0.0f;
        chCfg[CH_IDX_V_LS].output    = &outCfg[CH_IDX_V_LS];
        chCfg[CH_IDX_V_LS].dtm       = NULL_PTR;
        chCfg[CH_IDX_V_LS].mscOut    = NULL_PTR;
        chCfg[CH_IDX_V_LS].interrupt = NULL_PTR;

        /* CH_IDX_W_HS -> TOM1_6 */
        IfxGtm_Pwm_initChannelConfig(&chCfg[CH_IDX_W_HS]);
        outCfg[CH_IDX_W_HS].pin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        outCfg[CH_IDX_W_HS].polarity = Ifx_ActiveState_high;
        outCfg[CH_IDX_W_HS].outputMode = IfxPort_OutputMode_pushPull;
        outCfg[CH_IDX_W_HS].padDriver  = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        chCfg[CH_IDX_W_HS].timerCh   = IfxGtm_Pwm_SubModule_Ch_6;
        chCfg[CH_IDX_W_HS].phase     = 0.0f;
        chCfg[CH_IDX_W_HS].duty      = 0.0f;
        chCfg[CH_IDX_W_HS].output    = &outCfg[CH_IDX_W_HS];
        chCfg[CH_IDX_W_HS].dtm       = NULL_PTR;
        chCfg[CH_IDX_W_HS].mscOut    = NULL_PTR;
        chCfg[CH_IDX_W_HS].interrupt = NULL_PTR;

        /* CH_IDX_W_LS -> TOM1_5 */
        IfxGtm_Pwm_initChannelConfig(&chCfg[CH_IDX_W_LS]);
        outCfg[CH_IDX_W_LS].pin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        outCfg[CH_IDX_W_LS].polarity = Ifx_ActiveState_high;
        outCfg[CH_IDX_W_LS].outputMode = IfxPort_OutputMode_pushPull;
        outCfg[CH_IDX_W_LS].padDriver  = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        chCfg[CH_IDX_W_LS].timerCh   = IfxGtm_Pwm_SubModule_Ch_5;
        chCfg[CH_IDX_W_LS].phase     = 0.0f;
        chCfg[CH_IDX_W_LS].duty      = 0.0f;
        chCfg[CH_IDX_W_LS].output    = &outCfg[CH_IDX_W_LS];
        chCfg[CH_IDX_W_LS].dtm       = NULL_PTR;
        chCfg[CH_IDX_W_LS].mscOut    = NULL_PTR;
        chCfg[CH_IDX_W_LS].interrupt = NULL_PTR;

        /* Link channels array to config and initialize */
        cfg.channels = &chCfg[0];
        IfxGtm_Pwm_init(&s_pwm, &s_channels[0], &cfg);
    }

    /* 5) Start TOM timer and enable/start synchronized PWM channels */
    IfxGtm_Tom_Timer_run(&s_timebase);
    IfxGtm_Pwm_startSyncedChannels(&s_pwm);

    /* 6) Compute initial duties with software dead-time complement */
    {
        float32 duty[NUM_OF_CHANNELS];
        const float32 stepU = s_hsDutyPct[0];
        const float32 stepV = s_hsDutyPct[1];
        const float32 stepW = s_hsDutyPct[2];
        const float32 fxFreq = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0, TRUE);
        const float32 periodTicks = (float32)IfxGtm_Tom_Timer_getPeriod(&s_timebase);
        float32 deadPct = 0.0f;
        if (periodTicks > 0.0f)
        {
            const float32 deadTimeTicks = (TIMING_SOFTWARE_DEAD_TIME_US * 1.0e-6f) * fxFreq;
            deadPct = (deadTimeTicks / periodTicks) * 100.0f;
        }

        duty[CH_IDX_U_HS] = stepU;
        duty[CH_IDX_U_LS] = clampFloat32(100.0f - stepU - deadPct, 0.0f, 100.0f);
        duty[CH_IDX_V_HS] = stepV;
        duty[CH_IDX_V_LS] = clampFloat32(100.0f - stepV - deadPct, 0.0f, 100.0f);
        duty[CH_IDX_W_HS] = stepW;
        duty[CH_IDX_W_LS] = clampFloat32(100.0f - stepW - deadPct, 0.0f, 100.0f);

        /* 7) Apply atomically */
        IfxGtm_Tom_Timer_disableUpdate(&s_timebase);
        IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwm, duty);
        IfxGtm_Tom_Timer_applyUpdate(&s_timebase);
    }

    s_initialized = TRUE;
}

/**
 * Periodic duty update per SW Detailed Design.
 * 1) Read period ticks from timebase
 * 2) Increment HS duties by +10% with wrap to 0..100
 * 3) Convert 0.5us SW dead-time to duty percent via Fxclk0 and period
 * 4) For each pair, LS = 100 - HS - deadPct, clamped to [0..100]
 * 5) Build six-element array in configured order
 * 6) Disable/Apply update around immediate update for synchronous latch
 */
void updateGtmTomPwmDutyCycles(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit if init failed or not performed */
    }

    const float32 periodTicks = (float32)IfxGtm_Tom_Timer_getPeriod(&s_timebase);
    const float32 fxFreq      = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0, TRUE);

    /* 2) Increment HS duties and wrap */
    const float32 step = DUTY_UPDATE_POLICY_STEP_PERCENT;
    for (uint8 i = 0u; i < 3u; i++)
    {
        s_hsDutyPct[i] += step;
        if (s_hsDutyPct[i] >= 100.0f)
        {
            s_hsDutyPct[i] -= 100.0f;
        }
    }

    /* 3) Compute dead-time percentage of period */
    float32 deadPct = 0.0f;
    if (periodTicks > 0.0f)
    {
        const float32 deadTimeTicks = (TIMING_SOFTWARE_DEAD_TIME_US * 1.0e-6f) * fxFreq;
        deadPct = (deadTimeTicks / periodTicks) * 100.0f;
    }

    /* 4,5) Prepare duty array in configured order */
    float32 duty[NUM_OF_CHANNELS];
    duty[CH_IDX_U_HS] = s_hsDutyPct[0];
    duty[CH_IDX_U_LS] = clampFloat32(100.0f - s_hsDutyPct[0] - deadPct, 0.0f, 100.0f);
    duty[CH_IDX_V_HS] = s_hsDutyPct[1];
    duty[CH_IDX_V_LS] = clampFloat32(100.0f - s_hsDutyPct[1] - deadPct, 0.0f, 100.0f);
    duty[CH_IDX_W_HS] = s_hsDutyPct[2];
    duty[CH_IDX_W_LS] = clampFloat32(100.0f - s_hsDutyPct[2] - deadPct, 0.0f, 100.0f);

    /* 6) Atomic update */
    IfxGtm_Tom_Timer_disableUpdate(&s_timebase);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwm, duty);
    IfxGtm_Tom_Timer_applyUpdate(&s_timebase);
}
