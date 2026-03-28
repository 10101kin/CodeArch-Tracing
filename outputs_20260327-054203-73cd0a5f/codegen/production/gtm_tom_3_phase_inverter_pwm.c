/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief GTM TOM three-phase inverter PWM driver (TC3xx)
 *
 * Behavior summary:
 *  - Enables GTM and FXCLK domain
 *  - Configures TOM1 timer base on channel 0, 20 kHz, FXCLK0 clock
 *  - Sets up three complementary high/low channels (U,V,W) on P00.[2..7]
 *  - Center-aligned PWM, 0.5 us dead-time, 1.0 us min-pulse, push-pull, CMOS auto speed 1
 *  - Applies initial duties: U=25%, V=50%, W=75% using shadow-update
 *  - Provides periodic on-time ramp update with synchronous shadow transfer
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ========================================================================
 * Configuration macros (numeric constants from requirements)
 * ======================================================================== */
#define GTM_TOM_PWM_NUM_CHANNELS         (3U)
#define GTM_TOM_PWM_FREQUENCY_HZ         (20000U)             /* 20 kHz */

/* Timing assumptions (FXCLK0 = 100 MHz, tick = 10 ns) */
#define GTM_TOM_PWM_FXCLK0_FREQ_HZ       (100000000U)
#define GTM_TOM_PWM_TICK_NS              (10U)
#define GTM_TOM_PWM_DEADTIME_TICKS       (50U)                /* 0.5 us */
#define GTM_TOM_PWM_MINPULSE_TICKS       (100U)               /* 1.0 us */
#define GTM_TOM_PWM_PERIOD_TICKS         (GTM_TOM_PWM_FXCLK0_FREQ_HZ / GTM_TOM_PWM_FREQUENCY_HZ)

/* Dead-time and min-pulse in seconds (for driver base configuration) */
#define GTM_TOM_PWM_DEADTIME_S           (0.5e-6f)
#define GTM_TOM_PWM_MINPULSE_S           (1.0e-6f)

/* Initial duty ratios */
#define PHASE_U_INIT_DUTY_FRAC           (0.25f)
#define PHASE_V_INIT_DUTY_FRAC           (0.50f)
#define PHASE_W_INIT_DUTY_FRAC           (0.75f)

/* Duty ramp step as a fraction of the period (fixed fraction per update) */
#define GTM_TOM_PWM_STEP_FRACTION        (0.05f)              /* 5% of current period */

/* ISR priority and LED pin for debug-toggling in ISR */
#define ISR_PRIORITY_ATOM                (10)
#define LED                               &MODULE_P13, 0

/* ========================================================================
 * Pin routing macros (validated mappings for TOM1 on port P00.x)
 * ======================================================================== */
#define PHASE_U_HS                       &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS                       &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS                       &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS                       &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS                       &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS                       &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* ========================================================================
 * Internal state
 * ======================================================================== */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                         /* TOM timer handle */
    IfxGtm_Tom_PwmHl   pwm;                           /* PWM HL handle */
    Ifx_TimerValue     period;                        /* Cached period in ticks */
    Ifx_TimerValue     onTime[GTM_TOM_PWM_NUM_CHANNELS]; /* On-time ticks for U,V,W */
    boolean            initialized;                   /* Init status */
} GtmTom3PhPwm_State;

IFX_STATIC GtmTom3PhPwm_State g_gtaPwm = {0};

/* ========================================================================
 * ISR and callback (kept minimal for production; ISR toggles LED for debug)
 * ======================================================================== */
IFX_INTERRUPT(interruptGtm_TomAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtm_TomAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period callback (empty body, reserved for integration with higher-level frameworks) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================================================================
 * Local pin map arrays for high-side (ccx) and low-side (coutx)
 * ======================================================================== */
static IFX_CONST IfxGtm_Tom_ToutMapP s_tomCcx[GTM_TOM_PWM_NUM_CHANNELS] =
{
    PHASE_U_HS,
    PHASE_V_HS,
    PHASE_W_HS
};

static IFX_CONST IfxGtm_Tom_ToutMapP s_tomCoutx[GTM_TOM_PWM_NUM_CHANNELS] =
{
    PHASE_U_LS,
    PHASE_V_LS,
    PHASE_W_LS
};

/* ========================================================================
 * Public API
 * ======================================================================== */
/**
 * Initialize GTM TOM1 three-phase complementary PWM.
 *
 * Sequence:
 *  - Enable GTM and FXCLK/CLK0 domains
 *  - Configure TOM1 CH0 timer base @ 20 kHz using FXCLK0 clock
 *  - Configure PwmHl for 3 complementary pairs with required IO and timing
 *  - Set center-aligned mode, run timer, and apply initial duties (25/50/75%) via shadow update
 */
void initGtmTomPwm(void)
{
    /* Enable GTM and its FXCLK domain */
    IfxGtm_enable(&MODULE_GTM);
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_TOM_CMU_CLKEN_FXCLK | IFXGTM_TOM_CMU_CLKEN_CLK0));

    /* Configure TOM timer */
    IfxGtm_Tom_Timer_Config timerCfg;
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

    /* Timer base on TOM1 CH0, FXCLK0, 20 kHz */
    /* Note: Field names follow iLLD conventions */
    timerCfg.tom           = IfxGtm_Tom_1;                /* TOM1 */
    timerCfg.timerChannel  = IfxGtm_Tom_Ch_0;             /* base channel CH0 */
    timerCfg.clock         = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0; /* FXCLK0 clock source */
    timerCfg.frequency     = (float32)GTM_TOM_PWM_FREQUENCY_HZ; /* 20 kHz target */

    boolean ok = IfxGtm_Tom_Timer_init(&g_gtaPwm.timer, &timerCfg);
    if (!ok)
    {
        g_gtaPwm.initialized = FALSE;
        return;
    }

    /* Prepare complementary PWM HL configuration for three phases */
    IfxGtm_Tom_PwmHl_Config pwmCfg;
    IfxGtm_Tom_PwmHl_initConfig(&pwmCfg);

    pwmCfg.timer                     = &g_gtaPwm.timer;
    pwmCfg.tom                       = IfxGtm_Tom_1;             /* TOM1 */
    pwmCfg.ccx                       = s_tomCcx;                 /* high-side TOUTs */
    pwmCfg.coutx                     = s_tomCoutx;               /* low-side TOUTs */
    pwmCfg.initPins                  = TRUE;                     /* driver configures pins */

    /* Base (standard HL) configuration */
    pwmCfg.base.channelCount         = GTM_TOM_PWM_NUM_CHANNELS; /* three complementary pairs */
    pwmCfg.base.deadtime             = GTM_TOM_PWM_DEADTIME_S;   /* 0.5 us */
    pwmCfg.base.minPulse             = GTM_TOM_PWM_MINPULSE_S;   /* 1.0 us */
    pwmCfg.base.outputMode           = IfxPort_OutputMode_pushPull;
    pwmCfg.base.padDriver            = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    pwmCfg.base.ccxActiveState       = Ifx_ActiveState_high;     /* high-side active HIGH */
    pwmCfg.base.coutxActiveState     = Ifx_ActiveState_low;      /* low-side  active LOW  */

    ok = IfxGtm_Tom_PwmHl_init(&g_gtaPwm.pwm, &pwmCfg);
    if (!ok)
    {
        g_gtaPwm.initialized = FALSE;
        return;
    }

    /* Center-aligned PWM */
    ok = IfxGtm_Tom_PwmHl_setMode(&g_gtaPwm.pwm, Ifx_Pwm_Mode_centerAligned);
    if (!ok)
    {
        g_gtaPwm.initialized = FALSE;
        return;
    }

    /* Start the TOM timer */
    IfxGtm_Tom_Timer_run(&g_gtaPwm.timer);

    /* Cache current period (ticks) and compute initial on-times */
    g_gtaPwm.period      = (Ifx_TimerValue)GTM_TOM_PWM_PERIOD_TICKS;
    g_gtaPwm.onTime[0]   = (Ifx_TimerValue)((float32)g_gtaPwm.period * PHASE_U_INIT_DUTY_FRAC);
    g_gtaPwm.onTime[1]   = (Ifx_TimerValue)((float32)g_gtaPwm.period * PHASE_V_INIT_DUTY_FRAC);
    g_gtaPwm.onTime[2]   = (Ifx_TimerValue)((float32)g_gtaPwm.period * PHASE_W_INIT_DUTY_FRAC);

    /* Apply initial on-times via shadow-update */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtaPwm.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtaPwm.pwm, &g_gtaPwm.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtaPwm.timer);

    /* Configure a debug LED (optional, used by ISR to toggle) */
    IfxPort_setPinMode(LED, IfxPort_Mode_outputPushPull);

    g_gtaPwm.initialized = TRUE;
}

/**
 * Update the three-phase on-times with a fixed step (fraction of period) and apply synchronously.
 *
 * Behavior:
 *  - Read current period from persistent timer handle/cache
 *  - Compute step = period * fixedFraction
 *  - For each phase: onTime += step; if beyond max threshold, wrap to min threshold
 *  - Perform shadow-update: disableUpdate -> setOnTime -> applyUpdate
 */
void updateGtmTomPwmDutyCycles(void)
{
    if (!g_gtaPwm.initialized)
    {
        return;
    }

    const Ifx_TimerValue period = g_gtaPwm.period; /* cached */

    /* Compute step and thresholds */
    Ifx_TimerValue step = (Ifx_TimerValue)((float32)period * (float32)GTM_TOM_PWM_STEP_FRACTION);
    if (step == 0U)
    {
        step = 1U; /* ensure forward progress */
    }

    const Ifx_TimerValue tMin = (Ifx_TimerValue)GTM_TOM_PWM_MINPULSE_TICKS;
    const Ifx_TimerValue tMax = (Ifx_TimerValue)(period - GTM_TOM_PWM_MINPULSE_TICKS);

    /* Update U, V, W on-times with wrap */
    g_gtaPwm.onTime[0] = (g_gtaPwm.onTime[0] + step);
    if (g_gtaPwm.onTime[0] > tMax) { g_gtaPwm.onTime[0] = tMin; }

    g_gtaPwm.onTime[1] = (g_gtaPwm.onTime[1] + step);
    if (g_gtaPwm.onTime[1] > tMax) { g_gtaPwm.onTime[1] = tMin; }

    g_gtaPwm.onTime[2] = (g_gtaPwm.onTime[2] + step);
    if (g_gtaPwm.onTime[2] > tMax) { g_gtaPwm.onTime[2] = tMin; }

    /* Apply synchronously via shadow-update */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtaPwm.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtaPwm.pwm, &g_gtaPwm.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtaPwm.timer);
}
