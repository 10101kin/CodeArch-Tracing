/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM three-phase complementary PWM on TC3xx.
 *
 * Notes:
 * - Uses IfxGtm_Tom_Timer as time base (TOM1 CH0, FXCLK0), 20 kHz target.
 * - Uses IfxGtm_Tom_PwmHl for 3 complementary pairs with dead-time and min-pulse.
 * - All updates use disableUpdate -> setOnTime -> applyUpdate shadow sequence.
 * - No watchdog handling here (must be in Cpu0_Main.c only).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ========================= Configuration Macros (from requirements) ========================= */
#define PWM_CHANNELS_COUNT                 (3u)
#define PWM_FREQUENCY_HZ                   (20000.0f)          /* 20 kHz */
#define PWM_DEADTIME_US                    (0.5f)              /* microseconds */
#define PWM_MIN_PULSE_US                   (1.0f)              /* microseconds */
#define FXCLK0_ASSUMED_MHZ                 (100.0f)            /* not hard-coded into CMU; for tick math only */
#define GTM_TICK_NS                        (10.0f)             /* derived from 100 MHz */
#define PWM_DEADTIME_TICKS                 (50u)               /* 0.5 us @ 10 ns/tick */
#define PWM_MIN_PULSE_TICKS                (100u)              /* 1.0 us @ 10 ns/tick */
#define INITIAL_DUTY_U_PERCENT             (25.0f)
#define INITIAL_DUTY_V_PERCENT             (50.0f)
#define INITIAL_DUTY_W_PERCENT             (75.0f)
#define PWM_DUTY_STEP_FRAC                 (0.10f)             /* 10% of current period per update */

/* Interrupt priority for debug ISR (naming kept as *_ATOM per convention rule) */
#define ISR_PRIORITY_ATOM                  (20)

/* Debug LED on port P13.0 */
#define LED                                &MODULE_P13, 0

/* ========================= Validated GTM TOM1 Pin Macros (from reference pattern) ========================= */
#define PHASE_U_HS   &IfxGtm_TOM1_2_TOUT12_P00_3_OUT   /* TOM1 CH2 -> P00.3 */
#define PHASE_U_LS   &IfxGtm_TOM1_1_TOUT11_P00_2_OUT   /* TOM1 CH1 -> P00.2 */
#define PHASE_V_HS   &IfxGtm_TOM1_4_TOUT14_P00_5_OUT   /* TOM1 CH4 -> P00.5 */
#define PHASE_V_LS   &IfxGtm_TOM1_3_TOUT13_P00_4_OUT   /* TOM1 CH3 -> P00.4 */
#define PHASE_W_HS   &IfxGtm_TOM1_6_TOUT16_P00_7_OUT   /* TOM1 CH6 -> P00.7 */
#define PHASE_W_LS   &IfxGtm_TOM1_5_TOUT15_P00_6_OUT   /* TOM1 CH5 -> P00.6 */

/* ========================= Module State ========================= */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                        /* Persistent TOM timer handle */
    IfxGtm_Tom_PwmHl   pwmhl;                        /* Persistent PwmHl handle */
    Ifx_TimerValue     tOn[PWM_CHANNELS_COUNT];      /* Current on-time ticks per phase (U,V,W) */
} GtmTom3Ph_State;

IFX_STATIC GtmTom3Ph_State g_gttom3ph = { 0 };

/* ========================= ISR and Period Callback ========================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    /* Minimal ISR: toggle debug LED */
    IfxPort_togglePin(LED);
}

/* Period-event callback - empty by design */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Internal Helpers ========================= */
static inline Ifx_TimerValue percentToTicks(float32 percent, Ifx_TimerValue period)
{
    if (percent <= 0.0f) { return 0u; }
    if (percent >= 100.0f) { return period; }
    float32 ticks = ((percent * (float32)period) * 0.01f);
    if (ticks < 0.0f) { ticks = 0.0f; }
    return (Ifx_TimerValue)(ticks + 0.5f);
}

/* ========================= Public API Implementations ========================= */
void initGtmTomPwm(void)
{
    /* 1) Enable GTM and clocks (FXCLK and CLK0 as per reference pattern) */
    {
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_enable(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Configure TOM timer (TOM1, CH0 as time base, FXCLK0, 20 kHz) */
    {
        IfxGtm_Tom_Timer_Config tcfg;
        IfxGtm_Tom_Timer_initConfig(&tcfg, &MODULE_GTM);
        tcfg.tom           = IfxGtm_Tom_1;                    /* TOM1 instance */
        tcfg.timerChannel  = IfxGtm_Tom_Ch_0;                 /* base channel CH0 */
        tcfg.clock         = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;  /* FXCLK0 */
        tcfg.base.frequency = PWM_FREQUENCY_HZ;               /* 20 kHz */

        if (FALSE == IfxGtm_Tom_Timer_init(&g_gttom3ph.timer, &tcfg))
        {
            return; /* Initialization failed */
        }
    }

    /* 3) Prepare complementary PWM pins (high-side ccx[], low-side cCx[]) */
    const IfxGtm_Tom_ToutMap *ccx[PWM_CHANNELS_COUNT] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
    const IfxGtm_Tom_ToutMap *cCx[PWM_CHANNELS_COUNT] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

    /* 4) Configure PwmHl for 3 complementary channels, center-aligned */
    {
        IfxGtm_Tom_PwmHl_Config pcfg;
        IfxGtm_Tom_PwmHl_initConfig(&pcfg);
        pcfg.tom                   = IfxGtm_Tom_1;                    /* TOM1 */
        pcfg.timer                 = &g_gttom3ph.timer;               /* link timer */
        pcfg.base.channelCount     = PWM_CHANNELS_COUNT;              /* 3 phases */
        pcfg.base.deadtime         = (PWM_DEADTIME_US * 1.0e-6f);     /* seconds */
        pcfg.base.minPulse         = (PWM_MIN_PULSE_US * 1.0e-6f);    /* seconds */
        pcfg.base.outputMode       = IfxPort_OutputMode_pushPull;
        pcfg.base.padDriver        = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        /* Active-high for both high- and low-side outputs (per requirement) */
        /* IfxStdIf_PwmHl polarity fields are implementation-defined; default is active-high. */
        pcfg.pins.ccx              = ccx;                             /* HS */
        pcfg.pins.cCx              = cCx;                             /* LS */

        if (FALSE == IfxGtm_Tom_PwmHl_init(&g_gttom3ph.pwmhl, &pcfg))
        {
            return; /* Initialization failed */
        }

        if (FALSE == IfxGtm_Tom_PwmHl_setMode(&g_gttom3ph.pwmhl, Ifx_Pwm_Mode_centerAligned))
        {
            return; /* Mode setup failed */
        }
    }

    /* 5) Start the TOM timer */
    IfxGtm_Tom_Timer_run(&g_gttom3ph.timer);

    /* 6) Initialize on-times from initial duty ratios and apply via shadow update */
    {
        Ifx_TimerValue period = g_gttom3ph.timer.base.period; /* current period in ticks */
        g_gttom3ph.tOn[0] = percentToTicks(INITIAL_DUTY_U_PERCENT, period);
        g_gttom3ph.tOn[1] = percentToTicks(INITIAL_DUTY_V_PERCENT, period);
        g_gttom3ph.tOn[2] = percentToTicks(INITIAL_DUTY_W_PERCENT, period);

        IfxGtm_Tom_Timer_disableUpdate(&g_gttom3ph.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_gttom3ph.pwmhl, &g_gttom3ph.tOn[0]);
        IfxGtm_Tom_Timer_applyUpdate(&g_gttom3ph.timer);
    }

    /* 7) Configure debug LED pin after PWM init (push-pull, general output) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateGtmTomPwmDutyCycles(void)
{
    /* Read current period from timer handle */
    Ifx_TimerValue period   = g_gttom3ph.timer.base.period;
    Ifx_TimerValue minTicks = PWM_MIN_PULSE_TICKS;
    Ifx_TimerValue maxTicks = (period > minTicks) ? (period - minTicks) : minTicks;

    /* Fixed increment step as a fraction of period */
    Ifx_TimerValue step = (Ifx_TimerValue)((float32)period * PWM_DUTY_STEP_FRAC + 0.5f);
    if (step == 0u)
    {
        step = 1u; /* ensure progress even with very small periods */
    }

    /* Update on-times with wrap between [minTicks, maxTicks] */
    {
        Ifx_TimerValue t0 = g_gttom3ph.tOn[0];
        Ifx_TimerValue t1 = g_gttom3ph.tOn[1];
        Ifx_TimerValue t2 = g_gttom3ph.tOn[2];

        t0 = (t0 + step > maxTicks) ? minTicks : (t0 + step);
        t1 = (t1 + step > maxTicks) ? minTicks : (t1 + step);
        t2 = (t2 + step > maxTicks) ? minTicks : (t2 + step);

        g_gttom3ph.tOn[0] = t0;
        g_gttom3ph.tOn[1] = t1;
        g_gttom3ph.tOn[2] = t2;
    }

    /* Apply synchronously at next period event */
    IfxGtm_Tom_Timer_disableUpdate(&g_gttom3ph.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gttom3ph.pwmhl, &g_gttom3ph.tOn[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gttom3ph.timer);
}
