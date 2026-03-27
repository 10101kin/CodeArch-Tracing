/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM1 3-Phase complementary PWM (KIT_A2G_TC387_5V_TFT)
 *
 * Behavior:
 *  - Initializes GTM TOM1 for 3 complementary phase pairs using TOM timer as base and PwmHl for paired outputs
 *  - 20 kHz, center-aligned, FXCLK0 clock, 0.5 us dead-time, 1.0 us minPulse
 *  - Initial duties: U=25%%, V=50%%, W=75%%
 *  - Synchronous shadow transfer on updates via TOM TGC
 *
 * Notes:
 *  - No watchdog functions here (Cpu0_Main.c only)
 *  - Uses only iLLD APIs listed in available signatures
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu.h"

/* ====================================================================== */
/* Macros and configuration constants (from requirements)                  */
/* ====================================================================== */
#define NUM_PHASES                        (3U)

/* Timing requirements */
#define PWM_BASE_FREQUENCY_HZ            (20000.0f)     /* 20 kHz */
#define PWM_DEAD_TIME_US                 (0.5f)         /* 0.5 us */
#define PWM_MIN_PULSE_US                 (1.0f)         /* 1.0 us */

/* Computed ticks from requirements (not forced in HW; used as references) */
#define PWM_PERIOD_TICKS_REF             (5000U)
#define PWM_DEADTIME_TICKS_REF           (50U)
#define PWM_MINPULSE_TICKS_REF           (100U)

/* Initial duty (percent) */
#define PHASE_U_INIT_DUTY_PERCENT        (25.0f)
#define PHASE_V_INIT_DUTY_PERCENT        (50.0f)
#define PHASE_W_INIT_DUTY_PERCENT        (75.0f)

/* On-time update step: fraction of current period (1/100) */
#define ONTIME_STEP_DIVISOR              (100U)

/* LED (debug) on P13.0 */
#define LED                              &MODULE_P13, 0

/* ISR priority macro */
#define ISR_PRIORITY_ATOM                (20)

/* Pin routing: TOM1 channels to P00.[2..7] (validated/reference symbols) */
#define PHASE_U_LS                       (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT) /* CH1 -> P00.2 (low-side) */
#define PHASE_U_HS                       (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT) /* CH2 -> P00.3 (high-side) */
#define PHASE_V_LS                       (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT) /* CH3 -> P00.4 (low-side) */
#define PHASE_V_HS                       (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT) /* CH4 -> P00.5 (high-side) */
#define PHASE_W_LS                       (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT) /* CH5 -> P00.6 (low-side) */
#define PHASE_W_HS                       (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT) /* CH6 -> P00.7 (high-side) */

/* ====================================================================== */
/* Internal state                                                          */
/* ====================================================================== */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                 /* TOM timer driver (base)        */
    IfxGtm_Tom_PwmHl   pwmhl;                 /* High/Low paired PWM driver     */
    Ifx_TimerValue     onTime[NUM_PHASES];    /* Current on-time per phase [ticks] */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;

/* ====================================================================== */
/* ISR and callback (declared BEFORE init per structural rules)            */
/* ====================================================================== */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    /* Minimal ISR: toggle debug LED */
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Empty body by design */
}

/* ====================================================================== */
/* Helper: convert percent to on-time ticks based on current period        */
/* ====================================================================== */
static inline Ifx_TimerValue pctToTicks(float32 percent, Ifx_TimerValue period)
{
    float32 ticks = (percent * (float32)period) / 100.0f;
    if (ticks < 0.0f)
    {
        ticks = 0.0f;
    }
    return (Ifx_TimerValue)(ticks + 0.5f);
}

/* ====================================================================== */
/* Public API                                                              */
/* ====================================================================== */
void initGtmTom3phInv(void)
{
    /* 1) Enable GTM and FXCLK/CLK0 clocks (guarded) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        float32 moduleFreq = 0.0f;
        IfxGtm_enable(&MODULE_GTM);
        moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFreq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Initialize TOM timer configuration */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
        timerCfg.tom          = IfxGtm_Tom_1;                             /* TOM1 cluster */
        timerCfg.timerChannel = IfxGtm_Tom_Ch_0;                          /* Base counter channel */
        timerCfg.clock        = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;           /* FXCLK0 */
        timerCfg.frequency    = PWM_BASE_FREQUENCY_HZ;                    /* 20 kHz */
        (void)IfxGtm_Tom_Timer_init(&g_gtmTom3phInv.timer, &timerCfg);    /* check boolean in production systems */
    }

    /* 3) Route six TOM outputs to pads (push-pull, active-high outputs) */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) Initialize TOM PwmHl configuration (3 complementary pairs) */
    {
        IfxGtm_Tom_PwmHl_Config pwmhlCfg;
        IfxGtm_Tom_PwmHl_initConfig(&pwmhlCfg);
        pwmhlCfg.timer                 = &g_gtmTom3phInv.timer;            /* bind to TOM timer */
        pwmhlCfg.base.channelCount     = (uint8)NUM_PHASES;                /* U, V, W */
        pwmhlCfg.base.deadtime         = PWM_DEAD_TIME_US * 1e-6f;         /* seconds */
        pwmhlCfg.base.minPulse         = PWM_MIN_PULSE_US * 1e-6f;         /* seconds */
        pwmhlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;      /* push-pull */
        pwmhlCfg.base.activeStateHigh  = Ifx_ActiveState_high;             /* high-side active HIGH */
        pwmhlCfg.base.activeStateLow   = Ifx_ActiveState_high;             /* low-side active HIGH (per design) */
        /* Map complementary outputs for each pair */
        pwmhlCfg.ccx[0]  = (IfxGtm_Tom_ToutMap *)PHASE_U_HS;  /* U high */
        pwmhlCfg.ccxN[0] = (IfxGtm_Tom_ToutMap *)PHASE_U_LS;  /* U low  */
        pwmhlCfg.ccx[1]  = (IfxGtm_Tom_ToutMap *)PHASE_V_HS;  /* V high */
        pwmhlCfg.ccxN[1] = (IfxGtm_Tom_ToutMap *)PHASE_V_LS;  /* V low  */
        pwmhlCfg.ccx[2]  = (IfxGtm_Tom_ToutMap *)PHASE_W_HS;  /* W high */
        pwmhlCfg.ccxN[2] = (IfxGtm_Tom_ToutMap *)PHASE_W_LS;  /* W low  */
        (void)IfxGtm_Tom_PwmHl_init(&g_gtmTom3phInv.pwmhl, &pwmhlCfg);     /* check boolean in production systems */
        (void)IfxGtm_Tom_PwmHl_setMode(&g_gtmTom3phInv.pwmhl, Ifx_Pwm_Mode_centerAligned);
    }

    /* 5) Update timer input frequency from CMU and start timer */
    IfxGtm_Tom_Timer_updateInputFrequency(&g_gtmTom3phInv.timer);
    IfxGtm_Tom_Timer_run(&g_gtmTom3phInv.timer);

    /* 6) Compute initial on-times from current period (25%, 50%, 75%) */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_gtmTom3phInv.timer);
        g_gtmTom3phInv.onTime[0] = pctToTicks(PHASE_U_INIT_DUTY_PERCENT, period);
        g_gtmTom3phInv.onTime[1] = pctToTicks(PHASE_V_INIT_DUTY_PERCENT, period);
        g_gtmTom3phInv.onTime[2] = pctToTicks(PHASE_W_INIT_DUTY_PERCENT, period);
    }

    /* 7) Synchronous shadow transfer: disable update -> set on-times -> apply update */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phInv.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3phInv.pwmhl, &g_gtmTom3phInv.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phInv.timer);

    /* Debug LED configured after PWM init (push-pull, same pad driver) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

void updateGtmTom3phInvDuty(void)
{
    /* 1) Read current period and compute step/min/max thresholds */
    const Ifx_TimerValue period    = IfxGtm_Tom_Timer_getPeriod(&g_gtmTom3phInv.timer);
    const float32        resSec    = IfxGtm_Tom_Timer_getResolution(&g_gtmTom3phInv.timer); /* s/tick */
    const Ifx_TimerValue minTicks  = (Ifx_TimerValue)((PWM_MIN_PULSE_US * 1e-6f / resSec) + 0.5f);
    const Ifx_TimerValue maxTicks  = (period > minTicks) ? (period - minTicks) : 0U;
    const Ifx_TimerValue step      = (Ifx_TimerValue)((period / (Ifx_TimerValue)ONTIME_STEP_DIVISOR) > 0U ? (period / (Ifx_TimerValue)ONTIME_STEP_DIVISOR) : 1U);

    /* 2) Cyclic ramp: add step; wrap to minTicks if above maxTicks */
    {
        Ifx_TimerValue nextU = g_gtmTom3phInv.onTime[0] + step;
        Ifx_TimerValue nextV = g_gtmTom3phInv.onTime[1] + step;
        Ifx_TimerValue nextW = g_gtmTom3phInv.onTime[2] + step;
        if (nextU > maxTicks) { nextU = minTicks; }
        if (nextV > maxTicks) { nextV = minTicks; }
        if (nextW > maxTicks) { nextW = minTicks; }
        g_gtmTom3phInv.onTime[0] = nextU;
        g_gtmTom3phInv.onTime[1] = nextV;
        g_gtmTom3phInv.onTime[2] = nextW;
    }

    /* 3) Synchronous update via TGC shadow-transfer */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phInv.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3phInv.pwmhl, &g_gtmTom3phInv.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phInv.timer);
}
