/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production-ready GTM TOM 3-Phase Inverter PWM driver using iLLD (IfxGtm_Tom_Timer + IfxGtm_Tom_PwmHl)
 *
 * Notes:
 * - No watchdog handling here (must be in CpuX_Main.c only)
 * - Uses only the allowed iLLD calls from the provided API list
 * - Center-aligned complementary PWM with synchronous shadow updates
 */

/* Own header */
#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD dependencies */
#include "Ifx_Types.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* -----------------------------------------------------------------------------
 * Configuration macros (from requirements)
 * ---------------------------------------------------------------------------*/
#define NUM_OF_CHANNELS                 (3U)
#define PWM_FREQUENCY_HZ                (20000U)          /* 20 kHz */
#define PWM_DEADTIME_US                 (0.5f)            /* 0.5 microseconds */
#define PWM_MIN_PULSE_US                (1.0f)            /* 1.0 microseconds */
#define DUTY_U_INIT_PERCENT             (25.0f)
#define DUTY_V_INIT_PERCENT             (50.0f)
#define DUTY_W_INIT_PERCENT             (75.0f)
#define DUTY_STEP_FRACTION              (0.05f)           /* 5% of period per update step */

/* Time base assumptions (tick conversion) */
#define GTM_TICK_NS                     (10U)             /* assumption: 10 ns per tick (FXCLK0=100 MHz) */

/* Pin routing macros (validated TOM1 / P00.x mappings) */
#define PHASE_U_HS                      &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS                      &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS                      &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS                      &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS                      &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS                      &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* LED for ISR debug toggle (port, pin) */
#define LED                              &MODULE_P13, 0

/* ISR priority macro */
#ifndef ISR_PRIORITY_ATOM
#define ISR_PRIORITY_ATOM                (10)
#endif

/* Ensure IFX_STATIC is available */
#ifndef IFX_STATIC
#define IFX_STATIC static
#endif

/* -----------------------------------------------------------------------------
 * Internal state
 * ---------------------------------------------------------------------------*/
typedef struct
{
    IfxGtm_Tom_Timer     timer;                 /* Persistent TOM timer handle */
    IfxGtm_Tom_PwmHl     pwm;                   /* Persistent PwmHl driver handle */
    Ifx_TimerValue       onTime[NUM_OF_CHANNELS];
    Ifx_TimerValue       periodTicks;           /* Cached period in ticks */
    boolean              initialized;
} GtmTom3Ph_State;

IFX_STATIC GtmTom3Ph_State g_gtmTom3Ph = {0};

/* -----------------------------------------------------------------------------
 * ISR and Period Callback (minimal for debug/trace)
 * ---------------------------------------------------------------------------*/
IFX_INTERRUPT(interruptGtm_TomAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtm_TomAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period callback (assigned internally by timer config if used) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* -----------------------------------------------------------------------------
 * Local helpers
 * ---------------------------------------------------------------------------*/
static inline Ifx_TimerValue usToTicks(float32 us)
{
    /* ticks = (us * 1000 ns/us) / (tick_ns) */
    float32 ticks = (us * 1000.0f) / (float32)GTM_TICK_NS;
    if (ticks < 0.0f)
    {
        ticks = 0.0f;
    }
    return (Ifx_TimerValue)(ticks + 0.5f); /* round to nearest */
}

/* -----------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/
/**
 * Initialize GTM TOM PWM HL for 3-phase inverter
 */
void initGtmTomPwm(void)
{
    /* Safety: prevent re-init without deinit (not provided here) */
    g_gtmTom3Ph.initialized = FALSE;

    /* 1) Enable GTM and FXCLK domain */
    IfxGtm_enable(&MODULE_GTM);
    /* Enable FXCLK (and optionally CLK0 if required by timer). Only FXCLK is strictly needed for TOM) */
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)IFXGTM_TOM_CMU_CLKEN_FXCLK);

    /* 2) Configure TOM timer (TOM1, CH0 base) */
    IfxGtm_Tom_Timer_Config timerCfg;
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

    /* Assign TOM instance and base channel */
    timerCfg.tom          = IfxGtm_Tom_1;
    timerCfg.timerChannel = IfxGtm_Tom_Ch_0;              /* base channel */

    /* Set frequency and FXCLK0 clock source */
    timerCfg.base.frequency = (float32)PWM_FREQUENCY_HZ;  /* 20 kHz */
    timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;

    /* Optional: ISR setup (priority/provider) if used by application */
    timerCfg.base.isrPriority = ISR_PRIORITY_ATOM;
    timerCfg.base.isrProvider = IfxSrc_Tos_cpu0;

    if (IfxGtm_Tom_Timer_init(&g_gtmTom3Ph.timer, &timerCfg) == FALSE)
    {
        /* Timer init failed */
        return;
    }

    /* 3) Prepare complementary PWM HL configuration for 3 phases */
    static const IfxGtm_Tom_ToutMapP ccx[NUM_OF_CHANNELS]  = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
    static const IfxGtm_Tom_ToutMapP coutx[NUM_OF_CHANNELS] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

    IfxGtm_Tom_PwmHl_Config pwmhlCfg;
    IfxGtm_Tom_PwmHl_initConfig(&pwmhlCfg);

    pwmhlCfg.timer                 = &g_gtmTom3Ph.timer;
    pwmhlCfg.tom                   = IfxGtm_Tom_1;
    pwmhlCfg.ccx                   = ccx;
    pwmhlCfg.coutx                 = coutx;
    pwmhlCfg.initPins              = TRUE; /* let driver init pins */

    /* Base HL configuration */
    pwmhlCfg.base.channelCount     = NUM_OF_CHANNELS;      /* 3 pairs */
    pwmhlCfg.base.deadtime         = PWM_DEADTIME_US * 1e-6f;   /* seconds */
    pwmhlCfg.base.minPulse         = PWM_MIN_PULSE_US * 1e-6f;  /* seconds */
    pwmhlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;
    pwmhlCfg.base.padDriver        = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    pwmhlCfg.base.ccxActiveState   = Ifx_ActiveState_high; /* high-side active high */
    pwmhlCfg.base.coutxActiveState = Ifx_ActiveState_high; /* low-side  active high */

    if (IfxGtm_Tom_PwmHl_init(&g_gtmTom3Ph.pwm, &pwmhlCfg) == FALSE)
    {
        /* PWM HL init failed */
        return;
    }

    /* Set center-aligned PWM mode */
    if (IfxGtm_Tom_PwmHl_setMode(&g_gtmTom3Ph.pwm, Ifx_Pwm_Mode_centerAligned) == FALSE)
    {
        /* Mode set failed */
        return;
    }

    /* 4) Start the timer */
    IfxGtm_Tom_Timer_run(&g_gtmTom3Ph.timer);

    /* 5) Compute initial on-times from current period and initial duty ratios */
    /* Read period ticks from persistent timer handle */
    g_gtmTom3Ph.periodTicks = g_gtmTom3Ph.timer.base.period; /* access persistent handle */
    Ifx_TimerValue period   = g_gtmTom3Ph.periodTicks;

    g_gtmTom3Ph.onTime[0] = (Ifx_TimerValue)((((float32)period) * (DUTY_U_INIT_PERCENT / 100.0f)) + 0.5f);
    g_gtmTom3Ph.onTime[1] = (Ifx_TimerValue)((((float32)period) * (DUTY_V_INIT_PERCENT / 100.0f)) + 0.5f);
    g_gtmTom3Ph.onTime[2] = (Ifx_TimerValue)((((float32)period) * (DUTY_W_INIT_PERCENT / 100.0f)) + 0.5f);

    /* Apply initial on-times using shadow-update sequence */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3Ph.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3Ph.pwm, g_gtmTom3Ph.onTime);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3Ph.timer);

    g_gtmTom3Ph.initialized = TRUE;
}

/**
 * Update duty cycles with a fixed step and apply synchronously.
 */
void updateGtmTomPwmDutyCycles(void)
{
    if (g_gtmTom3Ph.initialized == FALSE)
    {
        return;
    }

    /* Read the current period from the persistent timer handle */
    Ifx_TimerValue period = g_gtmTom3Ph.timer.base.period;
    g_gtmTom3Ph.periodTicks = period;

    /* Compute increment step as fixed fraction of period */
    Ifx_TimerValue step = (Ifx_TimerValue)(((float32)period * DUTY_STEP_FRACTION) + 0.5f);
    if (step == 0U)
    {
        step = 1U; /* ensure progress */
    }

    /* Minimum and maximum on-time thresholds (in ticks) */
    const Ifx_TimerValue minTicks = usToTicks(PWM_MIN_PULSE_US);
    Ifx_TimerValue       maxTicks = (period > minTicks) ? (period - minTicks) : 0U;
    if (maxTicks < minTicks)
    {
        maxTicks = minTicks; /* avoid underflow corner case */
    }

    /* Update each phase: add step and wrap to minimum when exceeding maximum */
    /* Phase U */
    {
        Ifx_TimerValue t = g_gtmTom3Ph.onTime[0] + step;
        g_gtmTom3Ph.onTime[0] = (t > maxTicks) ? minTicks : t;
    }
    /* Phase V */
    {
        Ifx_TimerValue t = g_gtmTom3Ph.onTime[1] + step;
        g_gtmTom3Ph.onTime[1] = (t > maxTicks) ? minTicks : t;
    }
    /* Phase W */
    {
        Ifx_TimerValue t = g_gtmTom3Ph.onTime[2] + step;
        g_gtmTom3Ph.onTime[2] = (t > maxTicks) ? minTicks : t;
    }

    /* Apply synchronously using shadow-update sequence */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3Ph.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3Ph.pwm, g_gtmTom3Ph.onTime);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3Ph.timer);
}
