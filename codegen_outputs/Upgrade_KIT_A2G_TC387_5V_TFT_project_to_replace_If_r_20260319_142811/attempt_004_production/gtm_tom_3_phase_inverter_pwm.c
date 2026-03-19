/*
 * GTM TOM 3-Phase Inverter PWM - Production Source
 * Target: AURIX TC3xx (TC387)
 * Peripheral: GTM TOM PWM (complementary, center-aligned) using IfxGtm_Tom_PwmHl
 *
 * Implementation notes:
 * - Follows reference iLLD patterns (IfxGtm_Tom_Timer + IfxGtm_Tom_PwmHl)
 * - Uses TOM1 CH0 master timer, Fxclk0=100 MHz, 20 kHz center-aligned PWM
 * - Complementary outputs with hardware deadtime (0.5 us) and min pulse (1.0 us)
 * - Initial duties: U=25%, V=50%, W=75%
 * - Update API increments all by +10% each call, wraps 0..100%, clamps to safe range
 * - Synchronous update via Timer_disableUpdate/applyUpdate (TGC)
 * - No watchdog operations in this module (handled in CpuN_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxPort.h"

/* =============================== LOCAL STATE =============================== */

typedef struct
{
    IfxGtm_Tom_Timer   timer;                 /* Master TOM1 CH0 timer */
    IfxGtm_Tom_PwmHl   pwm;                   /* Complementary PWM driver */
    Ifx_TimerValue     pwmOnTimes[NUM_OF_CHANNELS];
    float32            dutyFrac[NUM_OF_CHANNELS];  /* 0.0f..1.0f normalized */
} GtmTom3PhasePwm_T;

static GtmTom3PhasePwm_T s_pwm = {0};
static boolean           s_initialized = FALSE;

/* ============================== LOCAL HELPERS ============================== */
static inline float32 clampf(float32 v, float32 lo, float32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* ================================ INIT API ================================= */
/* EXACT signature from SW Detailed Design */
void IfxGtm_Tom_PwmHl_init(void)
{
    boolean ok;

    /* 1) Enable GTM module */
    IfxGtm_enable(&MODULE_GTM);

    /* 2) CMU clock setup: set GCLK and Fxclk0 to required values and enable FXCLK */
    {
        float32 gtmModuleHz = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM); /* Cluster0 freq */
        /* Keep GCLK at module frequency for best resolution */
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, gtmModuleHz);
        /* Set Fxclk0 explicitly to 100 MHz as per requirement */
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, (CLOCK_GTM_FXCLK0_FREQ_MHZ * 1.0e6f));
        /* Enable FXCLK domain */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 3) Configure TOM1 CH0 master timer at 20 kHz center-aligned base */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

        timerCfg.base.frequency  = TIMING_PWM_FREQUENCY_HZ;           /* 20 kHz */
        timerCfg.clock           = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;    /* Fxclk0 time base */
        timerCfg.tom             = IfxGtm_Tom_1;                      /* TOM1 */
        timerCfg.timerChannel    = IfxGtm_Tom_Ch_0;                   /* Master CH0 */

        ok = IfxGtm_Tom_Timer_init(&s_pwm.timer, &timerCfg);
        if (ok == FALSE)
        {
            return; /* Early exit on failure */
        }
    }

    /* 4) Configure complementary PWM HL on TOM1 pairs (U,V,W) */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlCfg;

        IfxGtm_Tom_ToutMapP ccx[NUM_OF_CHANNELS] =
        {
            PHASE_U_HS, /* U high-side */
            PHASE_V_HS, /* V high-side */
            PHASE_W_HS  /* W high-side */
        };
        IfxGtm_Tom_ToutMapP coutx[NUM_OF_CHANNELS] =
        {
            PHASE_U_LS, /* U low-side  */
            PHASE_V_LS, /* V low-side  */
            PHASE_W_LS  /* W low-side  */
        };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlCfg);
        pwmHlCfg.base.channelCount    = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
        pwmHlCfg.base.deadtime        = (TIMING_DEADTIME_US * 1.0e-6f); /* seconds */
        pwmHlCfg.base.minPulse        = (TIMING_MIN_PULSE_US * 1.0e-6f); /* seconds */
        pwmHlCfg.base.outputMode      = IfxPort_OutputMode_pushPull;
        pwmHlCfg.base.outputDriver    = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        /* Use active-high for both CCX and COUTX, complementary handled internally */
        pwmHlCfg.base.ccxActiveState  = Ifx_ActiveState_high;
        pwmHlCfg.base.coutxActiveState= Ifx_ActiveState_high;

        pwmHlCfg.ccx   = ccx;
        pwmHlCfg.coutx = coutx;
        pwmHlCfg.timer = &s_pwm.timer;
        pwmHlCfg.tom   = IfxGtm_Tom_1; /* TOM1 resource */

        ok = IfxGtm_Tom_PwmHl_init(&s_pwm.pwm, &pwmHlCfg);
        if (ok == FALSE)
        {
            return; /* Early exit on failure */
        }

        /* Center-aligned operation */
        ok = IfxGtm_Tom_PwmHl_setMode(&s_pwm.pwm, Ifx_Pwm_Mode_centerAligned);
        if (ok == FALSE)
        {
            return; /* Early exit on failure */
        }

        /* Make sure timer driver has updated frequency/period based on CMU */
        IfxGtm_Tom_Timer_updateInputFrequency(&s_pwm.timer);

        /* 5) Start timer (enables outputs via PwmHl driver resources) */
        IfxGtm_Tom_Timer_run(&s_pwm.timer);
    }

    /* 6) Program initial duties (25%, 50%, 75%) with synchronous update */
    {
        const float32 periodTicks = (float32)s_pwm.pwm.timer->base.period; /* timer period in ticks */

        s_pwm.dutyFrac[0] = (INITIAL_DUTY_PERCENT_U / 100.0f);
        s_pwm.dutyFrac[1] = (INITIAL_DUTY_PERCENT_V / 100.0f);
        s_pwm.dutyFrac[2] = (INITIAL_DUTY_PERCENT_W / 100.0f);

        s_pwm.pwmOnTimes[0] = (Ifx_TimerValue)(periodTicks * s_pwm.dutyFrac[0]);
        s_pwm.pwmOnTimes[1] = (Ifx_TimerValue)(periodTicks * s_pwm.dutyFrac[1]);
        s_pwm.pwmOnTimes[2] = (Ifx_TimerValue)(periodTicks * s_pwm.dutyFrac[2]);

        /* Synchronous TGC update: disable, write, apply */
        IfxGtm_Tom_Timer_disableUpdate(&s_pwm.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&s_pwm.pwm, s_pwm.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&s_pwm.timer);
    }

    s_initialized = TRUE;
}

/* =============================== UPDATE API ================================ */
/* EXACT signature from SW Detailed Design */
void IfxGtm_Tom_PwmHl_setDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if init failed/not done */
    }

    /* 1) Increment each duty by +10 percentage points with wrap 0..100 */
    {
        const float32 step = (DUTY_UPDATE_POLICY_INCREMENT_PERCENT / 100.0f); /* 0.10 */
        for (uint8 i = 0; i < NUM_OF_CHANNELS; i++)
        {
            s_pwm.dutyFrac[i] += step;
            if (s_pwm.dutyFrac[i] >= 1.0f)
            {
                s_pwm.dutyFrac[i] = 0.0f; /* wrap */
            }
        }
    }

    /* 2) Compute safe duty range using minPulse and 2*deadtime */
    {
        const float32 period_s   = (1.0f / TIMING_PWM_FREQUENCY_HZ);                     /* seconds */
        const float32 minPulse_s = (TIMING_MIN_PULSE_US * 1.0e-6f);
        const float32 deadtime_s = (TIMING_DEADTIME_US * 1.0e-6f);
        const float32 safeMin    = (minPulse_s + (2.0f * deadtime_s)) / period_s;
        const float32 safeMax    = 1.0f - safeMin;

        for (uint8 i = 0; i < NUM_OF_CHANNELS; i++)
        {
            s_pwm.dutyFrac[i] = clampf(s_pwm.dutyFrac[i], safeMin, safeMax);
        }
    }

    /* 3) Write updated on-times synchronously at next period boundary */
    {
        const float32 periodTicks = (float32)s_pwm.pwm.timer->base.period; /* timer period in ticks */

        s_pwm.pwmOnTimes[0] = (Ifx_TimerValue)(periodTicks * s_pwm.dutyFrac[0]);
        s_pwm.pwmOnTimes[1] = (Ifx_TimerValue)(periodTicks * s_pwm.dutyFrac[1]);
        s_pwm.pwmOnTimes[2] = (Ifx_TimerValue)(periodTicks * s_pwm.dutyFrac[2]);

        IfxGtm_Tom_Timer_disableUpdate(&s_pwm.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&s_pwm.pwm, s_pwm.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&s_pwm.timer);
    }
}
