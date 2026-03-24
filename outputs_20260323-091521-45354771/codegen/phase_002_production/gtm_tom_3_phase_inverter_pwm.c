/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief GTM TOM 3-Phase Inverter PWM driver (TC387)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD includes - use generic headers only */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* --------------------------- Pin mapping (requirements) --------------------------- */
/* Keep TOM1, pins: U: P00.3/2, V: P00.5/4, W: P00.7/6 */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

#define NUM_PHASES   (3U)

/* --------------------------- Driver state --------------------------- */
typedef struct
{
    IfxGtm_Tom_Timer  timer;                  /* TOM time-base driver */
    IfxGtm_Tom_PwmHl  pwm;                    /* Complementary PWM driver */
    Ifx_TimerValue    pwmOnTimes[NUM_PHASES]; /* On-time ticks for U,V,W */
} GtmTom3PhaseOutput;

static GtmTom3PhaseOutput g_pwm3PhaseOutput;
static boolean            s_initialized = FALSE; /* Set TRUE only if all init steps succeed */

/* --------------------------- Private helpers --------------------------- */
/**
 * @brief Configure TOM1 output pins (sets port mode/pad driver via PinMap API).
 * Note: Low-level PwmHl flow requires explicit pin routing configuration.
 */
static void configureTom1Pins(void)
{
    /* Configure HS pins */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* Configure LS pins */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/* --------------------------- Public API --------------------------- */
/**
 * @brief Initialize GTM TOM complementary PWM for 3-phase inverter.
 *
 * Behavior per SW Detailed Design:
 *  - Enable GTM and clocks
 *  - Initialize TOM timer base for center-aligned PWM at required frequency
 *  - Configure complementary PWM with 3 channel pairs (U,V,W) and required pins
 *  - Apply deadtime and min pulse constraints
 *  - Start timer
 *  - Compute initial on-times from period using 25%, 50%, 75%
 *  - Program on-times using shadow-update sequence (disable, write, apply)
 */
void initGtmTom3phInv(void)
{
    boolean ok;

    /* 1) GTM module and CMU clock setup */
    IfxGtm_enable(&MODULE_GTM);
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        {
            float32 gclk = IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, gclk);
        }
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Configure TOM timer time base */
    IfxGtm_Tom_Timer_Config timerConfig;
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    timerConfig.base.frequency = TIMING_PWM_FREQUENCY_HZ;      /* 20 kHz */
    timerConfig.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
    timerConfig.tom            = IfxGtm_Tom_1;                 /* Keep TOM1 */
    timerConfig.timerChannel   = IfxGtm_Tom_Ch_0;              /* Master time base ch */

    ok = IfxGtm_Tom_Timer_init(&g_pwm3PhaseOutput.timer, &timerConfig);
    if (ok == FALSE)
    {
        return; /* Early exit on failure */
    }

    /* 3) Configure pins (routing + pad/drive) */
    configureTom1Pins();

    /* 4) Configure complementary PWM HL (3 channel pairs) */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlConfig;
        IfxGtm_Tom_ToutMapP ccx[] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
        IfxGtm_Tom_ToutMapP coutx[] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlConfig);
        pwmHlConfig.base.channelCount     = (uint32)(sizeof(ccx) / sizeof(ccx[0]));
        pwmHlConfig.base.deadtime         = PWM_DEAD_TIME;                         /* us */
        pwmHlConfig.base.minPulse         = PWM_MIN_PULSE_TIME;                    /* us */
        pwmHlConfig.base.outputMode       = IfxPort_OutputMode_pushPull;
        pwmHlConfig.base.outputDriver     = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlConfig.base.ccxActiveState   = Ifx_ActiveState_high;
        pwmHlConfig.base.coutxActiveState = Ifx_ActiveState_high;
        pwmHlConfig.ccx                   = ccx;
        pwmHlConfig.coutx                 = coutx;
        pwmHlConfig.timer                 = &g_pwm3PhaseOutput.timer;
        pwmHlConfig.tom                   = timerConfig.tom;

        ok = IfxGtm_Tom_PwmHl_init(&g_pwm3PhaseOutput.pwm, &pwmHlConfig);
        if (ok == FALSE)
        {
            return; /* Early exit on failure */
        }

        /* Center-aligned PWM mode */
        ok = IfxGtm_Tom_PwmHl_setMode(&g_pwm3PhaseOutput.pwm, Ifx_Pwm_Mode_centerAligned);
        if (ok == FALSE)
        {
            return;
        }

        /* Explicitly apply deadtime and min-pulse constraints (as per driver_calls) */
        ok = IfxGtm_Tom_PwmHl_setDeadtime(&g_pwm3PhaseOutput.pwm, PWM_DEAD_TIME);
        if (ok == FALSE)
        {
            return;
        }
        ok = IfxGtm_Tom_PwmHl_setMinPulse(&g_pwm3PhaseOutput.pwm, PWM_MIN_PULSE_TIME);
        if (ok == FALSE)
        {
            return;
        }
    }

    /* 5) Start the timer base */
    IfxGtm_Tom_Timer_run(&g_pwm3PhaseOutput.timer);

    /* 6) Compute initial on-times (25%, 50%, 75%) and program using shadow-update */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_pwm3PhaseOutput.timer);
        g_pwm3PhaseOutput.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * DUTY_25_PERCENT);
        g_pwm3PhaseOutput.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * DUTY_50_PERCENT);
        g_pwm3PhaseOutput.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * DUTY_75_PERCENT);

        IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
    }

    s_initialized = TRUE; /* All steps succeeded */
}

/**
 * @brief Update the three phase duties by +10% with wrap in 0..100% and apply synchronously.
 *
 * Behavior per SW Detailed Design:
 *  - Read current period
 *  - Compute step = 10% of period
 *  - Add step to each on-time; if any on-time >= period, wrap to 0
 *  - Perform shadow-update sequence to latch changes at the next period boundary
 *
 * Note: Caller is responsible for calling this function every 500 ms.
 */
void updateGtmTom3phInvDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit if init failed or not called */
    }

    Ifx_TimerValue period   = IfxGtm_Tom_Timer_getPeriod(&g_pwm3PhaseOutput.timer);
    Ifx_TimerValue dutyStep = (Ifx_TimerValue)((float32)period * DUTY_STEP);

    /* Increment and wrap per phase */
    for (uint32 i = 0U; i < NUM_PHASES; i++)
    {
        g_pwm3PhaseOutput.pwmOnTimes[i] += dutyStep;
        if (g_pwm3PhaseOutput.pwmOnTimes[i] >= period)
        {
            g_pwm3PhaseOutput.pwmOnTimes[i] = 0U; /* Wrap to 0% */
        }
    }

    /* Shadow-update to apply synchronously at period boundary */
    IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
    IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
}
