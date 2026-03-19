#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"

/* =============================
 * Internal types and state
 * ============================= */
typedef struct
{
    IfxGtm_Pwm          pwm;                            /* Unified PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];      /* Channel data populated by init */
    float32             dutyCycles[NUM_OF_CHANNELS];     /* Last applied duties (percent) in channel order */
} GtmTom3phInv_Driver;

static IfxGtm_Tom_Timer   s_timebase;                   /* Common TOM1 timebase */
static GtmTom3phInv_Driver s_drv;                       /* PWM driver state */
static boolean            s_initialized = FALSE;        /* Init flag */

/* HS duty accumulators for U, V, W (percent) */
static float32 s_phaseDutyHs[3] = {0.0f, 0.0f, 0.0f};   /* [U, V, W] */

/* =============================
 * Local helpers
 * ============================= */
static float32 clampFloat32(float32 value, float32 minVal, float32 maxVal)
{
    if (value < minVal)
    {
        return minVal;
    }
    if (value > maxVal)
    {
        return maxVal;
    }
    return value;
}

/* =============================
 * Public API
 * ============================= */
void initGtmTomPwm(void)
{
    /* 1) Enable GTM and FXCLK (retain existing clock init policy) */
    IfxGtm_enable(&MODULE_GTM);
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* 2) Configure TOM1 timebase (20 kHz, Fxclk0, center alignment via unified PWM config) */
    IfxGtm_Tom_Timer_Config timerConfig;
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    timerConfig.base.frequency = TIMEBASE_FREQUENCY_HZ;          /* 20 kHz */
    timerConfig.clock          = TIMEBASE_CLOCK_SOURCE;           /* IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0 */
    timerConfig.tom            = IfxGtm_Tom_1;                    /* TOM1 timebase */
    timerConfig.timerChannel   = IfxGtm_Tom_Ch_0;                 /* Common timebase channel */

    if (IfxGtm_Tom_Timer_init(&s_timebase, &timerConfig) == FALSE)
    {
        /* Error: timebase init failed - do not proceed */
        return;
    }

    IfxGtm_Tom_Timer_updateInputFrequency(&s_timebase);

    /* 3) Map six TOM1 channels to assigned pins (PinMap API) */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1); /* TOM1 CH1 LS */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1); /* TOM1 CH2 HS */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1); /* TOM1 CH3 LS */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1); /* TOM1 CH4 HS */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1); /* TOM1 CH5 LS */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1); /* TOM1 CH6 HS */

    /* 4) Initialize unified multi-channel PWM driver for six TOM1 channels */
    IfxGtm_Pwm_Config        pwmConfig;
    IfxGtm_Pwm_ChannelConfig chCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig  outCfg[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&pwmConfig, &MODULE_GTM);

    /* Init per-channel defaults */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
    }

    /* Output routing for six independent channels (no hardware DTM; software dead-time applied in duties) */
    /* Channel order (matches TOM1 channel numbers 1..6): [U_LS, U_HS, V_LS, V_HS, W_LS, W_HS] */
    outCfg[0].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;  /* TOM1 CH1 */
    outCfg[1].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;  /* TOM1 CH2 */
    outCfg[2].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;  /* TOM1 CH3 */
    outCfg[3].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;  /* TOM1 CH4 */
    outCfg[4].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;  /* TOM1 CH5 */
    outCfg[5].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;  /* TOM1 CH6 */

    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        outCfg[i].polarity   = Ifx_ActiveState_high;
        outCfg[i].outputMode = IfxPort_OutputMode_pushPull;
        outCfg[i].padDriver  = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    }

    /* Channel configuration: map to TOM1 channels 1..6 */
    chCfg[0].timerCh = IfxGtm_Pwm_SubModule_Ch_1; chCfg[0].phase = 0.0f; chCfg[0].duty = 0.0f; chCfg[0].output = &outCfg[0]; chCfg[0].dtm = NULL_PTR; chCfg[0].interrupt = NULL_PTR;
    chCfg[1].timerCh = IfxGtm_Pwm_SubModule_Ch_2; chCfg[1].phase = 0.0f; chCfg[1].duty = 0.0f; chCfg[1].output = &outCfg[1]; chCfg[1].dtm = NULL_PTR; chCfg[1].interrupt = NULL_PTR;
    chCfg[2].timerCh = IfxGtm_Pwm_SubModule_Ch_3; chCfg[2].phase = 0.0f; chCfg[2].duty = 0.0f; chCfg[2].output = &outCfg[2]; chCfg[2].dtm = NULL_PTR; chCfg[2].interrupt = NULL_PTR;
    chCfg[3].timerCh = IfxGtm_Pwm_SubModule_Ch_4; chCfg[3].phase = 0.0f; chCfg[3].duty = 0.0f; chCfg[3].output = &outCfg[3]; chCfg[3].dtm = NULL_PTR; chCfg[3].interrupt = NULL_PTR;
    chCfg[4].timerCh = IfxGtm_Pwm_SubModule_Ch_5; chCfg[4].phase = 0.0f; chCfg[4].duty = 0.0f; chCfg[4].output = &outCfg[4]; chCfg[4].dtm = NULL_PTR; chCfg[4].interrupt = NULL_PTR;
    chCfg[5].timerCh = IfxGtm_Pwm_SubModule_Ch_6; chCfg[5].phase = 0.0f; chCfg[5].duty = 0.0f; chCfg[5].output = &outCfg[5]; chCfg[5].dtm = NULL_PTR; chCfg[5].interrupt = NULL_PTR;

    pwmConfig.cluster            = IfxGtm_Cluster_0;                 /* TOM1 is in Cluster 0 on TC3xx */
    pwmConfig.subModule          = IfxGtm_Pwm_SubModule_tom;         /* TOM */
    pwmConfig.alignment          = IfxGtm_Pwm_Alignment_center;      /* Center-aligned (up-down) */
    pwmConfig.syncStart          = TRUE;                              /* Start channels after init */
    pwmConfig.syncUpdateEnabled  = TRUE;                              /* Shadow update sync */
    pwmConfig.numChannels        = NUM_OF_CHANNELS;                   /* Six independent channels */
    pwmConfig.channels           = chCfg;                             /* Channel config array */
    pwmConfig.frequency          = TIMING_PWM_FREQUENCY_HZ;           /* 20 kHz */
    pwmConfig.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;                /* Fxclk0 */

    IfxGtm_Pwm_init(&s_drv.pwm, s_drv.channels, &pwmConfig);

    /* 5) Start TOM timer and enable/start synchronized PWM channels */
    IfxGtm_Tom_Timer_run(&s_timebase);
    IfxGtm_Pwm_startSyncedChannels(&s_drv.pwm);

    /* 6) Compute initial duties: HS as requested; LS = 100 - HS - deadTimePct */
    s_phaseDutyHs[0] = INITIAL_DUTY_PERCENT_U; /* U */
    s_phaseDutyHs[1] = INITIAL_DUTY_PERCENT_V; /* V */
    s_phaseDutyHs[2] = INITIAL_DUTY_PERCENT_W; /* W */

    float32 fxclkHz     = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0, TRUE);
    Ifx_TimerValue periodTicks = IfxGtm_Tom_Timer_getPeriod(&s_timebase);
    float32 deadTicks   = fxclkHz * (TIMING_SOFTWARE_DEAD_TIME_US * 1.0e-6f);
    float32 deadPct     = (periodTicks > 0U) ? ((deadTicks / (float32)periodTicks) * 100.0f) : 0.0f;

    float32 dutyU_hs = s_phaseDutyHs[0];
    float32 dutyV_hs = s_phaseDutyHs[1];
    float32 dutyW_hs = s_phaseDutyHs[2];

    float32 dutyU_ls = clampFloat32(100.0f - dutyU_hs - deadPct, 0.0f, 100.0f);
    float32 dutyV_ls = clampFloat32(100.0f - dutyV_hs - deadPct, 0.0f, 100.0f);
    float32 dutyW_ls = clampFloat32(100.0f - dutyW_hs - deadPct, 0.0f, 100.0f);

    /* 7) Apply atomically in channel order: [U_LS, U_HS, V_LS, V_HS, W_LS, W_HS] */
    float32 duties6[NUM_OF_CHANNELS];
    duties6[0] = dutyU_ls; duties6[1] = dutyU_hs;
    duties6[2] = dutyV_ls; duties6[3] = dutyV_hs;
    duties6[4] = dutyW_ls; duties6[5] = dutyW_hs;

    IfxGtm_Tom_Timer_disableUpdate(&s_timebase);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, duties6);
    IfxGtm_Tom_Timer_applyUpdate(&s_timebase);

    /* Save last applied duties */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        s_drv.dutyCycles[i] = duties6[i];
    }

    s_initialized = TRUE;
}

void updateGtmTomPwmDutyCycles(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if not initialized */
    }

    /* 1) Read period ticks from timebase */
    Ifx_TimerValue periodTicks = IfxGtm_Tom_Timer_getPeriod(&s_timebase);

    /* 2) Increment HS duties by +10% with wrap in [0..100) */
    const float32 step = DUTY_UPDATE_POLICY_STEP_PERCENT;
    for (uint8 i = 0U; i < 3U; i++)
    {
        s_phaseDutyHs[i] += step;
        if (s_phaseDutyHs[i] >= 100.0f)
        {
            s_phaseDutyHs[i] -= 100.0f; /* modulo wrap */
        }
    }

    /* 3) Convert 0.5us dead-time to percent using Fxclk0 */
    float32 fxclkHz   = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0, TRUE);
    float32 deadTicks = fxclkHz * (TIMING_SOFTWARE_DEAD_TIME_US * 1.0e-6f);
    float32 deadPct   = (periodTicks > 0U) ? ((deadTicks / (float32)periodTicks) * 100.0f) : 0.0f;

    /* 4) LS = 100 - HS - deadPct; clamp [0..100] */
    float32 dutyU_hs = s_phaseDutyHs[0];
    float32 dutyV_hs = s_phaseDutyHs[1];
    float32 dutyW_hs = s_phaseDutyHs[2];

    float32 dutyU_ls = clampFloat32(100.0f - dutyU_hs - deadPct, 0.0f, 100.0f);
    float32 dutyV_ls = clampFloat32(100.0f - dutyV_hs - deadPct, 0.0f, 100.0f);
    float32 dutyW_ls = clampFloat32(100.0f - dutyW_hs - deadPct, 0.0f, 100.0f);

    /* 5) Build duties in configured channel order */
    float32 duties6[NUM_OF_CHANNELS];
    duties6[0] = dutyU_ls; duties6[1] = dutyU_hs;
    duties6[2] = dutyV_ls; duties6[3] = dutyV_hs;
    duties6[4] = dutyW_ls; duties6[5] = dutyW_hs;

    /* 6) Synchronous update using disable/apply gating */
    IfxGtm_Tom_Timer_disableUpdate(&s_timebase);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, duties6);
    IfxGtm_Tom_Timer_applyUpdate(&s_timebase);

    /* Save last applied duties */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        s_drv.dutyCycles[i] = duties6[i];
    }
}
