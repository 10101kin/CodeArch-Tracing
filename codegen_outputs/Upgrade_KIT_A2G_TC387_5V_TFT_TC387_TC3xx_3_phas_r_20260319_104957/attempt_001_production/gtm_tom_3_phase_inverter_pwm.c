/***************************************************************************
 * File: gtm_tom_3_phase_inverter_pwm.c
 * Description: GTM TOM 3-Phase Inverter PWM driver implementation
 * Target: TC3xx (GTM/TOM)
 * Drivers: IfxGtm_Tom_Timer (timebase), IfxGtm_Pwm (unified PWM), IfxGtm_PinMap
 ***************************************************************************/
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"
#include "IfxGtm_Tom.h"

/* ============================ Local Types/State ============================ */

typedef struct
{
    IfxGtm_Tom_Timer     timer;                    /* Common TOM timebase */
    IfxGtm_Pwm           pwm;                      /* Unified PWM driver */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];/* Per-channel driver data */

    /* Runtime state (percent) */
    float32              dutyHighPct[3];           /* High-side duties: U,V,W */
    float32              dutyAllPct[NUM_OF_CHANNELS]; /* Ordered as configured channels */

    boolean              initialized;              /* Init status */
} GtmTom3phInv_Driver;

static GtmTom3phInv_Driver g_drv = {0};

/* Channel order (by TOM1 channel ascending): CH1..CH6 */
#define CH_IDX_U_LS   (0u)  /* TOM1 CH1 */
#define CH_IDX_U_HS   (1u)  /* TOM1 CH2 */
#define CH_IDX_V_LS   (2u)  /* TOM1 CH3 */
#define CH_IDX_V_HS   (3u)  /* TOM1 CH4 */
#define CH_IDX_W_LS   (4u)  /* TOM1 CH5 */
#define CH_IDX_W_HS   (5u)  /* TOM1 CH6 */

/* ============================ Local Helpers ============================ */
static float32 clampFloat32(float32 value, float32 minVal, float32 maxVal)
{
    float32 v = value;
    if (v < minVal) { v = minVal; }
    if (v > maxVal) { v = maxVal; }
    return v;
}

/* Prepare initial HS/LS duties with software dead-time compensation */
static void prv_prepareInitialDuties(void)
{
    /* Initialize HS duties from requirements */
    g_drv.dutyHighPct[0] = INITIAL_DUTY_PERCENT_U; /* U */
    g_drv.dutyHighPct[1] = INITIAL_DUTY_PERCENT_V; /* V */
    g_drv.dutyHighPct[2] = INITIAL_DUTY_PERCENT_W; /* W */

    /* Compute software dead-time as percentage of period */
    Ifx_TimerValue periodTicks = IfxGtm_Tom_Timer_getPeriod(&g_drv.timer);
    float32 fxclkHz = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0, TRUE);
    float32 deadTimeTicks = fxclkHz * (TIMING_SOFTWARE_DEAD_TIME_US * 1.0e-6f);
    float32 deadTimePct = (periodTicks > 0) ? ((deadTimeTicks / (float32)periodTicks) * 100.0f) : 0.0f;

    /* High-sides per channel order: U_HS, V_HS, W_HS placed at CH2, CH4, CH6 */
    g_drv.dutyAllPct[CH_IDX_U_HS] = clampFloat32(g_drv.dutyHighPct[0], 0.0f, 100.0f);
    g_drv.dutyAllPct[CH_IDX_V_HS] = clampFloat32(g_drv.dutyHighPct[1], 0.0f, 100.0f);
    g_drv.dutyAllPct[CH_IDX_W_HS] = clampFloat32(g_drv.dutyHighPct[2], 0.0f, 100.0f);

    /* Low-sides = complement minus software dead-time; clamp [0,100] */
    g_drv.dutyAllPct[CH_IDX_U_LS] = clampFloat32(100.0f - g_drv.dutyHighPct[0] - deadTimePct, 0.0f, 100.0f);
    g_drv.dutyAllPct[CH_IDX_V_LS] = clampFloat32(100.0f - g_drv.dutyHighPct[1] - deadTimePct, 0.0f, 100.0f);
    g_drv.dutyAllPct[CH_IDX_W_LS] = clampFloat32(100.0f - g_drv.dutyHighPct[2] - deadTimePct, 0.0f, 100.0f);
}

/* ============================ Public Functions ============================ */
void initGtmTomPwm(void)
{
    /* 1) Enable GTM module and required clocks (retain existing board init pattern) */
    IfxGtm_enable(&MODULE_GTM);
    /* Set GCLK to module frequency, route CLK0 = GCLK, and enable FXCLK domain */
    {
        float32 gtmFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, gtmFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Create and initialize TOM timer config for 20 kHz timebase (TOM1) */
    {
        IfxGtm_Tom_Timer_Config timerConfig;
        IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
        timerConfig.base.frequency = TIMEBASE_FREQUENCY_HZ;             /* 20 kHz */
        timerConfig.clock          = TIMEBASE_CLOCK_SOURCE;              /* Fxclk0 */
        timerConfig.tom            = IfxGtm_Tom_1;                       /* TOM1 */
        timerConfig.timerChannel   = IfxGtm_Tom_Ch_0;                    /* Master timebase channel */
        if (IfxGtm_Tom_Timer_init(&g_drv.timer, &timerConfig) == FALSE)
        {
            /* HW init failed -> do not continue */
            g_drv.initialized = FALSE;
            return;
        }
        IfxGtm_Tom_Timer_updateInputFrequency(&g_drv.timer);
    }

    /* 3) Configure six PWM output channels and map to assigned pins via PinMap */
    {
        /* Route outputs: existing mapping on P00 */
        IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
        IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
        IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
        IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
        IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
        IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }

    /* 4) Initialize unified multi-channel PWM driver (6 channels on TOM, center-aligned) */
    {
        IfxGtm_Pwm_Config          config;
        IfxGtm_Pwm_ChannelConfig   chCfg[NUM_OF_CHANNELS];

        IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

        /* Per-channel defaults */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
            chCfg[i].phase     = 0.0f;       /* No phase shift */
            chCfg[i].duty      = 0.0f;       /* Will be set after start */
            chCfg[i].interrupt = NULL_PTR;   /* No ISR in this module */
        }
        /* Bind TOM1 CH1..CH6 in ascending order */
        chCfg[CH_IDX_U_LS].timerCh = IfxGtm_Pwm_SubModule_Ch_1; /* TOM1 CH1 */
        chCfg[CH_IDX_U_HS].timerCh = IfxGtm_Pwm_SubModule_Ch_2; /* TOM1 CH2 */
        chCfg[CH_IDX_V_LS].timerCh = IfxGtm_Pwm_SubModule_Ch_3; /* TOM1 CH3 */
        chCfg[CH_IDX_V_HS].timerCh = IfxGtm_Pwm_SubModule_Ch_4; /* TOM1 CH4 */
        chCfg[CH_IDX_W_LS].timerCh = IfxGtm_Pwm_SubModule_Ch_5; /* TOM1 CH5 */
        chCfg[CH_IDX_W_HS].timerCh = IfxGtm_Pwm_SubModule_Ch_6; /* TOM1 CH6 */

        /* Global PWM configuration */
        config.cluster            = IfxGtm_Cluster_0;
        config.subModule          = IfxGtm_Pwm_SubModule_tom;           /* TOM */
        config.alignment          = IfxGtm_Pwm_Alignment_center;        /* center-aligned */
        config.syncStart          = TRUE;                               /* start synced */
        config.syncUpdateEnabled  = TRUE;                               /* shadow update */
        config.numChannels        = NUM_OF_CHANNELS;                    /* 6 channels */
        config.channels           = &chCfg[0];
        config.frequency          = TIMEBASE_FREQUENCY_HZ;
        config.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;                 /* Fxclk0 */

        IfxGtm_Pwm_init(&g_drv.pwm, &g_drv.channels[0], &config);
    }

    /* 5) Start the TOM timer and enable/start synchronized PWM channels */
    IfxGtm_Tom_Timer_run(&g_drv.timer);
    IfxGtm_Pwm_startSyncedChannels(&g_drv.pwm);

    /* 6) Compute initial HS/LS duties with software dead-time, clamp, and 7) apply atomically */
    prv_prepareInitialDuties();
    IfxGtm_Tom_Timer_disableUpdate(&g_drv.timer);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_drv.pwm, (float32 *)&g_drv.dutyAllPct[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_drv.timer);

    g_drv.initialized = TRUE;
}

void updateGtmTomPwmDutyCycles(void)
{
    if (g_drv.initialized == FALSE)
    {
        return; /* Early-exit if init failed/not done */
    }

    /* 1) Read active timer period in ticks */
    Ifx_TimerValue periodTicks = IfxGtm_Tom_Timer_getPeriod(&g_drv.timer);

    /* 2) Add +10% to each HS accumulator with wrap at 100% */
    for (uint8 i = 0u; i < 3u; i++)
    {
        g_drv.dutyHighPct[i] += DUTY_UPDATE_POLICY_STEP_PERCENT;
        if (g_drv.dutyHighPct[i] >= 100.0f)
        {
            g_drv.dutyHighPct[i] -= 100.0f; /* wrap */
        }
    }

    /* 3) Convert software dead-time (0.5us) to duty fraction in percent */
    float32 fxclkHz = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0, TRUE);
    float32 deadTimeTicks = fxclkHz * (TIMING_SOFTWARE_DEAD_TIME_US * 1.0e-6f);
    float32 deadTimePct = (periodTicks > 0) ? ((deadTimeTicks / (float32)periodTicks) * 100.0f) : 0.0f;

    /* 4) For each complementary pair, LS = 100 - HS - deadTimePct; clamp [0,100] */
    /* HS channels */
    g_drv.dutyAllPct[CH_IDX_U_HS] = clampFloat32(g_drv.dutyHighPct[0], 0.0f, 100.0f);
    g_drv.dutyAllPct[CH_IDX_V_HS] = clampFloat32(g_drv.dutyHighPct[1], 0.0f, 100.0f);
    g_drv.dutyAllPct[CH_IDX_W_HS] = clampFloat32(g_drv.dutyHighPct[2], 0.0f, 100.0f);
    /* LS channels */
    g_drv.dutyAllPct[CH_IDX_U_LS] = clampFloat32(100.0f - g_drv.dutyHighPct[0] - deadTimePct, 0.0f, 100.0f);
    g_drv.dutyAllPct[CH_IDX_V_LS] = clampFloat32(100.0f - g_drv.dutyHighPct[1] - deadTimePct, 0.0f, 100.0f);
    g_drv.dutyAllPct[CH_IDX_W_LS] = clampFloat32(100.0f - g_drv.dutyHighPct[2] - deadTimePct, 0.0f, 100.0f);

    /* 5) Duties are already ordered to match configured channels (CH1..CH6) */

    /* 6) Gate the update for synchronous latch and apply all six duties */
    IfxGtm_Tom_Timer_disableUpdate(&g_drv.timer);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_drv.pwm, (float32 *)&g_drv.dutyAllPct[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_drv.timer);
}
