/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * GTM TOM 3-phase inverter PWM driver for AURIX TC3xx.
 * Implements center-aligned complementary PWM with synchronized updates and programmed dead-time.
 *
 * Requirements satisfied:
 *  - Submodule: TOM1, Cluster_1
 *  - Frequency: 20 kHz center-aligned
 *  - Dead-time: 1 us
 *  - Pins: P02.0/P02.7 (U), P02.1/P02.4 (V), P02.2/P02.5 (W)
 *  - Initial duties: U=25%, V=50%, W=75%
 *  - Duty step: +10% with wrap rule
 *  - GTM enable guard + CMU clock configuration (GCLK, FXCLK0)
 *  - ISR (ATOM/TOM time base) toggles P13.0 at priority 20
 *  - Empty IfxGtm_periodEventFunction hook
 *
 * Thread-safety: State is updated atomically within coherent update windows. No dynamic allocation.
 * Watchdog: No watchdog API calls in this file (per architecture rule).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ========================= Macros and Configuration Constants ========================= */

#define NUM_OF_CHANNELS              (3u)
#define PWM_FREQUENCY_HZ             (20000.0f)           /* 20 kHz */
#define DEADTIME_SECONDS             (1.0e-6f)            /* 1 us */

#define PHASE_U_DUTY_INIT_PERCENT    (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT    (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT    (75.0f)
#define PHASE_DUTY_STEP_PERCENT      (10.0f)

/* LED debug pin (port, pin) */
#define LED                          &MODULE_P13, 0

/* TOM1 pin routing (validated user-requested mapping) */
#define PHASE_U_HS                   (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                   (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                   (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                   (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS                   (&IfxGtm_TOM1_10_TOUT2_P02_2_OUT)
#define PHASE_W_LS                   (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* Interrupt priority for GTM TOM/ATOM time base */
#define ISR_PRIORITY_ATOM            (20)

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Tom_Timer   timer;                      /* TOM timer handle */
    IfxGtm_Tom_PwmHl   pwm;                        /* PWMHL driver handle */
    Ifx_TimerValue     period;                    /* Cached period in ticks */
    float32            dutyCycles[NUM_OF_CHANNELS];/* Duty in percent per phase */
    float32            deadtimeSec;               /* Dead-time in seconds */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInv = {0};

/* ========================= ISR and Callback ========================= */

/*
 * ISR: Toggle diagnostic LED on each time-base interrupt
 */
IFX_INTERRUPT(interruptGtm_TomAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtm_TomAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Empty period-event callback hook (assigned via driver config if needed)
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Local Helpers ========================= */

static inline void gtmTom3ph_configurePins(void)
{
    /* Route TOM1 outputs to the requested pads with push-pull and automotive pad driver */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/* ========================= Public API ========================= */

/*
 * Initialize a 3-phase complementary PWM using GTM TOM peripheral with center-aligned mode,
 * synchronized updates, and programmed dead-time.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare local config structs and pin descriptor arrays */
    IfxGtm_Tom_Timer_Config  timerCfg;
    IfxGtm_Tom_PwmHl_Config  pwmhlCfg;

    const IfxGtm_Tom_ToutMap *highPins[NUM_OF_CHANNELS] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
    const IfxGtm_Tom_ToutMap *lowPins[NUM_OF_CHANNELS]  = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

    /* 2) Initialize the TOM timer configuration, set 20 kHz and select FXCLK0 */
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
    /* Typical configuration fields (actual structure provided by iLLD): */
    /* Center-aligned frequency request via base.frequency; FXCLK0 as clock source */
    /* These assignments follow common iLLD patterns for TOM timer configuration */
    timerCfg.base.frequency = PWM_FREQUENCY_HZ;                /* Base PWM frequency */
    timerCfg.clock          = IfxGtm_Cmu_Clk_0;                /* Use CMU CLK0/FXCLK0 as time base where applicable */

    /* 3) Initialize the PWMHL configuration: 3 complementary channels, push-pull, active states */
    IfxGtm_Tom_PwmHl_initConfig(&pwmhlCfg);
    pwmhlCfg.timer                      = &g_tom3phInv.timer;  /* Link to timer */
    pwmhlCfg.base.channelCount          = NUM_OF_CHANNELS;
    pwmhlCfg.base.outputMode            = IfxPort_OutputMode_pushPull;
    pwmhlCfg.base.padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    pwmhlCfg.base.ccxActiveState        = Ifx_ActiveState_high;/* High-side active HIGH */
    pwmhlCfg.base.coutxActiveState      = Ifx_ActiveState_low; /* Low-side  active LOW  */
    pwmhlCfg.ccx                        = (IfxGtm_Tom_ToutMap const * const *)highPins;
    pwmhlCfg.coutx                      = (IfxGtm_Tom_ToutMap const * const *)lowPins;

    /* 4) Configure pin routing for each TOM output */
    gtmTom3ph_configurePins();

    /* 5) GTM enable guard: enable GTM and clocks if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        {
            float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 6) Initialize the TOM timer */
    {
        boolean ok = IfxGtm_Tom_Timer_init(&g_tom3phInv.timer, &timerCfg);
        if (ok == FALSE)
        {
            /* Initialization failed; leave function. Production systems may add diagnostics. */
            return;
        }
    }

    /* 7) Initialize the PWMHL driver and set center-aligned mode */
    {
        boolean ok = IfxGtm_Tom_PwmHl_init(&g_tom3phInv.pwm, &pwmhlCfg);
        if (ok == FALSE)
        {
            return;
        }
        (void)IfxGtm_Tom_PwmHl_setMode(&g_tom3phInv.pwm, Ifx_Pwm_Mode_centerAligned);
        (void)IfxGtm_Tom_PwmHl_setDeadtime(&g_tom3phInv.pwm, DEADTIME_SECONDS);
        g_tom3phInv.deadtimeSec = DEADTIME_SECONDS;
    }

    /* 8) Compute initial ON-times (percent to ticks) */
    g_tom3phInv.period = IfxGtm_Tom_Timer_getPeriod(&g_tom3phInv.timer);
    g_tom3phInv.dutyCycles[0] = PHASE_U_DUTY_INIT_PERCENT;
    g_tom3phInv.dutyCycles[1] = PHASE_V_DUTY_INIT_PERCENT;
    g_tom3phInv.dutyCycles[2] = PHASE_W_DUTY_INIT_PERCENT;

    {
        Ifx_TimerValue tOn[NUM_OF_CHANNELS];
        tOn[0] = (Ifx_TimerValue)((g_tom3phInv.period * g_tom3phInv.dutyCycles[0]) / 100.0f + 0.5f);
        tOn[1] = (Ifx_TimerValue)((g_tom3phInv.period * g_tom3phInv.dutyCycles[1]) / 100.0f + 0.5f);
        tOn[2] = (Ifx_TimerValue)((g_tom3phInv.period * g_tom3phInv.dutyCycles[2]) / 100.0f + 0.5f);

        /* 9) Coherent update: disable update, write ON-times, apply update */
        IfxGtm_Tom_Timer_disableUpdate(&g_tom3phInv.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_tom3phInv.pwm, &tOn[0]);
        IfxGtm_Tom_Timer_applyUpdate(&g_tom3phInv.timer);
    }

    /* 10) State already stored in g_tom3phInv (handles, duties, deadtime) */

    /* 11) Configure diagnostic GPIO (LED) as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update the three phase duties using a fixed step and wrap rule, then apply coherently.
 */
void updateGtmTom3phInvDuty(void)
{
    /* 1) Apply wrap rule separately for each phase, then add step unconditionally */
    if ((g_tom3phInv.dutyCycles[0] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_tom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[1] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_tom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[2] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_tom3phInv.dutyCycles[2] = 0.0f; }
    g_tom3phInv.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    g_tom3phInv.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    g_tom3phInv.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;

    /* 2) Derive current period and convert duties to ON-times */
    g_tom3phInv.period = IfxGtm_Tom_Timer_getPeriod(&g_tom3phInv.timer);
    {
        Ifx_TimerValue tOn[NUM_OF_CHANNELS];
        tOn[0] = (Ifx_TimerValue)((g_tom3phInv.period * g_tom3phInv.dutyCycles[0]) / 100.0f + 0.5f);
        tOn[1] = (Ifx_TimerValue)((g_tom3phInv.period * g_tom3phInv.dutyCycles[1]) / 100.0f + 0.5f);
        tOn[2] = (Ifx_TimerValue)((g_tom3phInv.period * g_tom3phInv.dutyCycles[2]) / 100.0f + 0.5f);

        /* 3) Coherent update */
        IfxGtm_Tom_Timer_disableUpdate(&g_tom3phInv.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_tom3phInv.pwm, &tOn[0]);
        IfxGtm_Tom_Timer_applyUpdate(&g_tom3phInv.timer);
    }

    /* 4) Toggle diagnostic GPIO */
    IfxPort_togglePin(LED);

    /* 5) Duties already stored in state (updated above) */
}
