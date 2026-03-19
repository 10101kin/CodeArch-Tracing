#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD headers (generic) */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ========================= REQUIREMENTS-DRIVEN CONSTANTS ========================= */
/* Timing and behavior requirements (use EXACT values from requirements) */
#define PWM_FREQ_HZ                               (20000.0f)         /* TIMING_PWM_FREQUENCY_HZ = 20000 */
#define PWM_DEAD_TIME_S                           (0.5e-6f)          /* TIMING_DEADTIME_US = 0.5 us */
#define PWM_MIN_PULSE_TIME_S                      (1.0e-6f)          /* TIMING_MIN_PULSE_US = 1.0 us */
#define GTM_FXCLK0_FREQ_HZ                        (100000000.0f)     /* CLOCK_GTM_FXCLK0_FREQ_MHZ = 100 MHz */

/* Duty policy requirements */
#define DUTY_UPDATE_INTERVAL_MS                   (500U)             /* DUTY_UPDATE_POLICY_UPDATE_INTERVAL_MS = 500 (handled by caller timing) */
#define DUTY_INCREMENT_PERCENT                    (10.0f)            /* DUTY_UPDATE_POLICY_INCREMENT_PERCENT */
#define DUTY_INCREMENT_FRACTION                   (0.10f)

/* Initial duties (as FRACTIONS, used with period ticks like reference pattern) */
#define DUTY_25_PERCENT                           (0.25f)
#define DUTY_50_PERCENT                           (0.50f)
#define DUTY_75_PERCENT                           (0.75f)

/* Reference-style macro kept for compatibility with patterns (fraction in seconds) */
#define PWM_MIN_PULSE_TIME                        (PWM_MIN_PULSE_TIME_S)

/* ============================== PIN ASSIGNMENTS ============================== */
/* Keep TOM1 pins as per user requirement: 
 * U: TOM1 CH2/P00.3 (HS) & CH1/P00.2 (LS)
 * V: TOM1 CH4/P00.5 (HS) & CH3/P00.4 (LS)
 * W: TOM1 CH6/P00.7 (HS) & CH5/P00.6 (LS)
 */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ============================== LOCAL STATE ============================== */
typedef struct
{
    IfxGtm_Tom_Timer   timer;          /* Master timer: TOM1 CH0 (per requirement) */
    IfxGtm_Tom_PwmHl   pwm;            /* Complementary PWM HL driver */
    Ifx_TimerValue     pwmOnTimes[3];  /* On-time per phase in timer ticks */
    float32            dutyFrac[3];    /* Persistent duty per phase in [0.0..1.0] */
} GtmTom3PhasePwm_Driver;

static GtmTom3PhasePwm_Driver s_drv;
static boolean s_initialized = FALSE;

/* ============================== INTERNAL UTILITIES ============================== */
static inline float32 clampf32(float32 v, float32 lo, float32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* ============================== PUBLIC API IMPLEMENTATIONS ============================== */

/*
 * Initialize the GTM for 3-phase complementary, center-aligned PWM using TOM1.
 * Behavior per SW Detailed Design and reference pattern.
 */
void IfxGtm_Tom_PwmHl_init(void)
{
    boolean ok;

    /* 1) Enable GTM module */
    IfxGtm_enable(&MODULE_GTM);

    /* 2) CMU clock configuration: set GCLK and FXCLK0 = 100 MHz, enable FXCLK */
    {
        float32 gtmModuleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, gtmModuleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, GTM_FXCLK0_FREQ_HZ);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 3) Configure TOM1 CH0 as master timer at 20 kHz, FXCLK0 */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

        timerCfg.base.frequency = PWM_FREQ_HZ;                        /* 20 kHz */
        timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;     /* FXCLK0 time base */
        timerCfg.tom            = IfxGtm_Tom_1;                       /* TOM1 */
        timerCfg.timerChannel   = IfxGtm_Tom_Ch_0;                    /* CH0 master */

        ok = IfxGtm_Tom_Timer_init(&s_drv.timer, &timerCfg);
        if (ok == FALSE)
        {
            return; /* Early exit on failure - do not set initialized */
        }
    }

    /* 4-5) Configure complementary PWM HL with dead-time and min pulse, center-aligned */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlCfg;
        IfxGtm_Tom_ToutMapP ccx[3] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
        IfxGtm_Tom_ToutMapP coutx[3] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlCfg);

        pwmHlCfg.base.channelCount     = (uint32)(sizeof(ccx) / sizeof(ccx[0]));
        pwmHlCfg.base.deadtime         = PWM_DEAD_TIME_S;                        /* 0.5 us */
        pwmHlCfg.base.minPulse         = PWM_MIN_PULSE_TIME_S;                   /* 1.0 us */
        pwmHlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;
        pwmHlCfg.base.outputDriver     = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlCfg.base.ccxActiveState   = Ifx_ActiveState_high;
        pwmHlCfg.base.coutxActiveState = Ifx_ActiveState_high;

        pwmHlCfg.ccx   = ccx;
        pwmHlCfg.coutx = coutx;
        pwmHlCfg.timer = &s_drv.timer;
        pwmHlCfg.tom   = IfxGtm_Tom_1;

        ok = IfxGtm_Tom_PwmHl_init(&s_drv.pwm, &pwmHlCfg);
        if (ok == FALSE)
        {
            return; /* Early exit on failure */
        }

        ok = IfxGtm_Tom_PwmHl_setMode(&s_drv.pwm, Ifx_Pwm_Mode_centerAligned);
        if (ok == FALSE)
        {
            return; /* Early exit on failure */
        }

        /* Update derived input frequency information */
        IfxGtm_Tom_Timer_updateInputFrequency(&s_drv.timer);
    }

    /* 6) Program initial duties 25% / 50% / 75% with synchronous update */
    {
        Ifx_TimerValue period = s_drv.pwm.timer->base.period;
        s_drv.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * DUTY_25_PERCENT);
        s_drv.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * DUTY_50_PERCENT);
        s_drv.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * DUTY_75_PERCENT);

        /* Track persistent duty fractions for runtime policy */
        s_drv.dutyFrac[0] = DUTY_25_PERCENT;
        s_drv.dutyFrac[1] = DUTY_50_PERCENT;
        s_drv.dutyFrac[2] = DUTY_75_PERCENT;

        IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer); /* Synchronous apply at next period */
    }

    /* 7) Start PWM (timer run enables outputs, sync handled by shadow update) */
    IfxGtm_Tom_Timer_run(&s_drv.timer);

    s_initialized = TRUE;
}

/*
 * Runtime update: every call increments each phase duty by +10 percentage points
 * with wrap-around and clamps to safe range considering minPulse and 2*deadtime.
 * Applies update synchronously at next timer event.
 */
void IfxGtm_Tom_PwmHl_setDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit if init failed or not called yet */
    }

    /* 1-2) Increment and wrap duties in [0.0 .. 1.0) */
    for (uint32 i = 0; i < 3U; i++)
    {
        s_drv.dutyFrac[i] += DUTY_INCREMENT_FRACTION;   /* +10% */
        if (s_drv.dutyFrac[i] >= 1.0f)
        {
            s_drv.dutyFrac[i] = 0.0f;                   /* wrap to 0% */
        }
    }

    /* 3) Compute PWM period in seconds from configured 20 kHz */
    const float32 period_s = 1.0f / PWM_FREQ_HZ; /* 50 us at 20 kHz */

    /* 4) Compute safe min/max duty based on minPulse and 2*deadtime */
    const float32 safeMinDuty = (PWM_MIN_PULSE_TIME_S + (2.0f * PWM_DEAD_TIME_S)) / period_s;
    const float32 safeMaxDuty = 1.0f - safeMinDuty;

    /* 5) Clamp each requested duty into [safeMin, safeMax] */
    for (uint32 i = 0; i < 3U; i++)
    {
        s_drv.dutyFrac[i] = clampf32(s_drv.dutyFrac[i], safeMinDuty, safeMaxDuty);
    }

    /* 6) Convert to on-times and apply synchronously (non-immediate) */
    {
        const Ifx_TimerValue period = s_drv.pwm.timer->base.period;
        s_drv.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * s_drv.dutyFrac[0]);
        s_drv.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * s_drv.dutyFrac[1]);
        s_drv.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * s_drv.dutyFrac[2]);

        IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer); /* Sync update at next timer boundary */
    }
}
