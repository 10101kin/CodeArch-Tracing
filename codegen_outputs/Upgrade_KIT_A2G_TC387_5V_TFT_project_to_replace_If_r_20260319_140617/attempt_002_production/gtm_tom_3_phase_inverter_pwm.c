#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"   /* Generic pin map header (TC3xx family auto-selected) */
#include "IfxPort.h"

/* ========================== Requirements-driven configuration ========================== */
#define NUM_OF_CHANNELS                               (3u)

/* Timing configuration (from requirements) */
#define TIMING_PWM_FREQUENCY_HZ                       (20000.0f)      /* 20 kHz */
#define TIMING_DEADTIME_US                            (0.5f)          /* 0.5 us */
#define TIMING_MIN_PULSE_US                           (1.0f)          /* 1.0 us */
#define TIMING_UPDATE_SYNCHRONOUS_TGC                 (1)             /* TRUE */

/* Clock expectations (from requirements) */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ                (300.0f)
#define CLOCK_GTM_FXCLK0_FREQ_MHZ                     (100.0f)

/* Initial duties in PERCENT (from requirements) */
#define INITIAL_DUTY_PERCENT_U                        (25.0f)
#define INITIAL_DUTY_PERCENT_V                        (50.0f)
#define INITIAL_DUTY_PERCENT_W                        (75.0f)

/* Duty update policy (from requirements) */
#define DUTY_UPDATE_POLICY_INCREMENT_PERCENT          (10.0f)
#define DUTY_UPDATE_POLICY_WRAP_0_TO_100              (1)             /* TRUE */
/* Clamp policy: clamp_to_minPulse_and_deadtime */

/* For reference parity (names used in some patterns) */
#define PWM_MIN_PULSE_TIME                            (1.0f)          /* us, informational */
#define DUTY_25_PERCENT                               (25.0f)
#define DUTY_50_PERCENT                               (50.0f)
#define DUTY_75_PERCENT                               (75.0f)
#define DUTY_STEP                                     (10.0f)         /* percent */

/* ============================= Pin assignment (TC387) ============================= */
/* Keep TOM1 pins (U: TOM1 CH2/P00.3 & CH1/P00.2; V: CH4/P00.5 & CH3/P00.4; W: CH6/P00.7 & CH5/P00.6) */
#define PHASE_U_HS                &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS                &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS                &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS                &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS                &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS                &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* ============================= Driver state ============================= */
typedef struct
{
    IfxGtm_Tom_Timer     timer;                    /* TOM master timer (TOM1 CH0 as master) */
    IfxGtm_Tom_PwmHl     pwm;                      /* Complementary PWM HL driver */
    Ifx_TimerValue       pwmOnTimes[NUM_OF_CHANNELS];
    float32              dutyFrac[NUM_OF_CHANNELS]; /* Normalized 0.0 .. 1.0 per-phase duty */
} GtmTom3PhPwm_Driver;

static GtmTom3PhPwm_Driver s_pwm3ph;
static boolean             s_initialized = FALSE;

/* ============================= Internal helpers ============================= */
static inline float32 clampf(float32 v, float32 lo, float32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* ============================= Public API implementations ============================= */

/*
 * IfxGtm_Tom_PwmHl_init
 * Initialize GTM TOM1 for 3-phase complementary, center-aligned PWM with DTM and minPulse.
 * Sequence per SW Detailed Design and iLLD reference:
 *  1) IfxGtm_enable
 *  2) IfxGtm_Cmu_enableClocks (FXCLK)
 *  3) Configure TOM1 CH0 master timer at 20 kHz using Fxclk0
 *  4) Configure PwmHl: complementary outputs, deadtime, minPulse, push-pull, pad driver
 *  5) Center-aligned mode; update input frequency
 *  6) Program initial duties (25/50/75 %) using synchronous update
 *  7) Start the timer (run)
 */
void IfxGtm_Tom_PwmHl_init(void)
{
    boolean ok;

    /* 1) Enable GTM module */
    IfxGtm_enable(&MODULE_GTM);

    /* 2) CMU clock setup: set GCLK to module frequency and enable FXCLK domain */
    {
        float32 modFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, modFreq);
        /* CLK0 not used by Fxclk timer source, but keep consistent with reference */
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 3) Configure TOM1 CH0 master timer at 20 kHz using Fxclk0 */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

        timerCfg.base.frequency = TIMING_PWM_FREQUENCY_HZ;         /* 20 kHz */
        timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;  /* Fxclk0 time base */
        timerCfg.tom            = IfxGtm_Tom_1;                    /* TOM1 */
        timerCfg.timerChannel   = IfxGtm_Tom_Ch_0;                 /* CH0 as master */

        ok = IfxGtm_Tom_Timer_init(&s_pwm3ph.timer, &timerCfg);
        if (!ok)
        {
            return; /* Early-exit on failure (tests expect no further actions) */
        }
    }

    /* 4) Configure complementary PWM HL with deadtime and minPulse */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlCfg;
        IfxGtm_Tom_ToutMapP     ccx[NUM_OF_CHANNELS] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
        IfxGtm_Tom_ToutMapP     coutx[NUM_OF_CHANNELS] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlCfg);

        pwmHlCfg.base.channelCount     = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
        pwmHlCfg.base.deadtime         = (TIMING_DEADTIME_US * 1.0e-6f);  /* seconds */
        pwmHlCfg.base.minPulse         = (TIMING_MIN_PULSE_US * 1.0e-6f);  /* seconds */
        pwmHlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;
        pwmHlCfg.base.outputDriver     = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlCfg.base.ccxActiveState   = Ifx_ActiveState_high;
        pwmHlCfg.base.coutxActiveState = Ifx_ActiveState_high;

        pwmHlCfg.ccx   = ccx;
        pwmHlCfg.coutx = coutx;
        pwmHlCfg.timer = &s_pwm3ph.timer;
        pwmHlCfg.tom   = IfxGtm_Tom_1;

        ok = IfxGtm_Tom_PwmHl_init(&s_pwm3ph.pwm, &pwmHlCfg);
        if (!ok)
        {
            return; /* Early-exit on failure */
        }

        /* Center-aligned mode as required */
        ok = IfxGtm_Tom_PwmHl_setMode(&s_pwm3ph.pwm, Ifx_Pwm_Mode_centerAligned);
        if (!ok)
        {
            return; /* Early-exit on failure */
        }

        /* Update timer input frequency info (reference pattern) */
        IfxGtm_Tom_Timer_updateInputFrequency(&s_pwm3ph.timer);
    }

    /* 5) Start timer (run) */
    IfxGtm_Tom_Timer_run(&s_pwm3ph.timer);

    /* 6) Program initial duties (25/50/75 %) with synchronous (TGC) update */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_pwm3ph.timer);

        s_pwm3ph.dutyFrac[0] = (INITIAL_DUTY_PERCENT_U * (1.0f / 100.0f));
        s_pwm3ph.dutyFrac[1] = (INITIAL_DUTY_PERCENT_V * (1.0f / 100.0f));
        s_pwm3ph.dutyFrac[2] = (INITIAL_DUTY_PERCENT_W * (1.0f / 100.0f));

        s_pwm3ph.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * s_pwm3ph.dutyFrac[0]);
        s_pwm3ph.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * s_pwm3ph.dutyFrac[1]);
        s_pwm3ph.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * s_pwm3ph.dutyFrac[2]);

        if (TIMING_UPDATE_SYNCHRONOUS_TGC)
        {
            IfxGtm_Tom_Timer_disableUpdate(&s_pwm3ph.timer);
            IfxGtm_Tom_PwmHl_setOnTime(&s_pwm3ph.pwm, s_pwm3ph.pwmOnTimes);
            IfxGtm_Tom_Timer_applyUpdate(&s_pwm3ph.timer);
        }
        else
        {
            IfxGtm_Tom_PwmHl_setOnTime(&s_pwm3ph.pwm, s_pwm3ph.pwmOnTimes);
        }
    }

    s_initialized = TRUE; /* Only after all steps succeed */
}

/*
 * IfxGtm_Tom_PwmHl_setDuty
 * Runtime duty update (caller handles 500 ms cadence):
 *  - Add +10 percentage points to each phase (wrap 0..100%)
 *  - Compute safeMin = (minPulse + 2*deadtime)/period and safeMax = 1 - safeMin
 *  - Clamp each duty to [safeMin, safeMax]
 *  - Apply via synchronous (non-immediate) update at next timer boundary
 */
void IfxGtm_Tom_PwmHl_setDuty(void)
{
    if (!s_initialized)
    {
        return; /* Early-exit if initialization failed or not done */
    }

    const float32 stepFrac = (DUTY_UPDATE_POLICY_INCREMENT_PERCENT * (1.0f / 100.0f));
    float32       minPulse_s = (TIMING_MIN_PULSE_US * 1.0e-6f);
    float32       deadtime_s = (TIMING_DEADTIME_US * 1.0e-6f);

    /* 1) Increment with wrap-around at 1.0 */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        s_pwm3ph.dutyFrac[i] += stepFrac;
        if (DUTY_UPDATE_POLICY_WRAP_0_TO_100 && (s_pwm3ph.dutyFrac[i] >= 1.0f))
        {
            s_pwm3ph.dutyFrac[i] = 0.0f;
        }
    }

    /* 2) Compute safe duty limits based on period (in seconds) */
    {
        float32 freq_Hz   = IfxGtm_Tom_Timer_getFrequency(&s_pwm3ph.timer);
        float32 period_s  = (freq_Hz > 0.0f) ? (1.0f / freq_Hz) : (1.0f / TIMING_PWM_FREQUENCY_HZ);
        float32 safeMin   = (minPulse_s + (2.0f * deadtime_s)) / period_s;
        float32 safeMax   = 1.0f - safeMin;

        /* Defensive bounds if timing constraints exceed period */
        if (safeMin < 0.0f) { safeMin = 0.0f; }
        if (safeMin > 0.5f) { safeMin = 0.5f; }
        if (safeMax < safeMin) { safeMax = safeMin; }

        /* 3) Clamp duties to [safeMin, safeMax] */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            s_pwm3ph.dutyFrac[i] = clampf(s_pwm3ph.dutyFrac[i], safeMin, safeMax);
        }
    }

    /* 4) Convert to on-times and request a synchronous update */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_pwm3ph.timer);
        s_pwm3ph.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * s_pwm3ph.dutyFrac[0]);
        s_pwm3ph.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * s_pwm3ph.dutyFrac[1]);
        s_pwm3ph.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * s_pwm3ph.dutyFrac[2]);

        if (TIMING_UPDATE_SYNCHRONOUS_TGC)
        {
            IfxGtm_Tom_Timer_disableUpdate(&s_pwm3ph.timer);
            IfxGtm_Tom_PwmHl_setOnTime(&s_pwm3ph.pwm, s_pwm3ph.pwmOnTimes);
            IfxGtm_Tom_Timer_applyUpdate(&s_pwm3ph.timer);
        }
        else
        {
            IfxGtm_Tom_PwmHl_setOnTime(&s_pwm3ph.pwm, s_pwm3ph.pwmOnTimes);
        }
    }
}
