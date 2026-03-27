/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM on TC3xx.
 *
 * Implements:
 *  - initGtmTomPwm()
 *  - updateGtmTomPwmDutyCycles()
 *
 * Dependencies used (per spec):
 *  - Ifx_Types.h
 *  - IfxGtm_Tom_PwmHl.h
 *  - IfxGtm_Tom_Timer.h
 *  - IfxGtm_PinMap.h
 *  - IfxPort.h
 *
 * Notes:
 *  - No watchdog API here (must be only in CpuX_Main.c).
 *  - Uses shadow-update sequence for synchronous PWM updates.
 *  - Center-aligned mode with complementary outputs via PwmHl.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm.h"
#include "IfxPort.h"

/* -------------------------------------------------------------------------- */
/* Configuration macros (from requirements)                                    */
/* -------------------------------------------------------------------------- */
#ifndef IFX_STATIC
#define IFX_STATIC static
#endif

#define NUM_CHANNELS                    (3)
#define PWM_FREQUENCY_HZ                (20000.0f)        /* 20 kHz */
#define PWM_DEADTIME_TICKS              (50u)             /* 0.5 us at 10 ns tick */
#define PWM_MIN_PULSE_TICKS             (100u)            /* 1.0 us at 10 ns tick */
#define PWM_DUTY_STEP_FRACTION          (0.1f)            /* 10% of current period (in ticks) per update */

/* LED macro (port,pin compound) */
#define LED                             &MODULE_P13, 0

/* Validated/reference pin symbols for TOM1 (U,V,W phases) */
#define PHASE_U_HS                      &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS                      &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS                      &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS                      &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS                      &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS                      &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* Initial duty ratios (fractions of period) */
#define DUTY_U_INIT_FRACTION            (0.25f)           /* 25% */
#define DUTY_V_INIT_FRACTION            (0.50f)           /* 50% */
#define DUTY_W_INIT_FRACTION            (0.75f)           /* 75% */

/* Interrupt priority for demo ISR (naming per project rule) */
#define ISR_PRIORITY_ATOM               (20)

/* -------------------------------------------------------------------------- */
/* Internal state                                                             */
/* -------------------------------------------------------------------------- */

typedef struct
{
    IfxGtm_Tom_Timer    timer;                 /* Persistent TOM timer handle */
    IfxGtm_Tom_PwmHl    pwmhl;                 /* Persistent PWM HL handle    */
    Ifx_TimerValue      onTime[NUM_CHANNELS];  /* On-time ticks for U,V,W     */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gta;

/* -------------------------------------------------------------------------- */
/* ISR and period callback (per structural rules)                              */
/* -------------------------------------------------------------------------- */

/* External ISR declaration (priority macro must match) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    /* Minimal ISR: toggle debug LED */
    IfxPort_togglePin(LED);
}

/* Period-event callback: MUST be visible and have empty body */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* -------------------------------------------------------------------------- */
/* Local helpers                                                               */
/* -------------------------------------------------------------------------- */

static void gta_configureGtmClocks(void)
{
    /* Enable GTM and configure CMU clocks only once */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        /* Set GCLK to module frequency and enable FXCLK, CLK0 as per pattern */
        {
            float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
            /* Enable FXCLK and CLK0 domains */
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                  */
/* -------------------------------------------------------------------------- */

/*
 * initGtmTomPwm
 *
 * Behavior:
 *  - Enable GTM and FXCLK domain; configure TOM timer on TOM1 base channel 0
 *  - Initialize timer to 20 kHz using FXCLK0, then init PwmHl for 3 channels
 *  - Setup high/low complementary pins, deadtime=0.5us, minPulse=1.0us
 *  - Push-pull, cmosAutomotiveSpeed1, active-high on both sides
 *  - Center-aligned mode; start timer; apply initial 25/50/75% duties via shadow update
 *  - Persist timer, pwmhl and on-time array in module state
 */
void initGtmTomPwm(void)
{
    /* 1) GTM enable + CMU clocking per mandatory pattern */
    gta_configureGtmClocks();

    /* 2) Init TOM timer */
    {
        IfxGtm_Tom_Timer_Config tmrCfg;
        IfxGtm_Tom_Timer_initConfig(&tmrCfg, &MODULE_GTM);

        /* Select TOM1 as instance, base channel CH0 as time base, FXCLK0 as clock source */
        tmrCfg.tom          = IfxGtm_Tom_1;
        tmrCfg.timerChannel = IfxGtm_Tom_Ch_0;                  /* base channel */
        tmrCfg.clock        = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;   /* FXCLK0 */
        /* Frequency will be set after init to ensure any derived internals are updated */

        if (FALSE == IfxGtm_Tom_Timer_init(&g_gta.timer, &tmrCfg))
        {
            /* Initialization failed; do not proceed */
            return;
        }

        /* Target PWM frequency 20 kHz */
        IfxGtm_Tom_Timer_setFrequency(&g_gta.timer, PWM_FREQUENCY_HZ);
    }

    /* 3) Prepare PwmHl configuration for 3 complementary channels (U,V,W) */
    {
        /* High-side and low-side TOUT pin arrays (parallel) */
        const IfxGtm_Tom_ToutMap *channelsHigh[NUM_CHANNELS] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
        const IfxGtm_Tom_ToutMap *channelsLow [NUM_CHANNELS] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

        IfxGtm_Tom_PwmHl_Config hlCfg;
        IfxGtm_Tom_PwmHl_initConfig(&hlCfg);

        /* Link timer and TOM instance */
        hlCfg.timer                 = &g_gta.timer;
        hlCfg.tom                   = IfxGtm_Tom_1;

        /* Channel/pin assignment */
        hlCfg.base.channelCount     = NUM_CHANNELS;
        hlCfg.pins.channel          = &channelsHigh[0];
        hlCfg.pins.complementary    = &channelsLow[0];

        /* Output characteristics */
        hlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;
        hlCfg.base.padDriver        = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        /* Active-high polarity for both high- and low-side per requirements */
        hlCfg.base.polarity.high    = Ifx_ActiveState_high;
        hlCfg.base.polarity.low     = Ifx_ActiveState_high;

        if (FALSE == IfxGtm_Tom_PwmHl_init(&g_gta.pwmhl, &hlCfg))
        {
            /* Initialization failed; do not proceed */
            return;
        }

        /* Apply deadtime and min-pulse (in timer ticks) */
        IfxGtm_Tom_PwmHl_setDeadtime(&g_gta.pwmhl, (Ifx_TimerValue)PWM_DEADTIME_TICKS, (Ifx_TimerValue)PWM_DEADTIME_TICKS);
        IfxGtm_Tom_PwmHl_setMinPulse(&g_gta.pwmhl, (Ifx_TimerValue)PWM_MIN_PULSE_TICKS);

        /* Set center-aligned PWM mode */
        (void)IfxGtm_Tom_PwmHl_setMode(&g_gta.pwmhl, Ifx_Pwm_Mode_centerAligned);
    }

    /* 4) Start the TOM timer */
    IfxGtm_Tom_Timer_run(&g_gta.timer);

    /* 5) Compute initial on-times from current period and apply via shadow-update */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_gta.timer);
        g_gta.onTime[0] = (Ifx_TimerValue)((float32)period * DUTY_U_INIT_FRACTION);
        g_gta.onTime[1] = (Ifx_TimerValue)((float32)period * DUTY_V_INIT_FRACTION);
        g_gta.onTime[2] = (Ifx_TimerValue)((float32)period * DUTY_W_INIT_FRACTION);

        IfxGtm_Tom_Timer_disableUpdate(&g_gta.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_gta.pwmhl, &g_gta.onTime[0]);
        IfxGtm_Tom_Timer_applyUpdate(&g_gta.timer);
    }

    /* 6) Configure debug LED pin as output (after PWM init) */
    IfxPort_setPinMode(LED, IfxPort_OutputMode_pushPull);
}

/*
 * updateGtmTomPwmDutyCycles
 *
 * Behavior:
 *  - Read current timer period
 *  - Compute step as fixed fraction of period (PWM_DUTY_STEP_FRACTION)
 *  - Compute min/max thresholds using configured min-pulse
 *  - For each phase: add step; if exceeds max, wrap to min
 *  - Apply updated on-times synchronously via shadow update
 */
void updateGtmTomPwmDutyCycles(void)
{
    /* Read current period and compute thresholds/step in ticks */
    const Ifx_TimerValue period  = IfxGtm_Tom_Timer_getPeriod(&g_gta.timer);
    const Ifx_TimerValue minOn   = (Ifx_TimerValue)PWM_MIN_PULSE_TICKS;
    const Ifx_TimerValue maxOn   = (Ifx_TimerValue)(period - (Ifx_TimerValue)PWM_MIN_PULSE_TICKS);
    const Ifx_TimerValue step    = (Ifx_TimerValue)((float32)period * PWM_DUTY_STEP_FRACTION);

    /* Update U phase */
    {
        Ifx_TimerValue next = g_gta.onTime[0] + step;
        g_gta.onTime[0] = (next > maxOn) ? minOn : next;
    }
    /* Update V phase */
    {
        Ifx_TimerValue next = g_gta.onTime[1] + step;
        g_gta.onTime[1] = (next > maxOn) ? minOn : next;
    }
    /* Update W phase */
    {
        Ifx_TimerValue next = g_gta.onTime[2] + step;
        g_gta.onTime[2] = (next > maxOn) ? minOn : next;
    }

    /* Apply synchronously via shadow-update */
    IfxGtm_Tom_Timer_disableUpdate(&g_gta.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gta.pwmhl, &g_gta.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gta.timer);
}
