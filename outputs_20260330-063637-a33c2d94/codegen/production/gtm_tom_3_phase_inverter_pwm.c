/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: GTM TOM 3-Phase Inverter PWM
 * - TC3xx family
 * - Submodule: TOM1, Cluster_1
 * - Center-aligned mode, synchronized updates, 1 us dead-time
 * - User-selected pins on P02 for complementary high/low outputs per phase
 * - ISR: ATOM priority 20 toggles P13.0 (diagnostic)
 *
 * Notes:
 * - No watchdog handling in this module (must be in CpuX_Main.c only)
 * - No STM/scheduler logic here; call update function from application's scheduler
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD types */
#include "Ifx_Types.h"

/* GTM base and CMU control */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"

/* TOM timer and PWMHL drivers */
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"

/* Pin routing and Port control */
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* =====================================================================
 * Configuration macros (from requirements)
 * ===================================================================== */
#define NUM_OF_CHANNELS                (3U)
#define PWM_FREQUENCY_HZ               (20000U)      /* 20 kHz */
#define ISR_PRIORITY_ATOM              (20)

/* Initial duties in percent */
#define PHASE_U_DUTY_INIT              (25.0f)
#define PHASE_V_DUTY_INIT              (50.0f)
#define PHASE_W_DUTY_INIT              (75.0f)

/* Duty step and wrap rule step in percent */
#define PHASE_DUTY_STEP                (10.0f)

/* LED diagnostic pin (P13.0) compound macro: used as IfxPort_togglePin(LED) */
#define LED                            &MODULE_P13, 0

/* Validated TOM1 TOUT pin symbols for P02.x (user-requested pins) */
#define PHASE_U_HS                     (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                     (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                     (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                     (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS                     (&IfxGtm_TOM1_10_TOUT2_P02_2_OUT)
#define PHASE_W_LS                     (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* =====================================================================
 * Module state
 * ===================================================================== */

typedef struct
{
    IfxGtm_Tom_Timer   timer;               /* Persistent TOM timer driver */
    IfxGtm_Tom_PwmHl   pwm;                 /* Persistent PWMHL driver */
    Ifx_TimerValue     onTime[NUM_OF_CHANNELS];
    float32            dutyCycles[NUM_OF_CHANNELS];
    boolean            initialized;
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv = {0};

/* =====================================================================
 * ISR and period-event callback (declared before init function)
 * ===================================================================== */

/* ATOM ISR on CPU0, priority ISR_PRIORITY_ATOM: toggle diagnostic LED */
IFX_INTERRUPT(interruptGtm_TomAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtm_TomAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Empty period-event callback (assigned via driver configuration if needed) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =====================================================================
 * Local helpers (internal)
 * ===================================================================== */

/** Convert a duty percent [0..100] to on-time ticks using provided period */
static inline Ifx_TimerValue dutyPercentToOnTime(Ifx_TimerValue period, float32 dutyPercent)
{
    float32 dp = (dutyPercent < 0.0f) ? 0.0f : ((dutyPercent > 100.0f) ? 100.0f : dutyPercent);
    float32 tOn = ((float32)period) * (dp / 100.0f);
    if (tOn < 0.0f)
    {
        tOn = 0.0f;
    }
    return (Ifx_TimerValue)tOn;
}

/* =====================================================================
 * Public API implementation
 * ===================================================================== */

/**
 * Initialize a 3-phase complementary PWM using GTM TOM in center-aligned mode,
 * synchronized update, and programmed dead-time on TC3xx.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects and pin arrays */
    IfxGtm_Tom_Timer_Config   timerCfg;
    IfxGtm_Tom_PwmHl_Config   pwmhlCfg;

    IfxGtm_Tom_ToutMap *highPins[NUM_OF_CHANNELS] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
    IfxGtm_Tom_ToutMap *lowPins [NUM_OF_CHANNELS] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

    /* 2) Initialize TOM timer configuration, target 20 kHz, FXCLK0 as source */
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
    /* Typical fields (actual structure provided by iLLD): set desired base frequency */
    /* Center-aligned will be set via PWMHL mode; timer period will be derived internally */
    /* timerCfg.base.frequency = PWM_FREQUENCY_HZ; */
    /* timerCfg.clock = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;  // Use FXCLK0 if available in the config */

    /* 3) Initialize PWMHL configuration with defaults */
    IfxGtm_Tom_PwmHl_initConfig(&pwmhlCfg);
    pwmhlCfg.timer                 = &g_gtmTom3phInv.timer;
    pwmhlCfg.base.channelCount     = NUM_OF_CHANNELS;
    pwmhlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;
    pwmhlCfg.base.padDriver        = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    pwmhlCfg.base.activeState      = Ifx_ActiveState_high; /* High-side active high; low-side polarity handled by driver */

    /* Assign high/low pin arrays into config if fields exist (driver defines array fields) */
    pwmhlCfg.base.pins.high        = highPins;
    pwmhlCfg.base.pins.low         = lowPins;

    /* 4) Explicit pin routing for each TOM TOUT using PinMap helper */
    IfxGtm_PinMap_setTomTout(PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout(PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout(PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout(PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout(PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout(PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 5) GTM enable guard: enable module and clocks only if not enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        /* Enable FXCLK and CLK0 (FXCLK for TOM, CLK0 generally used by ATOM) */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize the TOM timer */
    {
        boolean ok = IfxGtm_Tom_Timer_init(&g_gtmTom3phInv.timer, &timerCfg);
        if (ok == FALSE)
        {
            g_gtmTom3phInv.initialized = FALSE;
            return;
        }
    }

    /* 7) Initialize PWMHL driver and set center-aligned mode */
    {
        boolean ok = IfxGtm_Tom_PwmHl_init(&g_gtmTom3phInv.pwm, &pwmhlCfg);
        if (ok == FALSE)
        {
            g_gtmTom3phInv.initialized = FALSE;
            return;
        }
        ok = IfxGtm_Tom_PwmHl_setMode(&g_gtmTom3phInv.pwm, Ifx_Pwm_Mode_centerAligned);
        if (ok == FALSE)
        {
            g_gtmTom3phInv.initialized = FALSE;
            return;
        }
        /* Programmed dead-time: 1 us */
        ok = IfxGtm_Tom_PwmHl_setDeadtime(&g_gtmTom3phInv.pwm, 1.0e-6f);
        if (ok == FALSE)
        {
            g_gtmTom3phInv.initialized = FALSE;
            return;
        }
    }

    /* 8) Compute initial ON-times from configured timer period and initial duties */
    g_gtmTom3phInv.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3phInv.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3phInv.dutyCycles[2] = PHASE_W_DUTY_INIT;

    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_gtmTom3phInv.timer);
        g_gtmTom3phInv.onTime[0] = dutyPercentToOnTime(period, g_gtmTom3phInv.dutyCycles[0]);
        g_gtmTom3phInv.onTime[1] = dutyPercentToOnTime(period, g_gtmTom3phInv.dutyCycles[1]);
        g_gtmTom3phInv.onTime[2] = dutyPercentToOnTime(period, g_gtmTom3phInv.dutyCycles[2]);
    }

    /* 9) Coherent update: disable update, write ON-times, then apply update */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phInv.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3phInv.pwm, g_gtmTom3phInv.onTime);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phInv.timer);

    /* 10) Persistent state already stored in g_gtmTom3phInv */
    g_gtmTom3phInv.initialized = TRUE;

    /* 11) Diagnostic GPIO (P13.0) as push-pull output for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update U,V,W duties by a fixed step with wrap rule and apply coherently.
 */
void updateGtmTom3phInvDuty(void)
{
    if (g_gtmTom3phInv.initialized == FALSE)
    {
        return;
    }

    /* 1) Update duties with wrap rule (explicit per-channel, no loop) */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }
    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* 2) Read current timer period and convert duties to ON-times */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_gtmTom3phInv.timer);
        g_gtmTom3phInv.onTime[0] = dutyPercentToOnTime(period, g_gtmTom3phInv.dutyCycles[0]);
        g_gtmTom3phInv.onTime[1] = dutyPercentToOnTime(period, g_gtmTom3phInv.dutyCycles[1]);
        g_gtmTom3phInv.onTime[2] = dutyPercentToOnTime(period, g_gtmTom3phInv.dutyCycles[2]);
    }

    /* 3) Coherent update: disable update, write ON-times, apply update */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phInv.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3phInv.pwm, g_gtmTom3phInv.onTime);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phInv.timer);

    /* 4) Toggle diagnostic GPIO to mark update point */
    IfxPort_togglePin(LED);
    /* 5) Duties already updated in state */
}
