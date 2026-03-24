/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production 3‑phase inverter PWM using GTM TOM + PwmHl (TC3xx)
 */
#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_Tom_Timer.h"

/* ===================== Local driver state ===================== */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                 /* TOM time base */
    IfxGtm_Tom_PwmHl   pwm;                   /* Complementary PWM driver */
    Ifx_TimerValue     pwmOnTimes[NUM_PHASES];/* On-time ticks for U,V,W */
} GtmTom3PhInv_Driver;

static GtmTom3PhInv_Driver s_drv;
static boolean s_initialized = FALSE;

/* ===================== Private functions ===================== */
/** Configure TOM1 output pins using PinMap API (port mode + pad driver). */
static void configureTom1Pins(void)
{
    /* High-side outputs */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* Low-side (complementary) outputs */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/* ===================== Public functions ===================== */
void initGtmTom3phInv(void)
{
    boolean ok;

    /* 1) Enable GTM and clocks (FXCLK) */
    IfxGtm_enable(&MODULE_GTM);
    {
        float32 modFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, modFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Configure TOM1 time base for center-aligned PWM at 20 kHz */
    IfxGtm_Tom_Timer_Config timerCfg;
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
    timerCfg.base.frequency   = PWM_FREQ_HZ;
    timerCfg.clock            = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
    timerCfg.tom              = GTM_TOM_MASTER;
    timerCfg.timerChannel     = GTM_TOM_MASTER_TIMER_CH;

    ok = IfxGtm_Tom_Timer_init(&s_drv.timer, &timerCfg);
    if (ok == FALSE)
    {
        return; /* Early exit on failure */
    }

    /* 3) Configure complementary PWM driver (3 channel pairs) */
    IfxGtm_Tom_PwmHl_Config pwmHlCfg;
    IfxGtm_Tom_ToutMapP ccx[NUM_PHASES] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
    IfxGtm_Tom_ToutMapP coutx[NUM_PHASES] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

    IfxGtm_Tom_PwmHl_initConfig(&pwmHlCfg);
    pwmHlCfg.base.channelCount     = NUM_PHASES;
    pwmHlCfg.base.deadtime         = PWM_DEAD_TIME;         /* 0.5 us */
    pwmHlCfg.base.minPulse         = PWM_MIN_PULSE_TIME;    /* 1.0 us */
    pwmHlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;
    pwmHlCfg.base.outputDriver     = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    pwmHlCfg.base.ccxActiveState   = Ifx_ActiveState_high;
    pwmHlCfg.base.coutxActiveState = Ifx_ActiveState_high;

    pwmHlCfg.ccx   = ccx;
    pwmHlCfg.coutx = coutx;
    pwmHlCfg.timer = &s_drv.timer;
    pwmHlCfg.tom   = timerCfg.tom; /* TOM1 */

    /* Route pins explicitly via PinMap as required by SW design */
    configureTom1Pins();

    ok = IfxGtm_Tom_PwmHl_init(&s_drv.pwm, &pwmHlCfg);
    if (ok == FALSE)
    {
        return; /* Early exit on failure */
    }

    ok = IfxGtm_Tom_PwmHl_setMode(&s_drv.pwm, Ifx_Pwm_Mode_centerAligned);
    if (ok == FALSE)
    {
        return; /* Early exit on failure */
    }

    /* Ensure runtime adjustable constraints are applied (redundant safety) */
    ok = IfxGtm_Tom_PwmHl_setDeadtime(&s_drv.pwm, PWM_DEAD_TIME);
    if (ok == FALSE)
    {
        return;
    }
    ok = IfxGtm_Tom_PwmHl_setMinPulse(&s_drv.pwm, PWM_MIN_PULSE_TIME);
    if (ok == FALSE)
    {
        return;
    }

    /* Sync timer input frequency after configuration */
    IfxGtm_Tom_Timer_updateInputFrequency(&s_drv.timer);

    /* 4) Start the time base */
    IfxGtm_Tom_Timer_run(&s_drv.timer);

    /* 5) Compute initial on-time ticks from current period using requested duties */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
        s_drv.pwmOnTimes[0] = (Ifx_TimerValue)(period * DUTY_25_PERCENT); /* U = 25% */
        s_drv.pwmOnTimes[1] = (Ifx_TimerValue)(period * DUTY_50_PERCENT); /* V = 50% */
        s_drv.pwmOnTimes[2] = (Ifx_TimerValue)(period * DUTY_75_PERCENT); /* W = 75% */
    }

    /* 6) Program on-times using shadow-update sequence for synchronous latching */
    IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.pwmOnTimes);
    IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);

    s_initialized = TRUE;
}

void updateGtmTom3phInvDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit if not initialized per error handling requirements */
    }

    /* Obtain current period and compute step = 10% of period */
    Ifx_TimerValue period    = IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
    Ifx_TimerValue dutyStep  = (Ifx_TimerValue)(period * DUTY_STEP);
    Ifx_TimerValue dutyMax   = period;      /* 100% */
    Ifx_TimerValue dutyMin   = 0;           /* 0% */

    /* Increment and wrap per phase (U,V,W) */
    for (uint8 i = 0u; i < NUM_PHASES; i++)
    {
        s_drv.pwmOnTimes[i] = (Ifx_TimerValue)(s_drv.pwmOnTimes[i] + dutyStep);
        if (s_drv.pwmOnTimes[i] >= dutyMax)
        {
            s_drv.pwmOnTimes[i] = dutyMin; /* wrap to 0% */
        }
    }

    /* Apply synchronously at next PWM period boundary */
    IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.pwmOnTimes);
    IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
}
