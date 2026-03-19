/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production 3-phase inverter PWM using GTM TOM + PwmHl (TC3xx)
 */
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"  /* Generic pin map header (no family-specific suffix) */

/* Do NOT include watchdog APIs here per architecture rules */

/* ===================== Pin and Resource Selection (User Requirement) ===================== */
/* TOM1 pins: U: TOM1 CH2/P00.3 (HS) & CH1/P00.2 (LS);
 *            V: TOM1 CH4/P00.5 (HS) & CH3/P00.4 (LS);
 *            W: TOM1 CH6/P00.7 (HS) & CH5/P00.6 (LS)
 */
#define PHASE_U_HS   &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS   &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS   &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS   &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS   &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS   &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

#define GTM_TOM_MASTER               IfxGtm_Tom_1
#define GTM_TOM_MASTER_TIMER_CH      IfxGtm_Tom_Ch_0
#define NUM_PHASES                   (3u)

/* ===================== Module State ===================== */
typedef struct
{
    IfxGtm_Tom_Timer    timer;                 /* Master TOM timer */
    IfxGtm_Tom_PwmHl    pwm;                   /* Complementary high/low driver */
    Ifx_TimerValue      pwmOnTimes[NUM_PHASES];/* On-times in ticks for U,V,W */
    float32             dutyNorm[NUM_PHASES];  /* Duty in normalized fraction [0..1] */
    boolean             initialized;           /* Init complete flag */
} GtmTom3PhasePwm_t;

static GtmTom3PhasePwm_t g_pwm3Phase = {0};

/* ===================== Local Helpers ===================== */
static float32 clampf(float32 v, float32 lo, float32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* ===================== Public API Implementations ===================== */
void IfxGtm_Tom_PwmHl_init(void)
{
    /* Ensure single-init semantics */
    g_pwm3Phase.initialized = FALSE;

    /* 1) Enable GTM module */
    IfxGtm_enable(&MODULE_GTM);

    /* 2) CMU clock setup: set GCLK to module frequency, CLK0 to GCLK, enable FXCLK */
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 3) Configure TOM1 master timer (CH0) using Fxclk0 as time base */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
        timerCfg.base.frequency = TIMING_PWM_FREQUENCY_HZ;              /* 20 kHz */
        timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;       /* Fxclk0 */
        timerCfg.tom            = GTM_TOM_MASTER;                       /* TOM1 */
        timerCfg.timerChannel   = GTM_TOM_MASTER_TIMER_CH;              /* CH0 */

        if (IfxGtm_Tom_Timer_init(&g_pwm3Phase.timer, &timerCfg) == FALSE)
        {
            return; /* Error: leave initialized = FALSE */
        }
    }

    /* 4) Initialize TOM PwmHl for complementary outputs with hardware DTM */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlCfg;
        IfxGtm_Tom_ToutMapP ccx[] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
        IfxGtm_Tom_ToutMapP coutx[] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlCfg);
        pwmHlCfg.base.channelCount   = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
        pwmHlCfg.base.deadtime       = TIMING_DEADTIME_US * 1.0e-6f;    /* seconds */
        pwmHlCfg.base.minPulse       = TIMING_MIN_PULSE_US * 1.0e-6f;   /* seconds */
        pwmHlCfg.base.outputMode     = IfxPort_OutputMode_pushPull;
        pwmHlCfg.base.outputDriver   = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlCfg.base.ccxActiveState = Ifx_ActiveState_high;
        pwmHlCfg.base.coutxActiveState = Ifx_ActiveState_high;

        pwmHlCfg.ccx   = ccx;
        pwmHlCfg.coutx = coutx;
        pwmHlCfg.timer = &g_pwm3Phase.timer;
        pwmHlCfg.tom   = GTM_TOM_MASTER;

        if (IfxGtm_Tom_PwmHl_init(&g_pwm3Phase.pwm, &pwmHlCfg) == FALSE)
        {
            return; /* Error: leave initialized = FALSE */
        }

        /* Center-aligned PWM mode */
        if (IfxGtm_Tom_PwmHl_setMode(&g_pwm3Phase.pwm, Ifx_Pwm_Mode_centerAligned) == FALSE)
        {
            return; /* Error: leave initialized = FALSE */
        }

        /* Update input frequency info (keeps timer internals consistent) */
        IfxGtm_Tom_Timer_updateInputFrequency(&g_pwm3Phase.timer);
    }

    /* 5) Start the timer (outputs enabled by PwmHl initialization) */
    IfxGtm_Tom_Timer_run(&g_pwm3Phase.timer);

    /* 6) Program initial duties 25%/50%/75% via synchronous (non-immediate) update */
    {
        const float32 dutyU = INITIAL_DUTY_PERCENT_U * 0.01f; /* normalized [0..1] */
        const float32 dutyV = INITIAL_DUTY_PERCENT_V * 0.01f;
        const float32 dutyW = INITIAL_DUTY_PERCENT_W * 0.01f;
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_pwm3Phase.timer);

        g_pwm3Phase.dutyNorm[0] = dutyU;
        g_pwm3Phase.dutyNorm[1] = dutyV;
        g_pwm3Phase.dutyNorm[2] = dutyW;

        g_pwm3Phase.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * g_pwm3Phase.dutyNorm[0]);
        g_pwm3Phase.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * g_pwm3Phase.dutyNorm[1]);
        g_pwm3Phase.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * g_pwm3Phase.dutyNorm[2]);

        IfxGtm_Tom_Timer_disableUpdate(&g_pwm3Phase.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3Phase.pwm, g_pwm3Phase.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&g_pwm3Phase.timer);
    }

    g_pwm3Phase.initialized = TRUE;
}

void IfxGtm_Tom_PwmHl_setDuty(void)
{
    if (g_pwm3Phase.initialized == FALSE)
    {
        return; /* Early-exit on uninitialized, per production error-handling policy */
    }

    /* 1) Increment each duty by +10 percentage points, wrap to 0 if >= 1.0 */
    {
        const float32 step = DUTY_UPDATE_POLICY_INCREMENT_PERCENT * 0.01f; /* normalized step */
        for (uint8 i = 0; i < NUM_PHASES; i++)
        {
            g_pwm3Phase.dutyNorm[i] += step;
            if (g_pwm3Phase.dutyNorm[i] >= 1.0f)
            {
                g_pwm3Phase.dutyNorm[i] = 0.0f; /* wrap */
            }
        }
    }

    /* 2..4) Compute safe min/max fraction from configured frequency, minPulse and deadtime */
    float32 period_s   = 1.0f / TIMING_PWM_FREQUENCY_HZ; /* 50 us for 20 kHz */
    float32 minPulse_s = TIMING_MIN_PULSE_US * 1.0e-6f;
    float32 deadtime_s = TIMING_DEADTIME_US  * 1.0e-6f;
    float32 safeMin    = (minPulse_s + 2.0f * deadtime_s) / period_s; /* fraction in [0..1] */
    float32 safeMax    = 1.0f - safeMin;

    /* 5) Clamp each duty to [safeMin, safeMax] */
    for (uint8 i = 0; i < NUM_PHASES; i++)
    {
        g_pwm3Phase.dutyNorm[i] = clampf(g_pwm3Phase.dutyNorm[i], safeMin, safeMax);
    }

    /* 6) Convert to on-times and request synchronous TGC update */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_pwm3Phase.timer);
        g_pwm3Phase.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * g_pwm3Phase.dutyNorm[0]);
        g_pwm3Phase.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * g_pwm3Phase.dutyNorm[1]);
        g_pwm3Phase.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * g_pwm3Phase.dutyNorm[2]);

        IfxGtm_Tom_Timer_disableUpdate(&g_pwm3Phase.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3Phase.pwm, g_pwm3Phase.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&g_pwm3Phase.timer);
    }
}
