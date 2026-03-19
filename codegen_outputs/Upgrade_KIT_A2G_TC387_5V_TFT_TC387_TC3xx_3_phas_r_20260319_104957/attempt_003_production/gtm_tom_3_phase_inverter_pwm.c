/*
 * gtm_tom_3_phase_inverter_pwm.c
 * GTM TOM 3-Phase Inverter PWM Driver (TC3xx / TC387)
 *
 * Implements the EXACT API contract and behavior from SW Detailed Design.
 * Uses real iLLD drivers and required initialization sequence.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD headers (generic) */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* Internal driver state */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                         /* Common TOM timebase */
    IfxGtm_Pwm         pwm;                           /* Unified PWM driver handle */
    IfxGtm_Pwm_Channel channels[NUM_OF_CHANNELS];     /* Channel data after init */
    float32            dutyPercent[NUM_OF_CHANNELS];  /* Last applied duties (six channels) */
    float32            hsDutyPercent[3];              /* High-side duty accumulators (U,V,W) */
} GtmTom3phInv_Driver;

static GtmTom3phInv_Driver s_drv;
static boolean s_initialized = FALSE;

/* Local helpers */
static float32 clampFloat32(float32 value, float32 minVal, float32 maxVal)
{
    float32 v = value;
    if (v < minVal) { v = minVal; }
    if (v > maxVal) { v = maxVal; }
    return v;
}

/*
 * Initialize the GTM-based three-phase PWM inverter.
 * Behavior per SW Detailed Design
 */
void initGtmTomPwm(void)
{
    s_initialized = FALSE; /* reset init state */

    /* 1) Enable GTM module and FXCLK domain (retain existing clock setup) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
    }
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* 2) Create and initialize a TOM timer configuration for 20 kHz, center-aligned, FXCLK0 */
    {
        IfxGtm_Tom_Timer_Config timerConfig;
        IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);

        /* Timebase configuration per requirements */
        timerConfig.base.frequency = TIMEBASE_FREQUENCY_HZ;              /* 20 kHz */
        timerConfig.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;     /* FXCLK0 */
        timerConfig.tom            = IfxGtm_Tom_1;                       /* GTM.TOM1 timebase */
        timerConfig.timerChannel   = IfxGtm_Tom_Ch_0;                    /* Use CH0 as master timebase */

        if (IfxGtm_Tom_Timer_init(&s_drv.timer, &timerConfig) == FALSE)
        {
            return; /* Error handling: early exit on failure */
        }

        IfxGtm_Tom_Timer_updateInputFrequency(&s_drv.timer);
    }

    /* 3) Configure six PWM output channels and map to assigned output pins (PinMap API) */
    /*    Order channels by TOM1 CH1..CH6: U_LS, U_HS, V_LS, V_HS, W_LS, W_HS */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) Initialize the unified multi-channel PWM driver with six channels */
    {
        IfxGtm_Pwm_Config        config;
        IfxGtm_Pwm_ChannelConfig chCfg[NUM_OF_CHANNELS];
        uint8 i;

        IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);
        for (i = 0U; i < NUM_OF_CHANNELS; i++)
        {
            IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
        }

        /* Explicit sync/clock/frequency/alignment settings */
        config.cluster                 = IfxGtm_Cluster_0;
        config.subModule               = IfxGtm_Pwm_SubModule_tom;
        config.alignment               = IfxGtm_Pwm_Alignment_center;    /* Center-aligned (up-down) */
        config.syncStart               = TRUE;
        config.syncUpdateEnabled       = TRUE;                            /* Shadow update at period boundary */
        config.numChannels             = NUM_OF_CHANNELS;                 /* 6 channels */
        config.channels                = &chCfg[0];
        config.frequency               = TIMING_PWM_FREQUENCY_HZ;         /* 20 kHz */
        config.clockSource.tom         = IfxGtm_Cmu_Fxclk_0;              /* TOM uses Fxclk enum */

        IfxGtm_Pwm_init(&s_drv.pwm, &s_drv.channels[0], &config);
    }

    /* 5) Start the TOM timer and synchronized PWM channels */
    IfxGtm_Tom_Timer_run(&s_drv.timer);
    IfxGtm_Pwm_startSyncedChannels(&s_drv.pwm);

    /* 6) Compute initial HS duties and LS as complement minus SW dead-time (in percent) */
    {
        float32 periodTicks = (float32)IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
        float32 fxclkHz     = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0, TRUE);
        float32 deadTicks   = (TIMING_SOFTWARE_DEAD_TIME_US * 1.0e-6f) * fxclkHz;
        float32 deadPct     = (periodTicks > 0.0f) ? ((deadTicks / periodTicks) * 100.0f) : 0.0f;

        /* Initialize HS accumulators */
        s_drv.hsDutyPercent[0] = INITIAL_DUTY_PERCENT_U; /* U */
        s_drv.hsDutyPercent[1] = INITIAL_DUTY_PERCENT_V; /* V */
        s_drv.hsDutyPercent[2] = INITIAL_DUTY_PERCENT_W; /* W */

        /* Build six-element duty array in TOM1 CH1..CH6 order */
        /* U_LS (CH1) */ s_drv.dutyPercent[0] = clampFloat32(100.0f - s_drv.hsDutyPercent[0] - deadPct, 0.0f, 100.0f);
        /* U_HS (CH2) */ s_drv.dutyPercent[1] = clampFloat32(s_drv.hsDutyPercent[0], 0.0f, 100.0f);
        /* V_LS (CH3) */ s_drv.dutyPercent[2] = clampFloat32(100.0f - s_drv.hsDutyPercent[1] - deadPct, 0.0f, 100.0f);
        /* V_HS (CH4) */ s_drv.dutyPercent[3] = clampFloat32(s_drv.hsDutyPercent[1], 0.0f, 100.0f);
        /* W_LS (CH5) */ s_drv.dutyPercent[4] = clampFloat32(100.0f - s_drv.hsDutyPercent[2] - deadPct, 0.0f, 100.0f);
        /* W_HS (CH6) */ s_drv.dutyPercent[5] = clampFloat32(s_drv.hsDutyPercent[2], 0.0f, 100.0f);

        /* 7) Apply all six initial duty values atomically */
        IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
        IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, &s_drv.dutyPercent[0]);
        IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
    }

    s_initialized = TRUE;
}

/*
 * Periodic duty update per SW Detailed Design
 */
void updateGtmTomPwmDutyCycles(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit if init failed or not performed */
    }

    /* 1) Read active timer period in ticks */
    float32 periodTicks = (float32)IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);

    /* 2) Compute +10% step and wrap HS duties into [0,100) */
    s_drv.hsDutyPercent[0] += DUTY_UPDATE_POLICY_STEP_PERCENT;
    s_drv.hsDutyPercent[1] += DUTY_UPDATE_POLICY_STEP_PERCENT;
    s_drv.hsDutyPercent[2] += DUTY_UPDATE_POLICY_STEP_PERCENT;

    if (s_drv.hsDutyPercent[0] >= 100.0f) { s_drv.hsDutyPercent[0] -= 100.0f; }
    if (s_drv.hsDutyPercent[1] >= 100.0f) { s_drv.hsDutyPercent[1] -= 100.0f; }
    if (s_drv.hsDutyPercent[2] >= 100.0f) { s_drv.hsDutyPercent[2] -= 100.0f; }

    /* 3) Convert 0.5us SW dead-time into ticks and percentage of period */
    float32 fxclkHz   = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0, TRUE);
    float32 deadTicks = (TIMING_SOFTWARE_DEAD_TIME_US * 1.0e-6f) * fxclkHz;
    float32 deadPct   = (periodTicks > 0.0f) ? ((deadTicks / periodTicks) * 100.0f) : 0.0f;

    /* 4) For each complementary pair: LS = 100 - HS - deadPct; clamp [0,100] */
    /* 5) Prepare six-element duty array ordered to match configured TOM1 CH1..CH6 */
    /* U_LS (CH1) */ s_drv.dutyPercent[0] = clampFloat32(100.0f - s_drv.hsDutyPercent[0] - deadPct, 0.0f, 100.0f);
    /* U_HS (CH2) */ s_drv.dutyPercent[1] = clampFloat32(s_drv.hsDutyPercent[0], 0.0f, 100.0f);
    /* V_LS (CH3) */ s_drv.dutyPercent[2] = clampFloat32(100.0f - s_drv.hsDutyPercent[1] - deadPct, 0.0f, 100.0f);
    /* V_HS (CH4) */ s_drv.dutyPercent[3] = clampFloat32(s_drv.hsDutyPercent[1], 0.0f, 100.0f);
    /* W_LS (CH5) */ s_drv.dutyPercent[4] = clampFloat32(100.0f - s_drv.hsDutyPercent[2] - deadPct, 0.0f, 100.0f);
    /* W_HS (CH6) */ s_drv.dutyPercent[5] = clampFloat32(s_drv.hsDutyPercent[2], 0.0f, 100.0f);

    /* 6) Gate update with timer disable/apply to ensure synchronous latch, then apply all six */
    IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, &s_drv.dutyPercent[0]);
    IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
}
