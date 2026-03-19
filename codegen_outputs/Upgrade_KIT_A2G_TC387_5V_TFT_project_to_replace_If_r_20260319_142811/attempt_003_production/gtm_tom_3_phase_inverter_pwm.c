/**
 * GTM TOM 3-Phase Inverter PWM - Production Implementation (TC3xx / TC387)
 *
 * Requirements implemented:
 * - 20 kHz center-aligned PWM (TIMING_PWM_FREQUENCY_HZ = 20000)
 * - Complementary outputs with hardware dead-time (TIMING_DEADTIME_US = 0.5 us)
 * - Minimum pulse (TIMING_MIN_PULSE_US = 1.0 us)
 * - Initial duties U/V/W = 25%/50%/75%
 * - Update policy: every call add +10% with wrap, clamp to safe range using minPulse and 2*deadtime
 * - TOM1 CH0 as master timer, fxclk0 as time base, synchronous TGC update
 * - Pins: U: TOM1 CH2/P00.3 (HS) & CH1/P00.2 (LS); V: CH4/P00.5 & CH3/P00.4; W: CH6/P00.7 & CH5/P00.6
 *
 * Notes:
 * - Watchdog disable is not placed here (must be in CpuN_Main.c per AURIX standard).
 * - Uses generic PinMap headers (no family-specific suffixes).
 * - Error handling: checks boolean returns from iLLD init calls and only marks initialized on success.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"     /* Generic PinMap header (no family-specific suffix) */
#include "IfxPort.h"

/* ============================= Requirements Macros ============================= */
#define TIMING_PWM_FREQUENCY_HZ                 (20000.0f)    /* 20 kHz */
#define TIMING_DEADTIME_US                      (0.5f)        /* microseconds */
#define TIMING_MIN_PULSE_US                     (1.0f)        /* microseconds */
#define DUTY_UPDATE_POLICY_INCREMENT_PERCENT    (10.0f)       /* +10% each call */
#define CLOCK_GTM_FXCLK0_FREQ_MHZ               (100.0f)      /* FXCLK0 = 100 MHz */

#define INITIAL_DUTY_PERCENT_U                  (25.0f)
#define INITIAL_DUTY_PERCENT_V                  (50.0f)
#define INITIAL_DUTY_PERCENT_W                  (75.0f)

/* Convert microseconds to seconds */
#define US_TO_S(x_us)                           ((float32)((x_us) * 1.0e-6f))

/* ============================= Pin Assignments (TC387) ============================= */
/* Phase U: HS -> TOM1 CH2 (P00.3/TOUT12), LS -> TOM1 CH1 (P00.2/TOUT11) */
#define PHASE_U_HS   &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS   &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
/* Phase V: HS -> TOM1 CH4 (P00.5/TOUT14), LS -> TOM1 CH3 (P00.4/TOUT13) */
#define PHASE_V_HS   &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS   &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
/* Phase W: HS -> TOM1 CH6 (P00.7/TOUT16), LS -> TOM1 CH5 (P00.6/TOUT15) */
#define PHASE_W_HS   &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS   &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* ============================= Internal Driver State ============================= */
#define NUM_OF_CHANNELS  (3u)

typedef struct
{
    IfxGtm_Tom_Timer   timer;                     /* TOM master timer (TOM1 CH0) */
    IfxGtm_Tom_PwmHl   pwm;                       /* Complementary PWM driver */
    Ifx_TimerValue     pwmOnTimes[NUM_OF_CHANNELS];
    float32            dutyFrac[NUM_OF_CHANNELS]; /* normalized 0.0..1.0 */
    float32            deadtime_s;                /* seconds */
    float32            minPulse_s;                /* seconds */
    float32            frequency_Hz;              /* PWM frequency */
    boolean            initialized;               /* module init flag */
} GtmTom3PhPwm_t;

static GtmTom3PhPwm_t s_pwm = {
    .initialized = FALSE
};

/* ============================= Local Utilities ============================= */
static inline float32 clamp_f32(float32 v, float32 lo, float32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* ============================= Public API ============================= */

void IfxGtm_Tom_PwmHl_init(void)
{
    /* Ensure we start from a known state */
    s_pwm.initialized = FALSE;

    /* 1) Enable GTM module */
    IfxGtm_enable(&MODULE_GTM);

    /* 2) CMU clock setup: GCLK = module frequency, CLK0 = 100 MHz, enable FXCLK */
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, (CLOCK_GTM_FXCLK0_FREQ_MHZ * 1.0e6f));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 3) Configure TOM1 CH0 as master timer at 20 kHz using FXCLK0 */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

        timerCfg.base.frequency = TIMING_PWM_FREQUENCY_HZ;              /* 20 kHz */
        timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;       /* FXCLK0 */
        timerCfg.tom            = IfxGtm_Tom_1;                         /* TOM1 */
        timerCfg.timerChannel   = IfxGtm_Tom_Ch_0;                      /* CH0 master */

        if (IfxGtm_Tom_Timer_init(&s_pwm.timer, &timerCfg) == FALSE)
        {
            return; /* Error: do not proceed */
        }
    }

    /* 4-5) Configure complementary PWM HL with dead-time and min pulse */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlCfg;
        IfxGtm_Tom_ToutMapP ccx[NUM_OF_CHANNELS] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
        IfxGtm_Tom_ToutMapP coutx[NUM_OF_CHANNELS] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlCfg);

        pwmHlCfg.base.channelCount     = (uint8)NUM_OF_CHANNELS;
        pwmHlCfg.base.deadtime         = US_TO_S(TIMING_DEADTIME_US);        /* seconds */
        pwmHlCfg.base.minPulse         = US_TO_S(TIMING_MIN_PULSE_US);       /* seconds */
        pwmHlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;
        pwmHlCfg.base.outputDriver     = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlCfg.base.ccxActiveState   = Ifx_ActiveState_high;
        pwmHlCfg.base.coutxActiveState = Ifx_ActiveState_high;

        pwmHlCfg.ccx   = ccx;
        pwmHlCfg.coutx = coutx;
        pwmHlCfg.timer = &s_pwm.timer;
        pwmHlCfg.tom   = IfxGtm_Tom_1;

        if (IfxGtm_Tom_PwmHl_init(&s_pwm.pwm, &pwmHlCfg) == FALSE)
        {
            return; /* Error: do not proceed */
        }

        /* Center-aligned mode per requirement */
        (void)IfxGtm_Tom_PwmHl_setMode(&s_pwm.pwm, Ifx_Pwm_Mode_centerAligned);

        /* Cache timing parameters for runtime policy */
        s_pwm.deadtime_s   = pwmHlCfg.base.deadtime;
        s_pwm.minPulse_s   = pwmHlCfg.base.minPulse;
        s_pwm.frequency_Hz = TIMING_PWM_FREQUENCY_HZ;

        /* Ensure timer base frequencies are updated internally */
        IfxGtm_Tom_Timer_updateInputFrequency(&s_pwm.timer);
    }

    /* 7) Start timer (PWM outputs controlled by PwmHl and TGC updates) */
    IfxGtm_Tom_Timer_run(&s_pwm.timer);

    /* 6) Program initial duties: 25% / 50% / 75% with synchronous (non-immediate) update */
    {
        const float32 du = INITIAL_DUTY_PERCENT_U * (1.0f / 100.0f);
        const float32 dv = INITIAL_DUTY_PERCENT_V * (1.0f / 100.0f);
        const float32 dw = INITIAL_DUTY_PERCENT_W * (1.0f / 100.0f);

        s_pwm.dutyFrac[0] = du;
        s_pwm.dutyFrac[1] = dv;
        s_pwm.dutyFrac[2] = dw;

        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_pwm.timer);
        s_pwm.pwmOnTimes[0]   = (Ifx_TimerValue)((float32)period * s_pwm.dutyFrac[0]);
        s_pwm.pwmOnTimes[1]   = (Ifx_TimerValue)((float32)period * s_pwm.dutyFrac[1]);
        s_pwm.pwmOnTimes[2]   = (Ifx_TimerValue)((float32)period * s_pwm.dutyFrac[2]);

        IfxGtm_Tom_Timer_disableUpdate(&s_pwm.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&s_pwm.pwm, s_pwm.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&s_pwm.timer);
    }

    s_pwm.initialized = TRUE;
}

void IfxGtm_Tom_PwmHl_setDuty(void)
{
    if (s_pwm.initialized == FALSE)
    {
        return; /* Early exit on uninitialized state */
    }

    /* 1) Increment duties by +10 percentage points (0.10 in normalized form) */
    const float32 step = (DUTY_UPDATE_POLICY_INCREMENT_PERCENT * (1.0f / 100.0f));
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        s_pwm.dutyFrac[i] += step;
        /* 2) Wrap at 1.0 -> 0.0 */
        if (s_pwm.dutyFrac[i] >= 1.0f)
        {
            s_pwm.dutyFrac[i] = 0.0f;
        }
    }

    /* 3) Compute period from configured frequency (20 kHz) */
    const float32 freq_Hz    = s_pwm.frequency_Hz; /* as configured */
    const float32 period_s   = (freq_Hz > 0.0f) ? (1.0f / freq_Hz) : 0.0f;

    /* 4) Compute safe min/max duty using minPulse and 2*deadtime */
    const float32 safeMin_s  = s_pwm.minPulse_s + (2.0f * s_pwm.deadtime_s);
    const float32 safeMin    = (period_s > 0.0f) ? clamp_f32(safeMin_s / period_s, 0.0f, 0.5f) : 0.0f;
    const float32 safeMax    = 1.0f - safeMin;

    /* 5) Clamp requests into [safeMin, safeMax] */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        s_pwm.dutyFrac[i] = clamp_f32(s_pwm.dutyFrac[i], safeMin, safeMax);
    }

    /* Convert duties to on-times in timer ticks */
    const Ifx_TimerValue periodTicks = IfxGtm_Tom_Timer_getPeriod(&s_pwm.timer);
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        s_pwm.pwmOnTimes[i] = (Ifx_TimerValue)((float32)periodTicks * s_pwm.dutyFrac[i]);
    }

    /* 6) Apply using synchronous TGC update (non-immediate) */
    IfxGtm_Tom_Timer_disableUpdate(&s_pwm.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&s_pwm.pwm, s_pwm.pwmOnTimes);
    IfxGtm_Tom_Timer_applyUpdate(&s_pwm.timer);
}
