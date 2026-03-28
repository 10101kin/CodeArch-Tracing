/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM three-phase inverter PWM on TC3xx.
 *
 * This module configures GTM TOM1 with a dedicated base channel and three complementary
 * high/low output pairs (U, V, W) at 20 kHz, center-aligned, active-high ccx and active-low coutx,
 * with dead-time and minimum pulse constraints. It maintains a persistent state for runtime updates.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* =======================================================================
 * Configuration Macros (from requirements)
 * ======================================================================= */
#define NUM_OF_CHANNELS                 (3)
#define PWM_FREQUENCY_HZ                (20000.0f)          /* 20 kHz */
#define DEADTIME_US                     (0.5f)              /* 0.5 microseconds */
#define MIN_PULSE_US                    (1.0f)              /* 1.0 microseconds */
#define INITIAL_DUTY_U_PERCENT          (25.0f)
#define INITIAL_DUTY_V_PERCENT          (50.0f)
#define INITIAL_DUTY_W_PERCENT          (75.0f)
#define PWM_DUTY_STEP_FRAC              (0.10f)             /* 10% of period per update step */

/* Timing assumption from requirements (fxclk0 = 100 MHz => tick = 10 ns) */
#define FXCLK0_TICK_NS                  (10.0f)
#define DEADTIME_TICKS                  ((Ifx_TimerValue)50u)    /* 0.5 us / 10 ns */
#define MIN_PULSE_TICKS                 ((Ifx_TimerValue)100u)   /* 1.0 us / 10 ns */

/* LED for basic debug (port, pin) */
#define LED &MODULE_P13, 0

/* TOM instance and base channel */
#define TOM_INSTANCE                    IfxGtm_Tom_1
#define TOM_BASE_CHANNEL                IfxGtm_Tom_Ch_0

/* Pin routing macros (user-specified mapping; validated against template) */
#define PHASE_U_HS                      &IfxGtm_TOM1_2_TOUT12_P00_3_OUT  /* U high-side: TOM1 CH2 -> P00.3 */
#define PHASE_U_LS                      &IfxGtm_TOM1_1_TOUT11_P00_2_OUT  /* U low-side:  TOM1 CH1 -> P00.2 */
#define PHASE_V_HS                      &IfxGtm_TOM1_4_TOUT14_P00_5_OUT  /* V high-side: TOM1 CH4 -> P00.5 */
#define PHASE_V_LS                      &IfxGtm_TOM1_3_TOUT13_P00_4_OUT  /* V low-side:  TOM1 CH3 -> P00.4 */
#define PHASE_W_HS                      &IfxGtm_TOM1_6_TOUT16_P00_7_OUT  /* W high-side: TOM1 CH6 -> P00.7 */
#define PHASE_W_LS                      &IfxGtm_TOM1_5_TOUT15_P00_6_OUT  /* W low-side:  TOM1 CH5 -> P00.6 */

/* =======================================================================
 * Persistent Module State
 * ======================================================================= */

typedef struct
{
    IfxGtm_Tom_Timer   timer;                 /* TOM timer handle */
    IfxGtm_Tom_PwmHl   pwmhl;                 /* PwmHl handle */
    Ifx_TimerValue     periodTicks;           /* Cached period ticks */
    Ifx_TimerValue     tOn[NUM_OF_CHANNELS];  /* On-time ticks per phase (U,V,W) */
} GtmTom3ph_State;

/* IFX_STATIC as required (Compilers.h provides definition via Ifx_Types.h) */
IFX_STATIC GtmTom3ph_State g_gtmTom3phState;

/* =======================================================================
 * Local pin map arrays for PwmHl (ccx = high-side, coutx = low-side)
 * ======================================================================= */
static IFX_CONST IfxGtm_Tom_ToutMapP s_tomCc[NUM_OF_CHANNELS]   = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
static IFX_CONST IfxGtm_Tom_ToutMapP s_tomCout[NUM_OF_CHANNELS] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

/* =======================================================================
 * Private helpers
 * ======================================================================= */
static inline Ifx_TimerValue gtm_computePeriodTicks(float32 fxclkHz, float32 freqHz)
{
    float32 periodF = fxclkHz / freqHz;   /* ticks = fxclk/f */
    if (periodF < 1.0f)
    {
        periodF = 1.0f; /* guard */
    }
    return (Ifx_TimerValue)(periodF + 0.5f);
}

/* =======================================================================
 * Public API implementations
 * ======================================================================= */

/**
 * Initialize GTM TOM PWM for three-phase inverter per design requirements.
 * Sequence:
 *  - Enable GTM, configure GCLK and FXCLK domain
 *  - Configure TOM1 timer (base channel 0), target frequency 20 kHz (FXCLK0)
 *  - Prepare complementary high/low PwmHl for 3 phases with dead-time and min pulse
 *  - Center-aligned mode, start timer
 *  - Compute initial on-time ticks (25%, 50%, 75%) and apply via shadow update
 */
void initGtmTomPwm(void)
{
    /* Enable GTM and clocks (GCLK at module frequency, enable FXCLK and CLK0 domains) */
    IfxGtm_enable(&MODULE_GTM);
    {
        float32 modFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, modFreq);
        /* Configure CMU CLK0 to module frequency (kept enabled alongside FXCLK) */
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, modFreq);
        /* Enable FXCLK and CLK0 domains */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* Determine FXCLK0 for period computation */
    float32 fxclk0Hz = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0);

    /* ---------------------- Timer configuration ---------------------- */
    IfxGtm_Tom_Timer_Config timerCfg;
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

    /* Assign TOM instance and base channel (using TOM1 CH0 as time base) */
    timerCfg.tom          = TOM_INSTANCE;
    timerCfg.timerChannel = TOM_BASE_CHANNEL;
    /* Select FXCLK0 source for TOM time base if available in config */
    /* Note: The exact field name for TOM channel clock source may vary by iLLD version.
       When available, set it to FXCLK0 (cmuFxclk0). */
#ifdef IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0
    timerCfg.clock        = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
#endif

    /* Initialize timer driver */
    if (IfxGtm_Tom_Timer_init(&g_gtmTom3phState.timer, &timerCfg) == FALSE)
    {
        return; /* error: timer init failed */
    }

    /* Compute and set period ticks for 20 kHz */
    g_gtmTom3phState.periodTicks = gtm_computePeriodTicks(fxclk0Hz, PWM_FREQUENCY_HZ);
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phState.timer);
    IfxStdIf_Timer_setPeriod(&g_gtmTom3phState.timer, g_gtmTom3phState.periodTicks);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phState.timer);

    /* ---------------------- PwmHl configuration ---------------------- */
    IfxGtm_Tom_PwmHl_Config pwmhlCfg;
    IfxGtm_Tom_PwmHl_initConfig(&pwmhlCfg);

    pwmhlCfg.timer   = &g_gtmTom3phState.timer;
    pwmhlCfg.tom     = TOM_INSTANCE;
    pwmhlCfg.ccx     = s_tomCc;      /* high-side array */
    pwmhlCfg.coutx   = s_tomCout;    /* low-side array */
    pwmhlCfg.initPins = TRUE;        /* let driver init pins */

    /* Standard interface base parameters */
    pwmhlCfg.base.channelCount     = NUM_OF_CHANNELS;                 /* 3 complementary pairs */
    pwmhlCfg.base.deadtime         = DEADTIME_US * 1.0e-6f;           /* seconds */
    pwmhlCfg.base.minPulse         = MIN_PULSE_US * 1.0e-6f;          /* seconds */
    pwmhlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;     /* push-pull */
    pwmhlCfg.base.padDriver        = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Polarity: high-side active HIGH, low-side active LOW (complementary) */
#ifdef IfxStdIf_PwmHl_Polarity
    pwmhlCfg.base.polarity.ccx     = Ifx_ActiveState_high;
    pwmhlCfg.base.polarity.coutx   = Ifx_ActiveState_low;
#else
    pwmhlCfg.base.ccxActiveState   = Ifx_ActiveState_high;
    pwmhlCfg.base.coutxActiveState = Ifx_ActiveState_low;
#endif

    if (IfxGtm_Tom_PwmHl_init(&g_gtmTom3phState.pwmhl, &pwmhlCfg) == FALSE)
    {
        return; /* error: PwmHl init failed */
    }

    /* Center-aligned mode */
    (void)IfxGtm_Tom_PwmHl_setMode(&g_gtmTom3phState.pwmhl, Ifx_Pwm_Mode_centerAligned);

    /* Start the TOM timer */
    IfxGtm_Tom_Timer_run(&g_gtmTom3phState.timer);

    /* ---------------------- Initial on-times and shadow update ---------------------- */
    g_gtmTom3phState.tOn[0] = (Ifx_TimerValue)((g_gtmTom3phState.periodTicks * (INITIAL_DUTY_U_PERCENT / 100.0f)) + 0.5f);
    g_gtmTom3phState.tOn[1] = (Ifx_TimerValue)((g_gtmTom3phState.periodTicks * (INITIAL_DUTY_V_PERCENT / 100.0f)) + 0.5f);
    g_gtmTom3phState.tOn[2] = (Ifx_TimerValue)((g_gtmTom3phState.periodTicks * (INITIAL_DUTY_W_PERCENT / 100.0f)) + 0.5f);

    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phState.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3phState.pwmhl, g_gtmTom3phState.tOn);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phState.timer);

    /* Configure LED for optional ISR/debug use (no ISR in this driver) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three-phase on-times by a fixed fraction of the period and wrap within thresholds.
 * Algorithm:
 *  - Read current period from persistent state
 *  - step = period * PWM_DUTY_STEP_FRAC
 *  - minTicks = MIN_PULSE_TICKS; maxTicks = period - MIN_PULSE_TICKS
 *  - For each phase: tOn += step; if (tOn > maxTicks) tOn = minTicks
 *  - Apply all three on-times synchronously via shadow update
 */
void updateGtmTomPwmDutyCycles(void)
{
    Ifx_TimerValue period    = g_gtmTom3phState.periodTicks;
    Ifx_TimerValue step      = (Ifx_TimerValue)((float32)period * PWM_DUTY_STEP_FRAC);
    Ifx_TimerValue minTicks  = (MIN_PULSE_TICKS < (period / 2U)) ? MIN_PULSE_TICKS : (period / 4U);
    Ifx_TimerValue maxTicks  = (period > MIN_PULSE_TICKS) ? (period - MIN_PULSE_TICKS) : (period - 1U);

    if (step == 0U)
    {
        step = 1U; /* ensure progress */
    }

    /* Phase U */
    {
        Ifx_TimerValue next = g_gtmTom3phState.tOn[0] + step;
        g_gtmTom3phState.tOn[0] = (next > maxTicks) ? minTicks : next;
    }

    /* Phase V */
    {
        Ifx_TimerValue next = g_gtmTom3phState.tOn[1] + step;
        g_gtmTom3phState.tOn[1] = (next > maxTicks) ? minTicks : next;
    }

    /* Phase W */
    {
        Ifx_TimerValue next = g_gtmTom3phState.tOn[2] + step;
        g_gtmTom3phState.tOn[2] = (next > maxTicks) ? minTicks : next;
    }

    /* Apply synchronously using shadow-update sequence */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phState.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3phState.pwmhl, g_gtmTom3phState.tOn);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phState.timer);
}
