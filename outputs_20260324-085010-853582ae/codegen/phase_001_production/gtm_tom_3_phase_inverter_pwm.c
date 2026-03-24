#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_PinMap.h"   /* Generic header per requirements */
#include "IfxPort.h"

/* =============================================================================
 * Configuration values from requirements (highest priority)
 * ============================================================================= */
#define PWM_FREQ_HZ          (20000.0f)  /* TIMING_FREQUENCY_HZ = 20 kHz */
#define PWM_DEAD_TIME        (0.5f)      /* TIMING_DEADTIME_US = 0.5 us */
#define PWM_MIN_PULSE_TIME   (1.0f)      /* TIMING_MIN_PULSE_US = 1.0 us */

#define DUTY_25_PERCENT      (0.25f)     /* INITIAL_DUTY_CYCLES_PHASE_U */
#define DUTY_50_PERCENT      (0.50f)     /* INITIAL_DUTY_CYCLES_PHASE_V */
#define DUTY_75_PERCENT      (0.75f)     /* INITIAL_DUTY_CYCLES_PHASE_W */

#define DUTY_STEP            (0.1f)
#define DUTY_MIN             (0.1f)
#define DUTY_MAX             (0.9f)

/* =============================================================================
 * TOM instance and base channel selection
 * Requirements prefer TOM1 with CH0, but P02.* pins are mapped to TOM0 on TC387.
 * Mapping verification outcome: Use TOM0 and CH0 as time base to match P02.* TOUTs.
 * ============================================================================= */
#define GTM_TOM_MASTER            IfxGtm_Tom_0
#define GTM_TOM_MASTER_TIMER_CH   IfxGtm_Tom_Ch_0

/* =============================================================================
 * Phase pin assignments (KIT_A2G_TC387_5V_TFT, P02 pins from requirements)
 * High-side/Low-side pairs for U, V, W phases.
 * Verified TOM instance selection: TOM0 for P02.* pins.
 * Note: The specific TOM channel indices are resolved by the PinMap macros.
 * ============================================================================= */
#define PHASE_U_HS   &IfxGtm_TOM0_0_TOUT0_P02_0_OUT      /* P02.0 */
#define PHASE_U_LS   &IfxGtm_TOM0_0N_TOUT7_P02_7_OUT     /* P02.7 */
#define PHASE_V_HS   &IfxGtm_TOM0_9_TOUT1_P02_1_OUT      /* P02.1 */
#define PHASE_V_LS   &IfxGtm_TOM0_12_TOUT4_P02_4_OUT     /* P02.4 */
#define PHASE_W_HS   &IfxGtm_TOM0_10_TOUT2_P02_2_OUT     /* P02.2 */
#define PHASE_W_LS   &IfxGtm_TOM0_13_TOUT5_P02_5_OUT     /* P02.5 */

/* =============================================================================
 * Driver/application state
 * ============================================================================= */
typedef struct
{
    IfxGtm_Tom_Timer   timer;               /* TOM timer driver handle */
    IfxGtm_Tom_PwmHl   pwm;                 /* PWMHL (complementary) driver handle */
    Ifx_TimerValue     pwmOnTimes[3];       /* On-times for U, V, W */
    boolean            initialized;         /* Init status flag for runtime guarding */
} GtmTom3PhaseOutput_t;

static GtmTom3PhaseOutput_t g_pwm3PhaseOutput = {0};

/* =============================================================================
 * Public functions (API contract)
 * ============================================================================= */

void initGtmTomPwm(void)
{
    boolean ok;

    /* 1) Enable GTM and functional clocks required by TOM */
    IfxGtm_enable(&MODULE_GTM);
    /* Follow reference pattern: set GCLK and CLK0, then enable FXCLK */
    IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM));
    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* 2) Configure TOM timer: frequency, clock, TOM instance, base channel */
    IfxGtm_Tom_Timer_Config timerConfig;
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    timerConfig.base.frequency = PWM_FREQ_HZ;                          /* 20 kHz */
    timerConfig.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;       /* cmuFxclk0 */
    timerConfig.tom            = GTM_TOM_MASTER;                       /* TOM0 */
    timerConfig.timerChannel   = GTM_TOM_MASTER_TIMER_CH;              /* CH0 */

    ok = IfxGtm_Tom_Timer_init(&g_pwm3PhaseOutput.timer, &timerConfig);
    if (!ok)
    {
        g_pwm3PhaseOutput.initialized = FALSE;
        return; /* Early exit on failure */
    }

    /* 3) Configure output pins via PinMap API with push-pull and pad driver */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) PWMHL configuration: three complementary channel pairs on selected TOM */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlConfig;
        IfxGtm_Tom_ToutMapP ccx[] = {
            PHASE_U_HS,
            PHASE_V_HS,
            PHASE_W_HS
        };
        IfxGtm_Tom_ToutMapP coutx[] = {
            PHASE_U_LS,
            PHASE_V_LS,
            PHASE_W_LS
        };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlConfig);
        pwmHlConfig.base.channelCount     = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
        pwmHlConfig.base.deadtime         = PWM_DEAD_TIME;                          /* 0.5 us */
        pwmHlConfig.base.minPulse         = PWM_MIN_PULSE_TIME;                     /* 1.0 us */
        pwmHlConfig.base.outputMode       = IfxPort_OutputMode_pushPull;
        pwmHlConfig.base.outputDriver     = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlConfig.base.ccxActiveState   = Ifx_ActiveState_high;                   /* Active high */
        pwmHlConfig.base.coutxActiveState = Ifx_ActiveState_high;                   /* Active high */
        pwmHlConfig.ccx                   = ccx;
        pwmHlConfig.coutx                 = coutx;
        pwmHlConfig.timer                 = &g_pwm3PhaseOutput.timer;
        pwmHlConfig.tom                   = timerConfig.tom;                         /* TOM0 */

        ok = IfxGtm_Tom_PwmHl_init(&g_pwm3PhaseOutput.pwm, &pwmHlConfig);
        if (!ok)
        {
            g_pwm3PhaseOutput.initialized = FALSE;
            return; /* Early exit on failure */
        }

        /* Center-aligned PWM mode per reference */
        (void)IfxGtm_Tom_PwmHl_setMode(&g_pwm3PhaseOutput.pwm, Ifx_Pwm_Mode_centerAligned);

        /* Refresh frequency linkage */
        IfxGtm_Tom_Timer_updateInputFrequency(&g_pwm3PhaseOutput.timer);
    }

    /* 5) Start the timer */
    IfxGtm_Tom_Timer_run(&g_pwm3PhaseOutput.timer);

    /* 6) Initialize on-times from period and requested initial duty fractions */
    {
        Ifx_TimerValue period = g_pwm3PhaseOutput.pwm.timer->base.period;
        g_pwm3PhaseOutput.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * DUTY_25_PERCENT);
        g_pwm3PhaseOutput.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * DUTY_50_PERCENT);
        g_pwm3PhaseOutput.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * DUTY_75_PERCENT);

        IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
    }

    g_pwm3PhaseOutput.initialized = TRUE;
}

void IfxGtm_Tom_PwmHl_setOnTime(void)
{
    if (!g_pwm3PhaseOutput.initialized)
    {
        return; /* Early-exit if init failed or not yet called */
    }

    /* Algorithm per SW Detailed Design (ramp with wrap-around) */
    {
        Ifx_TimerValue period      = g_pwm3PhaseOutput.pwm.timer->base.period;
        Ifx_TimerValue dutyStep    = (Ifx_TimerValue)((float32)period * DUTY_STEP);
        Ifx_TimerValue dutyStepMin = (Ifx_TimerValue)((float32)period * DUTY_MIN);
        Ifx_TimerValue dutyStepMax = (Ifx_TimerValue)((float32)period * DUTY_MAX);

        /* Increment on-times */
        g_pwm3PhaseOutput.pwmOnTimes[0] += dutyStep;
        g_pwm3PhaseOutput.pwmOnTimes[1] += dutyStep;
        g_pwm3PhaseOutput.pwmOnTimes[2] += dutyStep;

        /* Wrap-around at maximum boundary */
        if (g_pwm3PhaseOutput.pwmOnTimes[0] >= dutyStepMax)
        {
            g_pwm3PhaseOutput.pwmOnTimes[0] = dutyStepMin;
        }
        if (g_pwm3PhaseOutput.pwmOnTimes[1] >= dutyStepMax)
        {
            g_pwm3PhaseOutput.pwmOnTimes[1] = dutyStepMin;
        }
        if (g_pwm3PhaseOutput.pwmOnTimes[2] >= dutyStepMax)
        {
            g_pwm3PhaseOutput.pwmOnTimes[2] = dutyStepMin;
        }

        /* Coherent update: disable, write, apply */
        IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
    }
}
