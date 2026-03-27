/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief GTM TOM three-phase complementary PWM driver implementation (TC3xx)
 *
 * Implementation follows iLLD patterns and the module design:
 * - GTM enable + CMU clock setup
 * - TOM timer as base (20 kHz, FXCLK0)
 * - Six TOM outputs routed to P00.2..P00.7
 * - PwmHl for 3 complementary pairs with dead-time and minPulse
 * - Center-aligned mode and synchronous shadow transfer on updates
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ===================== Configuration Macros (from requirements) ===================== */
#define PWM_NUM_PHASES                 (3U)
#define PWM_BASE_FREQUENCY_HZ          (20000.0f)     /* 20 kHz */
#define PWM_ALIGNMENT_CENTER           (1)

/* GTM CMU computed ticks at FXCLK0 = 100 MHz (provided) */
#define PWM_PERIOD_TICKS               (5000U)        /* 100 MHz / 20 kHz */
#define PWM_DEADTIME_TICKS             (50U)          /* 0.5 us at 100 MHz */
#define PWM_MINPULSE_TICKS             (100U)         /* 1.0 us at 100 MHz */

/* Initial duties (as fraction of period) */
#define PWM_INIT_DUTY_U_FRAC           (0.25f)
#define PWM_INIT_DUTY_V_FRAC           (0.50f)
#define PWM_INIT_DUTY_W_FRAC           (0.75f)

/* Duty ramp step as fraction of current period (fixed fraction) */
#define PWM_DUTY_STEP_FRAC             (0.05f)        /* 5% of period per call */

/* TOM1 pin routing: channel mapping as per requirements */
#define PHASE_U_HS     (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT) /* TOM1 CH1 -> P00.2 */
#define PHASE_U_LS     (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT) /* TOM1 CH2 -> P00.3 */
#define PHASE_V_HS     (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT) /* TOM1 CH3 -> P00.4 */
#define PHASE_V_LS     (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT) /* TOM1 CH4 -> P00.5 */
#define PHASE_W_HS     (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT) /* TOM1 CH5 -> P00.6 */
#define PHASE_W_LS     (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT) /* TOM1 CH6 -> P00.7 */

/* LED macro (port, pin) for ISR debug toggle */
#define LED            &MODULE_P13, 0

/* ISR priority macro */
#define ISR_PRIORITY_ATOM              (20)

/* ============================= Module State ======================================== */

typedef struct
{
    IfxGtm_Tom_Timer   timer;                     /* TOM timer base */
    IfxGtm_Tom_PwmHl   pwmhl;                     /* PwmHl driver for complementary pairs */
    Ifx_TimerValue     onTime[PWM_NUM_PHASES];    /* per-phase on-times (ticks) */
    Ifx_TimerValue     minPulse;                  /* configured min pulse (ticks) */
    Ifx_TimerValue     deadTime;                  /* configured dead-time (ticks) */
    float32            baseFreqHz;                /* configured PWM base frequency */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;     /* persistent module state */

/* ======================= ISR and Period Callback (per rules) ======================= */

/* ISR declaration: toggles debug LED */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Empty period-event callback placeholder (assigned via interrupt config in unified flows) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================ Local Utilities ====================================== */

static inline Ifx_TimerValue clampMin(Ifx_TimerValue v, Ifx_TimerValue minV)
{
    return (v < minV) ? minV : v;
}

/* ============================ Public API =========================================== */

/**
 * @brief Initialize GTM for 3-phase complementary PWM on TOM1 using TOM timer as base and PwmHl for paired outputs.
 */
void initGtmTom3phInv(void)
{
    /* 1) Enable GTM module and enable FXCLK domain (and CLK0 as used by TOM timers) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 modFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, modFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, modFreq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Initialize TOM timer configuration for 20 kHz base, FXCLK0 source, TOM1 cluster, base channel */
    IfxGtm_Tom_Timer_Config timerCfg;
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

    /* Configure base frequency and typical TOM1 timer settings */
    timerCfg.frequency     = PWM_BASE_FREQUENCY_HZ;   /* 20 kHz */
    timerCfg.tom           = IfxGtm_Tom_1;            /* TOM1 cluster */
    timerCfg.timerChannel  = IfxGtm_Tom_Ch_0;         /* Use CH0 as base counter for TGC */
    timerCfg.clock         = IfxGtm_Cmu_Fxclk_0;      /* FXCLK0 as clock source */

    if (FALSE == IfxGtm_Tom_Timer_init(&g_gtmTom3phInv.timer, &timerCfg))
    {
        /* Initialization failed; do not proceed */
        return;
    }

    /* 3) Route six TOM outputs (U,V,W complementary pairs) to pads, push-pull, requested pad driver */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) Initialize TOM PwmHl configuration: 3 channel pairs, dead-time 0.5us, minPulse 1.0us, push-pull, active-high */
    IfxGtm_Tom_PwmHl_Config hlCfg;
    IfxGtm_Tom_PwmHl_initConfig(&hlCfg);

    /* Bind to initialized timer (base) */
    hlCfg.timer = &g_gtmTom3phInv.timer;

    /* Standard interface configuration (pair count, output mode, active state) */
    hlCfg.base.channelCount   = PWM_NUM_PHASES;                   /* three complementary pairs */
    hlCfg.base.outputMode     = IfxPort_OutputMode_pushPull;      /* push-pull */
    hlCfg.base.ccxActiveState = Ifx_ActiveState_high;             /* high-side active HIGH */
    hlCfg.base.coutxActiveState = Ifx_ActiveState_high;           /* low-side  active HIGH */

    /* Channel mapping: TOM1 pairs (1/2), (3/4), (5/6) */
    hlCfg.ccx[0] = IfxGtm_Tom_Ch_1; hlCfg.coutx[0] = IfxGtm_Tom_Ch_2; /* U */
    hlCfg.ccx[1] = IfxGtm_Tom_Ch_3; hlCfg.coutx[1] = IfxGtm_Tom_Ch_4; /* V */
    hlCfg.ccx[2] = IfxGtm_Tom_Ch_5; hlCfg.coutx[2] = IfxGtm_Tom_Ch_6; /* W */

    if (FALSE == IfxGtm_Tom_PwmHl_init(&g_gtmTom3phInv.pwmhl, &hlCfg))
    {
        return;
    }

    /* Program dead-time and minimum pulse in timer ticks */
    g_gtmTom3phInv.deadTime = (Ifx_TimerValue)PWM_DEADTIME_TICKS;
    g_gtmTom3phInv.minPulse = (Ifx_TimerValue)PWM_MINPULSE_TICKS;
    IfxGtm_Tom_PwmHl_setDeadtime(&g_gtmTom3phInv.pwmhl, g_gtmTom3phInv.deadTime);
    IfxGtm_Tom_PwmHl_setMinPulse(&g_gtmTom3phInv.pwmhl, g_gtmTom3phInv.minPulse);

    /* Center-aligned PWM mode */
    (void)IfxGtm_Tom_PwmHl_setMode(&g_gtmTom3phInv.pwmhl, Ifx_Pwm_Mode_centerAligned);

    /* 5) Update timer input frequency from CMU, then start timer */
    IfxGtm_Tom_Timer_updateInputFrequency(&g_gtmTom3phInv.timer);
    IfxGtm_Tom_Timer_run(&g_gtmTom3phInv.timer);

    /* 6) Compute initial on-times at 25%, 50%, 75% of current period and store */
    {
        float32 period = IfxGtm_Tom_Timer_getPeriod(&g_gtmTom3phInv.timer);
        g_gtmTom3phInv.onTime[0] = (Ifx_TimerValue)(period * PWM_INIT_DUTY_U_FRAC);
        g_gtmTom3phInv.onTime[1] = (Ifx_TimerValue)(period * PWM_INIT_DUTY_V_FRAC);
        g_gtmTom3phInv.onTime[2] = (Ifx_TimerValue)(period * PWM_INIT_DUTY_W_FRAC);

        /* Ensure initial on-times respect minPulse constraints */
        g_gtmTom3phInv.onTime[0] = clampMin(g_gtmTom3phInv.onTime[0], g_gtmTom3phInv.minPulse);
        g_gtmTom3phInv.onTime[1] = clampMin(g_gtmTom3phInv.onTime[1], g_gtmTom3phInv.minPulse);
        g_gtmTom3phInv.onTime[2] = clampMin(g_gtmTom3phInv.onTime[2], g_gtmTom3phInv.minPulse);
    }

    /* 7) Synchronous shadow-transfer sequence to apply initial on-times */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phInv.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3phInv.pwmhl, &g_gtmTom3phInv.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phInv.timer);

    /* Optional: configure LED pin after PWM init (debug) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Update three phase on-times in a cyclic ramp and apply synchronously at the PWM period boundary.
 */
void updateGtmTom3phInvDuty(void)
{
    /* 1) Read current period and compute step and limits */
    float32 periodF = IfxGtm_Tom_Timer_getPeriod(&g_gtmTom3phInv.timer);
    Ifx_TimerValue period = (Ifx_TimerValue)periodF;
    Ifx_TimerValue step   = (Ifx_TimerValue)(periodF * PWM_DUTY_STEP_FRAC);

    /* Respect minPulse at both start and end */
    Ifx_TimerValue minTh = (Ifx_TimerValue)PWM_MINPULSE_TICKS;
    Ifx_TimerValue maxTh = (Ifx_TimerValue)((period > (Ifx_TimerValue)PWM_MINPULSE_TICKS) ? (period - (Ifx_TimerValue)PWM_MINPULSE_TICKS) : period);

    /* 2) Ramp each phase with wrap within [minTh, maxTh] */
    {
        Ifx_TimerValue u = g_gtmTom3phInv.onTime[0] + step;
        Ifx_TimerValue v = g_gtmTom3phInv.onTime[1] + step;
        Ifx_TimerValue w = g_gtmTom3phInv.onTime[2] + step;

        if (u > maxTh) { u = minTh; }
        if (v > maxTh) { v = minTh; }
        if (w > maxTh) { w = minTh; }

        g_gtmTom3phInv.onTime[0] = u;
        g_gtmTom3phInv.onTime[1] = v;
        g_gtmTom3phInv.onTime[2] = w;
    }

    /* 3) Perform a synchronous update using the TOM TGC shadow-transfer sequence */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phInv.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3phInv.pwmhl, &g_gtmTom3phInv.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phInv.timer);
}
