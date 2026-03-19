/*
 * gtm_tom_3_phase_inverter_pwm.c
 * GTM TOM 3-Phase Inverter PWM - production implementation
 */
#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD headers - use generic includes only */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* =====================================================================
 * Pin assignments (validated for TC3xx, TOM1 on Port 00 per requirements)
 * U: TOM1 CH2/P00.3 (HS) & CH1/P00.2 (LS)
 * V: TOM1 CH4/P00.5 (HS) & CH3/P00.4 (LS)
 * W: TOM1 CH6/P00.7 (HS) & CH5/P00.6 (LS)
 * ===================================================================== */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

#define NUM_OF_CHANNELS   (3u)

/* Derived timing constants */
#define PWM_DEADTIME_S     (TIMING_DEADTIME_US * 1.0e-6f)
#define PWM_MIN_PULSE_S    (TIMING_MIN_PULSE_US * 1.0e-6f)

/* Internal driver/application state */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                 /* TOM master timer (TOM1 CH0) */
    IfxGtm_Tom_PwmHl   pwm;                   /* PWMHL driver */
    Ifx_TimerValue     tOn[NUM_OF_CHANNELS];  /* On-time ticks per phase */
    float32            dutyNorm[NUM_OF_CHANNELS]; /* Normalized 0..1 duty per phase */
} GtmTom3phDrv_t;

static GtmTom3phDrv_t s_drv;
static boolean        s_initialized = FALSE;

/* Local helpers */
static float32 clampf(float32 v, float32 lo, float32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* =====================================================================
 * IfxGtm_Tom_PwmHl_init - Module initialization
 * ===================================================================== */
void IfxGtm_Tom_PwmHl_init(void)
{
    /* 1) Enable GTM module */
    IfxGtm_enable(&MODULE_GTM);

    /* 2) CMU clock setup: GCLK and CLK0; enable FXCLK so Fxclk0 is available */
    {
        float32 gtmClusterFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM); /* Cluster frequency */
        /* Keep GCLK at cluster frequency; set CLK0 to requested 100 MHz fxclk base */
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, gtmClusterFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, (CLOCK_GTM_FXCLK0_FREQ_MHZ * 1.0e6f));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 3) Configure and start TOM1 CH0 master timer at 20 kHz, FXCLK0 */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

        timerCfg.base.frequency  = TIMING_PWM_FREQUENCY_HZ;               /* 20 kHz */
        timerCfg.clock           = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;        /* Fxclk0 time base */
        timerCfg.tom             = IfxGtm_Tom_1;                          /* TOM1 */
        timerCfg.timerChannel    = IfxGtm_Tom_Ch_0;                       /* CH0 as master */

        if (IfxGtm_Tom_Timer_init(&s_drv.timer, &timerCfg) == FALSE)
        {
            /* Init failed: leave uninitialized */
            return;
        }
    }

    /* 4) Configure PWMHL for 3 complementary pairs with DTM dead-time and min pulse */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlCfg;
        IfxGtm_Tom_ToutMapP ccx[NUM_OF_CHANNELS] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
        IfxGtm_Tom_ToutMapP coutx[NUM_OF_CHANNELS] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlCfg);

        pwmHlCfg.base.channelCount     = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
        pwmHlCfg.base.deadtime         = PWM_DEADTIME_S;                      /* 0.5 us */
        pwmHlCfg.base.minPulse         = PWM_MIN_PULSE_S;                     /* 1.0 us */
        pwmHlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;
        pwmHlCfg.base.outputDriver     = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlCfg.base.ccxActiveState   = Ifx_ActiveState_high;
        pwmHlCfg.base.coutxActiveState = Ifx_ActiveState_high;

        pwmHlCfg.ccx   = ccx;               /* High-side pins */
        pwmHlCfg.coutx = coutx;             /* Low-side pins */
        pwmHlCfg.timer = &s_drv.timer;      /* Link to master timer */
        pwmHlCfg.tom   = IfxGtm_Tom_1;      /* TOM1 */

        if (IfxGtm_Tom_PwmHl_init(&s_drv.pwm, &pwmHlCfg) == FALSE)
        {
            /* Init failed: leave uninitialized */
            return;
        }

        if (IfxGtm_Tom_PwmHl_setMode(&s_drv.pwm, Ifx_Pwm_Mode_centerAligned) == FALSE)
        {
            /* Mode set failed: leave uninitialized */
            return;
        }

        /* Update any derived input frequencies */
        IfxGtm_Tom_Timer_updateInputFrequency(&s_drv.timer);
    }

    /* 5) Start the timer (PWM will run; updates are synchronized with disable/apply) */
    IfxGtm_Tom_Timer_run(&s_drv.timer);

    /* 6) Program initial duties (25/50/75%) as a synchronous update (non-immediate) */
    {
        float32 dutyU = INITIAL_DUTY_PERCENT_U / 100.0f;
        float32 dutyV = INITIAL_DUTY_PERCENT_V / 100.0f;
        float32 dutyW = INITIAL_DUTY_PERCENT_W / 100.0f;

        s_drv.dutyNorm[0] = dutyU;
        s_drv.dutyNorm[1] = dutyV;
        s_drv.dutyNorm[2] = dutyW;

        {
            Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
            s_drv.tOn[0] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[0]);
            s_drv.tOn[1] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[1]);
            s_drv.tOn[2] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[2]);
        }

        IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.tOn);
        IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
    }

    s_initialized = TRUE;
}

/* =====================================================================
 * IfxGtm_Tom_PwmHl_setDuty - Runtime duty update (500 ms cadence managed by caller)
 * ===================================================================== */
void IfxGtm_Tom_PwmHl_setDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit on not initialized */
    }

    /* 1) Increment each duty by +10 percentage points (normalized +0.10) with wrap at 1.0 */
    {
        const float32 step = (DUTY_UPDATE_POLICY_INCREMENT_PERCENT / 100.0f);
        for (uint8 i = 0; i < NUM_OF_CHANNELS; i++)
        {
            s_drv.dutyNorm[i] += step;
            if (s_drv.dutyNorm[i] >= 1.0f)
            {
                s_drv.dutyNorm[i] = 0.0f; /* wrap to 0.0 */
            }
        }
    }

    /* 2) Compute safe duty range from timing constraints */
    float32 period_s;
    {
        float32 f_pwm = IfxGtm_Tom_Timer_getFrequency(&s_drv.timer); /* Hz */
        if (f_pwm <= 0.0f)
        {
            return; /* Safety: can't compute period */
        }
        period_s = 1.0f / f_pwm;
    }

    const float32 safeMinDuty = clampf((PWM_MIN_PULSE_S + (2.0f * PWM_DEADTIME_S)) / period_s, 0.0f, 0.5f);
    const float32 safeMaxDuty = 1.0f - safeMinDuty;

    /* 3) Clamp duties into [safeMinDuty, safeMaxDuty] */
    for (uint8 i = 0; i < NUM_OF_CHANNELS; i++)
    {
        s_drv.dutyNorm[i] = clampf(s_drv.dutyNorm[i], safeMinDuty, safeMaxDuty);
    }

    /* 4) Convert to on-time ticks and apply synchronously */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
        s_drv.tOn[0] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[0]);
        s_drv.tOn[1] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[1]);
        s_drv.tOn[2] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[2]);

        IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.tOn);
        IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
    }
}
