/**********************************************************************************************************************
 * Module: GTM_TOM_3_Phase_Inverter_PWM
 * File:   gtm_tom_3_phase_inverter_pwm.c
 * Target: TC3xx (e.g., TC387)
 * Desc:   3-phase complementary center-aligned PWM using GTM TOM + PWMHL (production, iLLD-based)
 *********************************************************************************************************************/
#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"
/* Include PwmHl header before introducing any macro remapping for the update wrapper */
#include "IfxGtm_Tom_PwmHl.h"

#include "gtm_tom_3_phase_inverter_pwm.h"
/* Undo the public header macro remap so we can call the real iLLD API name inside this TU */
#undef IfxGtm_Tom_PwmHl_setOnTime

/*==================================================================================================
 * Local types and globals
 *================================================================================================*/

typedef struct
{
    IfxGtm_Tom_Timer   timer;                       /* TOM timer driver */
    IfxGtm_Tom_PwmHl   pwm;                         /* PWMHL driver */
    Ifx_TimerValue     pwmOnTimes[NUM_OF_CHANNELS]; /* ON times in timer ticks */
    boolean            initialized;                 /* init status flag */
} GtmTom3PhasePwm_Driver;

static GtmTom3PhasePwm_Driver g_pwm3PhaseOutput = { 0 };  /* Zero-init all fields */

/*==================================================================================================
 * Local helpers (constants)
 *================================================================================================*/
#define SEC_PER_US                 (1.0e-6f)
#define PWM_DEAD_TIME              (PWM_DEAD_TIME_US * SEC_PER_US)
#define PWM_MIN_PULSE_TIME         (PWM_MIN_PULSE_TIME_US * SEC_PER_US)

/*==================================================================================================
 * Doxygen: Initialize the GTM PWM subsystem to generate three complementary, center-aligned PWM pairs
 *================================================================================================*/
void initGtmTomPwm(void)
{
    boolean ok;

    /* 1) Enable GTM + clocks for TOM (FXCLK) */
    IfxGtm_enable(&MODULE_GTM);
    /* Set GCLK to module frequency, feed CLK0 from GCLK, enable FXCLK domain */
    IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM));
    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* 2) Configure TOM timer: frequency, instance TOM0, base channel CH0, FXCLK0 */
    IfxGtm_Tom_Timer_Config timerConfig;
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    timerConfig.base.frequency = PWM_FREQ_HZ;
    timerConfig.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
    timerConfig.tom            = GTM_TOM_MASTER;
    timerConfig.timerChannel   = GTM_TOM_MASTER_TIMER_CH;

    ok = IfxGtm_Tom_Timer_init(&g_pwm3PhaseOutput.timer, &timerConfig);
    if (ok == FALSE)
    {
        g_pwm3PhaseOutput.initialized = FALSE;
        return; /* Early exit on failure */
    }

    /* 3) Configure the six output pins via PinMap (push-pull, specified pad driver) */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, PWM_OUTPUT_MODE, PWM_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, PWM_OUTPUT_MODE, PWM_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, PWM_OUTPUT_MODE, PWM_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, PWM_OUTPUT_MODE, PWM_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, PWM_OUTPUT_MODE, PWM_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, PWM_OUTPUT_MODE, PWM_PAD_DRIVER);

    /* 4) Configure PWMHL (3 complementary pairs, dead-time, min pulse, output mode/driver) */
    IfxGtm_Tom_PwmHl_Config pwmHlConfig;
    IfxGtm_Tom_ToutMapP ccx[] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
    IfxGtm_Tom_ToutMapP coutx[] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

    IfxGtm_Tom_PwmHl_initConfig(&pwmHlConfig);
    pwmHlConfig.base.channelCount     = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
    pwmHlConfig.base.deadtime         = PWM_DEAD_TIME;
    pwmHlConfig.base.minPulse         = PWM_MIN_PULSE_TIME;
    pwmHlConfig.base.outputMode       = PWM_OUTPUT_MODE;
    pwmHlConfig.base.outputDriver     = PWM_PAD_DRIVER;
    pwmHlConfig.base.ccxActiveState   = Ifx_ActiveState_high;
    pwmHlConfig.base.coutxActiveState = Ifx_ActiveState_high;

    pwmHlConfig.ccx   = ccx;
    pwmHlConfig.coutx = coutx;
    pwmHlConfig.timer = &g_pwm3PhaseOutput.timer;
    pwmHlConfig.tom   = timerConfig.tom;

    ok = IfxGtm_Tom_PwmHl_init(&g_pwm3PhaseOutput.pwm, &pwmHlConfig);
    if (ok == FALSE)
    {
        g_pwm3PhaseOutput.initialized = FALSE;
        return; /* Early exit on failure */
    }

    /* Explicitly apply dead-time and minPulse post-init per design's driver_calls */
    (void)IfxGtm_Tom_PwmHl_setDeadtime(&g_pwm3PhaseOutput.pwm, PWM_DEAD_TIME);
    (void)IfxGtm_Tom_PwmHl_setMinPulse(&g_pwm3PhaseOutput.pwm, PWM_MIN_PULSE_TIME);

    /* 5) Center-aligned PWM mode */
    (void)IfxGtm_Tom_PwmHl_setMode(&g_pwm3PhaseOutput.pwm, Ifx_Pwm_Mode_centerAligned);

    /* Ensure internal timing is up to date */
    IfxGtm_Tom_Timer_updateInputFrequency(&g_pwm3PhaseOutput.timer);

    /* Start the timer base */
    IfxGtm_Tom_Timer_run(&g_pwm3PhaseOutput.timer);

    /* 6) Compute initial ON-times (in ticks) and apply coherently */
    {
        Ifx_TimerValue period = g_pwm3PhaseOutput.pwm.timer->base.period;
        g_pwm3PhaseOutput.pwmOnTimes[0] = (Ifx_TimerValue)(period * DUTY_25_PERCENT);
        g_pwm3PhaseOutput.pwmOnTimes[1] = (Ifx_TimerValue)(period * DUTY_50_PERCENT);
        g_pwm3PhaseOutput.pwmOnTimes[2] = (Ifx_TimerValue)(period * DUTY_75_PERCENT);

        IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
    }

    g_pwm3PhaseOutput.initialized = TRUE;
}

/*==================================================================================================
 * Doxygen: Periodic duty-cycle ramp with wrap-around and coherent update (runtime)
 *
 * Name preserved by API contract via macro in the public header. The actual symbol defined here is
 * UpdateGtmTomPwmDutyRamp; external callers use IfxGtm_Tom_PwmHl_setOnTime() as declared in the header.
 *================================================================================================*/
void UpdateGtmTomPwmDutyRamp(void)
{
    if (g_pwm3PhaseOutput.initialized == FALSE)
    {
        return; /* Early-exit if init failed or not called yet */
    }

    /* 1) Read current period */
    const Ifx_TimerValue period = g_pwm3PhaseOutput.pwm.timer->base.period;

    /* 2) Compute duty step and min/max boundaries in ticks */
    const Ifx_TimerValue dutyStep    = (Ifx_TimerValue)(period * DUTY_STEP);
    const Ifx_TimerValue dutyStepMin = (Ifx_TimerValue)(period * DUTY_MIN);
    const Ifx_TimerValue dutyStepMax = (Ifx_TimerValue)(period * DUTY_MAX);

    /* 3) Increment each channel's ON-time with wrap-around at max -> min */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_pwm3PhaseOutput.pwmOnTimes[i] = (Ifx_TimerValue)(g_pwm3PhaseOutput.pwmOnTimes[i] + dutyStep);
        if (g_pwm3PhaseOutput.pwmOnTimes[i] >= dutyStepMax)
        {
            g_pwm3PhaseOutput.pwmOnTimes[i] = dutyStepMin;
        }
    }

    /* 4) Apply ON-times coherently at the next update point */
    IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
    IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
}
