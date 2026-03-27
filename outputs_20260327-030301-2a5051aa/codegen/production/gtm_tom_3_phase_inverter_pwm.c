/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM1 3-phase complementary PWM using TOM timer base
 * and IfxGtm_Tom_PwmHl (KIT_A2G_TC387_5V_TFT / TC3xx family).
 *
 * Implementation follows iLLD patterns and the specified behavior descriptions.
 *
 * Notes:
 * - Watchdog disable is NOT included here (must be in CpuX_Main.c only).
 * - No STM-based timing here; scheduling belongs to the CPU main loop.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD dependencies */
#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* =========================================================================
 * Configuration macros (from requirements)
 * ========================================================================= */
#define NUM_PHASES                        (3)
#define PWM_BASE_FREQUENCY_HZ             (20000.0f)     /* 20 kHz */
#define PWM_DEADTIME_US                   (0.5f)         /* 0.5 us */
#define PWM_MIN_PULSE_US                  (1.0f)         /* 1.0 us */

#define PHASE_U_INIT_DUTY_PCT             (25.0f)
#define PHASE_V_INIT_DUTY_PCT             (50.0f)
#define PHASE_W_INIT_DUTY_PCT             (75.0f)

/* Duty update step as a fixed fraction of period (1/64 of period per call) */
#define PWM_DUTY_STEP_DENOMINATOR         (64U)

/* LED for ISR debug toggle: compound macro (port, pin) */
#define LED                                &MODULE_P13, 0

/* ISR priority macro (used in IFX_INTERRUPT and any SRC config if added externally) */
#define ISR_PRIORITY_ATOM                 (20)

/* Pin routing (validated/reference symbols for TOM1 on P00.[2..7]) */
#define PHASE_U_HS                        &IfxGtm_TOM1_2_TOUT12_P00_3_OUT  /* TOM1 CH2 -> P00.3 */
#define PHASE_U_LS                        &IfxGtm_TOM1_1_TOUT11_P00_2_OUT  /* TOM1 CH1 -> P00.2 */
#define PHASE_V_HS                        &IfxGtm_TOM1_4_TOUT14_P00_5_OUT  /* TOM1 CH4 -> P00.5 */
#define PHASE_V_LS                        &IfxGtm_TOM1_3_TOUT13_P00_4_OUT  /* TOM1 CH3 -> P00.4 */
#define PHASE_W_HS                        &IfxGtm_TOM1_6_TOUT16_P00_7_OUT  /* TOM1 CH6 -> P00.7 */
#define PHASE_W_LS                        &IfxGtm_TOM1_5_TOUT15_P00_6_OUT  /* TOM1 CH5 -> P00.6 */

/* =========================================================================
 * Module state
 * ========================================================================= */

typedef struct
{
    IfxGtm_Tom_Timer     timer;                 /* TOM timer base */
    IfxGtm_Tom_PwmHl     pwmhl;                 /* PwmHl for complementary pairs */
    Ifx_TimerValue       onTime[NUM_PHASES];    /* current on-time ticks for U,V,W */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gti;

/* =========================================================================
 * ISR and period callback (required by structural rules)
 * ========================================================================= */

/* External ISR declaration; priority macro used by system SRC config (not in this file) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback placeholder (assigned where applicable; empty body) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =========================================================================
 * Local helpers
 * ========================================================================= */

/** Compute ticks from microseconds using current timer input frequency. */
static IFX_INLINE Ifx_TimerValue gtm_usToTicks(IfxGtm_Tom_Timer *t, float32 us)
{
    const float32 fin = IfxGtm_Tom_Timer_getInputFrequency(t); /* Hz */
    const float32 ticksPerUs = fin / 1.0e6f;
    float32 ticks = (us * ticksPerUs) + 0.5f;
    if (ticks < 1.0f)
    {
        ticks = 1.0f;
    }
    return (Ifx_TimerValue)((uint32)ticks);
}

/* =========================================================================
 * Public API
 * ========================================================================= */

/**
 * Initialize GTM for 3-phase complementary PWM on a TOM cluster using a TOM timer
 * as the base and a PwmHl helper for paired outputs.
 */
void initGtmTom3phInv(void)
{
    /* 1) Enable GTM and required CMU clocks (FXCLK and CLK0) using guard */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        /* Set CLK0 to module frequency; FXCLK derived inside GTM */
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        /* Enable FXCLK (for TOM) and CLK0 domains */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Initialize TOM timer configuration */
    IfxGtm_Tom_Timer_Config tCfg;
    IfxGtm_Tom_Timer_initConfig(&tCfg, &MODULE_GTM);
    /* Select TOM1 and a base counter channel (use CH0 as base) */
    tCfg.tom          = IfxGtm_Tom_1;
    tCfg.timerChannel = IfxGtm_Tom_Ch_0;
    /* Select FXCLK0 as clock source where applicable (implementation-dependent field names) */
    tCfg.clock        = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;

    /* Initialize timer driver */
    if (IfxGtm_Tom_Timer_init(&g_gti.timer, &tCfg) == FALSE)
    {
        /* Failed to init timer; abort init early */
        return;
    }

    /* Set target PWM frequency to 20 kHz */
    IfxGtm_Tom_Timer_setFrequency(&g_gti.timer, PWM_BASE_FREQUENCY_HZ);

    /* 3) Route six TOM outputs (three complementary pairs) */
    IfxGtm_PinMap_setTomTout(PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout(PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout(PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout(PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout(PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout(PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) Initialize TOM PwmHl with 3 pairs, set dead-time and min-pulse, bind to timer */
    IfxGtm_Tom_PwmHl_Config hlCfg;
    IfxGtm_Tom_PwmHl_initConfig(&hlCfg);

    /* Bind to base timer */
    hlCfg.timer                 = &g_gti.timer;
    /* Three complementary pairs */
    hlCfg.base.channelCount     = NUM_PHASES;
    /* Dead-time and minimum pulse (seconds) */
    hlCfg.base.deadtime         = (PWM_DEADTIME_US * 1.0e-6f);
    hlCfg.base.minPulse         = (PWM_MIN_PULSE_US * 1.0e-6f);
    /* Output mode and polarity (complementary: HS active HIGH, LS active LOW) */
    hlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;
    hlCfg.base.outputOnState    = Ifx_ActiveState_high;
    hlCfg.base.outputOffState   = Ifx_ActiveState_low;

    if (IfxGtm_Tom_PwmHl_init(&g_gti.pwmhl, &hlCfg) == FALSE)
    {
        /* Failed to init PwmHl; abort init early */
        return;
    }

    /* Center-aligned PWM mode */
    (void)IfxGtm_Tom_PwmHl_setMode(&g_gti.pwmhl, Ifx_Pwm_Mode_centerAligned);

    /* 5) Update timer input frequency from CMU and start timer */
    IfxGtm_Tom_Timer_updateInputFrequency(&g_gti.timer);
    IfxGtm_Tom_Timer_run(&g_gti.timer);

    /* Configure LED after PWM init (debug toggle in ISR) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 6) Compute initial on-times as 25%, 50%, 75% of period */
    const uint32 period = IfxGtm_Tom_Timer_getPeriod(&g_gti.timer);
    g_gti.onTime[0] = (Ifx_TimerValue)((period * PHASE_U_INIT_DUTY_PCT) * 0.01f + 0.5f);
    g_gti.onTime[1] = (Ifx_TimerValue)((period * PHASE_V_INIT_DUTY_PCT) * 0.01f + 0.5f);
    g_gti.onTime[2] = (Ifx_TimerValue)((period * PHASE_W_INIT_DUTY_PCT) * 0.01f + 0.5f);

    /* 7) Synchronous shadow transfer: disable update -> set on-times -> apply update */
    IfxGtm_Tom_Timer_disableUpdate(&g_gti.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gti.pwmhl, &g_gti.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gti.timer);
}

/**
 * Update on-times in a cyclic ramp within min/max thresholds (respecting minPulse)
 * and synchronously apply at the PWM period boundary.
 */
void updateGtmTom3phInvDuty(void)
{
    const uint32 period = IfxGtm_Tom_Timer_getPeriod(&g_gti.timer);

    /* Step is a fixed fraction of the period */
    const Ifx_TimerValue step = (Ifx_TimerValue)(period / PWM_DUTY_STEP_DENOMINATOR);

    /* Respect configured minPulse on both edges */
    const Ifx_TimerValue minTicks = gtm_usToTicks(&g_gti.timer, PWM_MIN_PULSE_US);
    const Ifx_TimerValue maxTicks = (period > minTicks) ? (Ifx_TimerValue)(period - minTicks) : (Ifx_TimerValue)1U;

    /* Advance each phase and wrap within [min, max] */
    {
        Ifx_TimerValue v;
        v = g_gti.onTime[0] + step; g_gti.onTime[0] = (v > maxTicks) ? minTicks : v;
        v = g_gti.onTime[1] + step; g_gti.onTime[1] = (v > maxTicks) ? minTicks : v;
        v = g_gti.onTime[2] + step; g_gti.onTime[2] = (v > maxTicks) ? minTicks : v;
    }

    /* Synchronous update via TGC shadow transfer */
    IfxGtm_Tom_Timer_disableUpdate(&g_gti.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gti.pwmhl, &g_gti.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gti.timer);
}
