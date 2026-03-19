/*
 * Module: GTM_TOM_3_Phase_Inverter_PWM
 * File: gtm_tom_3_phase_inverter_pwm.c
 *
 * Production implementation of 3-phase inverter PWM using GTM TOM and
 * IfxGtm_Tom_PwmHl per authoritative iLLD reference patterns.
 *
 * Mandatory rules followed:
 * - Uses generic PinMap header (no family-specific suffix)
 * - No watchdog handling here (belongs only in CpuN_Main.c)
 * - Clocks enabled via IfxGtm_Cmu APIs
 * - Complementary outputs configured via PwmHl ccx/coutx arrays
 * - Dead-time and min-pulse configured in PwmHl config
 * - Synchronous updates via Timer_disableUpdate/applyUpdate
 * - Error handling: checks boolean returns and guards runtime with s_initialized
 */

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ========================= Requirements-derived configuration ========================= */
#define TIMING_PWM_FREQUENCY_HZ                 (20000.0f)         /* 20 kHz */
#define TIMING_DEADTIME_US                      (0.5f)             /* 0.5 microseconds */
#define TIMING_MIN_PULSE_US                     (1.0f)             /* 1.0 microseconds */
#define TIMING_USE_HARDWARE_DTM                 (1)                /* True */
#define TIMING_UPDATE_SYNCHRONOUS_TGC           (1)                /* True */

#define INITIAL_DUTY_PERCENT_U                  (25.0f)            /* percent */
#define INITIAL_DUTY_PERCENT_V                  (50.0f)            /* percent */
#define INITIAL_DUTY_PERCENT_W                  (75.0f)            /* percent */

#define DUTY_UPDATE_POLICY_INCREMENT_PERCENT    (10.0f)            /* +10 percentage points per call */
#define DUTY_UPDATE_POLICY_WRAP_0_TO_100        (1)                /* True */
/* Extreme duty handling policy: clamp to minPulse and 2*deadtime (implemented below) */

/* Optional clock targets (documented values; actual FXCLK is derived by CMU): */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ          (300.0f)
#define CLOCK_GTM_FXCLK0_FREQ_MHZ               (100.0f)

/* ========================= Hardware resource and pin mapping ========================= */
/*
 * Board requirement: Keep TOM1 pins
 * - U: TOM1 CH2/P00.3 & CH1/P00.2
 * - V: TOM1 CH4/P00.5 & CH3/P00.4
 * - W: TOM1 CH6/P00.7 & CH5/P00.6
 */
#define PHASE_U_HS   &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS   &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS   &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS   &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS   &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS   &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

#define GTM_TOM_MASTER                IfxGtm_Tom_1
#define GTM_TOM_MASTER_TIMER_CH       IfxGtm_Tom_Ch_0  /* Master timer channel */

/* ========================= Driver state ========================= */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                 /* Master TOM timer */
    IfxGtm_Tom_PwmHl   pwm;                   /* Complementary PWM driver */
    Ifx_TimerValue     pwmOnTimes[3];         /* On-times (timer ticks) for U/V/W */
    float32            dutyNorm[3];           /* Normalized [0..1] duty for U/V/W */
    float32            deadtime_s;            /* Dead-time in seconds */
    float32            minPulse_s;            /* Minimum pulse in seconds */
} GtmTom3phDrv;

static GtmTom3phDrv s_drv;
static boolean      s_initialized = FALSE;

/* ========================= Internal utilities ========================= */
/** Clamp requested normalized duty [0..1] to the safe range considering minPulse and DTM. */
static inline float32 clampDutyToSafeRange(float32 requestDuty, float32 minPulse_s, float32 deadtime_s, float32 period_s)
{
    float32 safeMin = (minPulse_s + (2.0f * deadtime_s)) / period_s;
    if (safeMin < 0.0f) { safeMin = 0.0f; }
    if (safeMin > 0.5f) { safeMin = 0.5f; } /* pathological guard */
    float32 safeMax = 1.0f - safeMin;

    float32 d = requestDuty;
    if (d < safeMin) { d = safeMin; }
    if (d > safeMax) { d = safeMax; }
    return d;
}

/* ========================= Public API implementations ========================= */
/**
 * Initialize GTM/TOM for 3-phase complementary, center-aligned PWM at 20 kHz.
 * Sequence per reference:
 *  1) IfxGtm_enable
 *  2) IfxGtm_Cmu_*: set GCLK, set CLK0 (documentary), enable FXCLK
 *  3) Configure TOM timer (fxclk0, TOM1, CH0 master)
 *  4) Configure PwmHl with complementary pins, dead-time, min-pulse
 *  5) Set mode center-aligned, update input frequency
 *  6) Start timer, program initial duties 25/50/75 percent via synchronous update
 */
void IfxGtm_Tom_PwmHl_init(void)
{
    boolean ok;

    /* 1) Enable GTM module */
    IfxGtm_enable(&MODULE_GTM);

    /* 2) CMU clock setup (reference pattern) */
    {
        float32 gtmModFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, gtmModFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 3) Configure TOM1 master timer (CH0) at 20 kHz using Fxclk0 */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

        timerCfg.base.frequency  = TIMING_PWM_FREQUENCY_HZ;
        timerCfg.clock           = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
        timerCfg.tom             = GTM_TOM_MASTER;
        timerCfg.timerChannel    = GTM_TOM_MASTER_TIMER_CH;
        timerCfg.triggerOut      = NULL_PTR; /* Not used */

        ok = IfxGtm_Tom_Timer_init(&s_drv.timer, &timerCfg);
        if (!ok)
        {
            s_initialized = FALSE;
            return; /* Early exit on failure */
        }
    }

    /* 4) Configure complementary PWM (PwmHl) with dead-time and min-pulse */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlCfg;
        IfxGtm_Tom_ToutMapP ccx[] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
        IfxGtm_Tom_ToutMapP coutx[] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlCfg);

        pwmHlCfg.base.channelCount    = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
        pwmHlCfg.base.deadtime        = (TIMING_DEADTIME_US * 1.0e-6f);
        pwmHlCfg.base.minPulse        = (TIMING_MIN_PULSE_US * 1.0e-6f);
        pwmHlCfg.base.outputMode      = IfxPort_OutputMode_pushPull;
        pwmHlCfg.base.outputDriver    = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlCfg.base.ccxActiveState  = Ifx_ActiveState_high;
        pwmHlCfg.base.coutxActiveState= Ifx_ActiveState_high;

        pwmHlCfg.ccx   = ccx;     /* High-side pins */
        pwmHlCfg.coutx = coutx;   /* Low-side pins (complementary) */
        pwmHlCfg.timer = &s_drv.timer;
        pwmHlCfg.tom   = GTM_TOM_MASTER;

        ok = IfxGtm_Tom_PwmHl_init(&s_drv.pwm, &pwmHlCfg);
        if (!ok)
        {
            s_initialized = FALSE;
            return; /* Early exit on failure */
        }

        /* Store timing constraints for runtime safety clamping */
        s_drv.deadtime_s = pwmHlCfg.base.deadtime;
        s_drv.minPulse_s = pwmHlCfg.base.minPulse;

        /* Center-aligned mode as required */
        (void)IfxGtm_Tom_PwmHl_setMode(&s_drv.pwm, Ifx_Pwm_Mode_centerAligned);

        /* Ensure timer input frequency is latched after configuration */
        IfxGtm_Tom_Timer_updateInputFrequency(&s_drv.timer);
    }

    /* 5) Start timer */
    IfxGtm_Tom_Timer_run(&s_drv.timer);

    /* 6) Program initial duties 25% / 50% / 75% with synchronous update */
    {
        const float32 dU = INITIAL_DUTY_PERCENT_U / 100.0f;
        const float32 dV = INITIAL_DUTY_PERCENT_V / 100.0f;
        const float32 dW = INITIAL_DUTY_PERCENT_W / 100.0f;

        s_drv.dutyNorm[0] = dU;
        s_drv.dutyNorm[1] = dV;
        s_drv.dutyNorm[2] = dW;

        const Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
        s_drv.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[0]);
        s_drv.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[1]);
        s_drv.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[2]);

        IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
    }

    s_initialized = TRUE;
}

/**
 * Runtime duty update implementing the behavior description:
 * - Increment normalized duty by +0.10 (10%) per phase
 * - Wrap [0..1): if >= 1.0 -> 0.0
 * - Compute safeMin = (minPulse + 2*deadtime)/period_s, safeMax = 1 - safeMin
 * - Clamp requested duties into [safeMin, safeMax]
 * - Compute on-times (ticks) and apply via synchronous update
 *
 * Timing policy: caller (Cpu0 loop) invokes this every 500 ms
 */
void IfxGtm_Tom_PwmHl_setDuty(void)
{
    if (!s_initialized)
    {
        return; /* Early exit if init failed or not done */
    }

    /* 1) Increment each duty by +10 percentage points with wrap */
    const float32 step = (DUTY_UPDATE_POLICY_INCREMENT_PERCENT / 100.0f); /* 0.10 */
    for (uint8 i = 0U; i < 3U; i++)
    {
        s_drv.dutyNorm[i] += step;
        if (s_drv.dutyNorm[i] >= 1.0f)
        {
            s_drv.dutyNorm[i] = 0.0f; /* wrap */
        }
    }

    /* 2) Compute period in seconds from configured 20 kHz */
    const float32 period_s = 1.0f / TIMING_PWM_FREQUENCY_HZ; /* 50 us */

    /* 3) Clamp to safe range considering minPulse + 2*deadtime */
    for (uint8 i = 0U; i < 3U; i++)
    {
        s_drv.dutyNorm[i] = clampDutyToSafeRange(s_drv.dutyNorm[i], s_drv.minPulse_s, s_drv.deadtime_s, period_s);
    }

    /* 4) Convert to on-times (ticks) and apply synchronously */
    const Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
    s_drv.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[0]);
    s_drv.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[1]);
    s_drv.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[2]);

    IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.pwmOnTimes);
    IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
}
