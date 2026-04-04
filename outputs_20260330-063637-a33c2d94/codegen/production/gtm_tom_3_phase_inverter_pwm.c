/*
 * GTM TOM 3-Phase Inverter PWM Driver
 *
 * Implements a 3-phase complementary PWM using GTM TOM1 with center-aligned mode,
 * synchronized updates, and programmed dead-time.
 *
 * Notes:
 * - No watchdog disable calls are present here (must be in Cpu0_Main.c only).
 * - No STM scheduling here; call updateGtmTom3phInvDuty() periodically from main.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD core types */
#include "Ifx_Types.h"

/* GTM / TOM drivers */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"

/* Port / GPIO */
#include "IfxPort.h"

/* ========================================================================== */
/* Macros and Configuration Constants                                         */
/* ========================================================================== */

/* Channel count */
#define TOM3PH_NUM_CHANNELS                (3)

/* PWM base frequency (Hz) */
#define TOM3PH_PWM_FREQUENCY_HZ            (20000.0f)

/* Dead-time (seconds) */
#define TOM3PH_DEADTIME_SECONDS            (1.0e-6f)

/* Initial duties in percent */
#define TOM3PH_INIT_DUTY_U_PCT             (25.0f)
#define TOM3PH_INIT_DUTY_V_PCT             (50.0f)
#define TOM3PH_INIT_DUTY_W_PCT             (75.0f)

/* Duty step and ISR priority */
#define TOM3PH_DUTY_STEP_PCT               (10.0f)
#define ISR_PRIORITY_ATOM                  (20)

/* LED (diagnostic GPIO) P13.0: compound macro used by IfxPort_* APIs */
#define LED                                &MODULE_P13, 0

/* TOM1 pin routing (validated mapping for P02.x) */
#define PHASE_U_HS                         &IfxGtm_TOM1_0_TOUT0_P02_0_OUT
#define PHASE_U_LS                         &IfxGtm_TOM1_0N_TOUT7_P02_7_OUT
#define PHASE_V_HS                         &IfxGtm_TOM1_1_TOUT1_P02_1_OUT
#define PHASE_V_LS                         &IfxGtm_TOM1_12_TOUT4_P02_4_OUT
#define PHASE_W_HS                         &IfxGtm_TOM1_10_TOUT2_P02_2_OUT
#define PHASE_W_LS                         &IfxGtm_TOM1_13_TOUT5_P02_5_OUT

/* Pin electrical configuration */
#define TOM3PH_PIN_OUTPUT_MODE             (IfxPort_OutputMode_pushPull)
#define TOM3PH_PIN_PAD_DRIVER              (IfxPort_PadDriver_cmosAutomotiveSpeed1)

/* ========================================================================== */
/* Internal State                                                             */
/* ========================================================================== */

typedef struct
{
    IfxGtm_Tom_Timer       timer;                             /* TOM timer driver handle */
    IfxGtm_Tom_PwmHl       pwmhl;                             /* TOM PWMHL driver handle */

    Ifx_TimerValue         onTimes[TOM3PH_NUM_CHANNELS];      /* current on-times (ticks) */
    float32                dutyPct[TOM3PH_NUM_CHANNELS];      /* current duties (percent) */
    float32                deadtimeSec;                       /* configured dead-time (sec) */

    const IfxGtm_Tom_ToutMap *hsPins[TOM3PH_NUM_CHANNELS];    /* high-side pins */
    const IfxGtm_Tom_ToutMap *lsPins[TOM3PH_NUM_CHANNELS];    /* low-side pins */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_tom3ph = {0};

/* ========================================================================== */
/* ISR and Period Callback (declared before init per structural rules)        */
/* ========================================================================== */

/* ATOM/TOM debug ISR: toggle LED only */
IFX_INTERRUPT(interruptGtm_TomAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtm_TomAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Empty period-event callback as required */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================================================================== */
/* Local Helpers                                                              */
/* ========================================================================== */

/** Convert duty in percent to ticks based on a given period */
static inline Ifx_TimerValue tom3ph_dutyToTicks(Ifx_TimerValue period, float32 dutyPct)
{
    if (dutyPct <= 0.0f)
    {
        return 0U;
    }
    if (dutyPct >= 100.0f)
    {
        return period;
    }
    return (Ifx_TimerValue)((period * dutyPct) / 100.0f);
}

/* ========================================================================== */
/* Public API                                                                 */
/* ========================================================================== */

/**
 * Initialize a 3-phase complementary PWM using GTM TOM in center-aligned mode.
 * - Configures TOM timer at 20 kHz using FXCLK0
 * - Routes six TOM outputs (U/V/W high and low) on P02.x
 * - Enables synchronized updates and programs dead-time
 * - Sets initial duties: U=25%, V=50%, W=75%
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configs and pin arrays */
    IfxGtm_Tom_Timer_Config   timerCfg;
    IfxGtm_Tom_PwmHl_Config   pwmhlCfg;

    const IfxGtm_Tom_ToutMap *hs[TOM3PH_NUM_CHANNELS] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
    const IfxGtm_Tom_ToutMap *ls[TOM3PH_NUM_CHANNELS] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

    /* 2) Initialize timer config: 20 kHz, FXCLK0 */
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
    /* Base frequency selection; actual field names are provided by iLLD */
    timerCfg.base.frequency = TOM3PH_PWM_FREQUENCY_HZ;
    timerCfg.clock = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0; /* select FXCLK0 */

    /* 5) GTM enable guard with CMU configuration inside the guard */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        {
            float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFreq);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_TOM_CMU_CLKEN_FXCLK | IFXGTM_TOM_CMU_CLKEN_CLK0));
        }
    }

    /* 6) Initialize the TOM timer */
    {
        boolean ok = IfxGtm_Tom_Timer_init(&g_tom3ph.timer, &timerCfg);
        if (ok == FALSE)
        {
            return; /* error path: do not proceed */
        }
    }

    /* 3) Initialize PWMHL configuration: 3 complementary channels, push-pull, pad driver */
    IfxGtm_Tom_PwmHl_initConfig(&pwmhlCfg);

    pwmhlCfg.timer                     = &g_tom3ph.timer;
    pwmhlCfg.base.channelCount         = TOM3PH_NUM_CHANNELS;
    pwmhlCfg.base.outputMode           = TOM3PH_PIN_OUTPUT_MODE;
    pwmhlCfg.base.padDriver            = TOM3PH_PIN_PAD_DRIVER;
    pwmhlCfg.base.ccxActiveState       = Ifx_ActiveState_high; /* high-side active HIGH  */
    pwmhlCfg.base.coutxActiveState     = Ifx_ActiveState_low;  /* low-side  active LOW   */

    pwmhlCfg.ccx[0] = hs[0]; pwmhlCfg.coutx[0] = ls[0];
    pwmhlCfg.ccx[1] = hs[1]; pwmhlCfg.coutx[1] = ls[1];
    pwmhlCfg.ccx[2] = hs[2]; pwmhlCfg.coutx[2] = ls[2];

    /* 4) Configure pin routing explicitly on all six outputs */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)hs[0], TOM3PH_PIN_OUTPUT_MODE, TOM3PH_PIN_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)ls[0], TOM3PH_PIN_OUTPUT_MODE, TOM3PH_PIN_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)hs[1], TOM3PH_PIN_OUTPUT_MODE, TOM3PH_PIN_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)ls[1], TOM3PH_PIN_OUTPUT_MODE, TOM3PH_PIN_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)hs[2], TOM3PH_PIN_OUTPUT_MODE, TOM3PH_PIN_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)ls[2], TOM3PH_PIN_OUTPUT_MODE, TOM3PH_PIN_PAD_DRIVER);

    /* 7) Initialize PWMHL and set center-aligned mode */
    {
        boolean ok = IfxGtm_Tom_PwmHl_init(&g_tom3ph.pwmhl, &pwmhlCfg);
        if (ok == FALSE)
        {
            return; /* error path */
        }
    }

    {
        boolean ok = IfxGtm_Tom_PwmHl_setMode(&g_tom3ph.pwmhl, Ifx_Pwm_Mode_centerAligned);
        if (ok == FALSE)
        {
            return; /* error path */
        }
    }

    /* Program dead-time (seconds) */
    {
        boolean ok = IfxGtm_Tom_PwmHl_setDeadtime(&g_tom3ph.pwmhl, TOM3PH_DEADTIME_SECONDS);
        if (ok == FALSE)
        {
            return; /* error path */
        }
    }

    /* 8) Compute initial ON-times from period and initial duties */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_tom3ph.timer);
        g_tom3ph.dutyPct[0] = TOM3PH_INIT_DUTY_U_PCT;
        g_tom3ph.dutyPct[1] = TOM3PH_INIT_DUTY_V_PCT;
        g_tom3ph.dutyPct[2] = TOM3PH_INIT_DUTY_W_PCT;

        g_tom3ph.onTimes[0] = tom3ph_dutyToTicks(period, g_tom3ph.dutyPct[0]);
        g_tom3ph.onTimes[1] = tom3ph_dutyToTicks(period, g_tom3ph.dutyPct[1]);
        g_tom3ph.onTimes[2] = tom3ph_dutyToTicks(period, g_tom3ph.dutyPct[2]);
        g_tom3ph.deadtimeSec = TOM3PH_DEADTIME_SECONDS;
    }

    /* 9) Coherent update: disable update, write on-times, apply update */
    IfxGtm_Tom_Timer_disableUpdate(&g_tom3ph.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_tom3ph.pwmhl, g_tom3ph.onTimes);
    IfxGtm_Tom_Timer_applyUpdate(&g_tom3ph.timer);

    /* 10) Persist pin arrays in module state */
    g_tom3ph.hsPins[0] = hs[0]; g_tom3ph.lsPins[0] = ls[0];
    g_tom3ph.hsPins[1] = hs[1]; g_tom3ph.lsPins[1] = ls[1];
    g_tom3ph.hsPins[2] = hs[2]; g_tom3ph.lsPins[2] = ls[2];

    /* 11) Configure diagnostic LED pin after PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update U/V/W duties with fixed step and wrap rule, then apply coherently.
 * Wrap rule: if (duty + step) >= 100, set to 0, then always add step.
 */
void updateGtmTom3phInvDuty(void)
{
    /* 1) Apply wrap rule (no loops; explicit for U, V, W) */
    if ((g_tom3ph.dutyPct[0] + TOM3PH_DUTY_STEP_PCT) >= 100.0f) { g_tom3ph.dutyPct[0] = 0.0f; }
    if ((g_tom3ph.dutyPct[1] + TOM3PH_DUTY_STEP_PCT) >= 100.0f) { g_tom3ph.dutyPct[1] = 0.0f; }
    if ((g_tom3ph.dutyPct[2] + TOM3PH_DUTY_STEP_PCT) >= 100.0f) { g_tom3ph.dutyPct[2] = 0.0f; }
    g_tom3ph.dutyPct[0] += TOM3PH_DUTY_STEP_PCT;
    g_tom3ph.dutyPct[1] += TOM3PH_DUTY_STEP_PCT;
    g_tom3ph.dutyPct[2] += TOM3PH_DUTY_STEP_PCT;

    /* 2) Derive period and convert duties to on-times */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_tom3ph.timer);
        g_tom3ph.onTimes[0] = tom3ph_dutyToTicks(period, g_tom3ph.dutyPct[0]);
        g_tom3ph.onTimes[1] = tom3ph_dutyToTicks(period, g_tom3ph.dutyPct[1]);
        g_tom3ph.onTimes[2] = tom3ph_dutyToTicks(period, g_tom3ph.dutyPct[2]);
    }

    /* 3) Coherent update: disable update, set new on-times, apply update */
    IfxGtm_Tom_Timer_disableUpdate(&g_tom3ph.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_tom3ph.pwmhl, g_tom3ph.onTimes);
    IfxGtm_Tom_Timer_applyUpdate(&g_tom3ph.timer);

    /* 4) Optional diagnostic toggle to mark update */
    IfxPort_togglePin(LED);
}
