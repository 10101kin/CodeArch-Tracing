/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief GTM TOM 3-Phase Inverter PWM driver implementation (TC3xx / TC387)
 *
 * Implements the exact API contract and behavior specified in SW Detailed Design.
 * Uses real iLLD drivers: IfxGtm_Tom_Timer and IfxGtm_Tom_PwmHl.
 *
 * Critical compliance:
 * - Generic PinMap header (IfxGtm_PinMap.h), not family-specific
 * - No watchdog handling here (handled in CpuN_Main.c)
 * - Error handling: check boolean returns, early exit on failure
 * - No direct SFR access; use iLLD APIs only
 */

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/*
 * Name collision workaround:
 * The iLLD exports IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl*, Ifx_TimerValue*).
 * SW Detailed Design also mandates a module-level function named IfxGtm_Tom_PwmHl_setOnTime(void).
 * To avoid a symbol clash in C, we temporarily alias the iLLD symbol during inclusion.
 */
#define IfxGtm_Tom_PwmHl_setOnTime IfxGtm_Tom_PwmHl_setOnTime_Driver
#include "IfxGtm_Tom_PwmHl.h"
#undef IfxGtm_Tom_PwmHl_setOnTime

#include "gtm_tom_3_phase_inverter_pwm.h"

/* ========================= Requirements-derived configuration ========================= */
#define NUM_OF_CHANNELS                (3u)

/* Timing requirements */
#define PWM_FREQ_HZ                    (20000.0f)   /* 20 kHz */
#define PWM_DEAD_TIME                  (0.5f)       /* 0.5 us (unit as used by iLLD PwmHl config) */
#define PWM_MIN_PULSE_TIME             (1.0f)       /* 1.0 us */

/* Duty macros (fractions), per reference patterns */
#define DUTY_25_PERCENT                (0.25f)
#define DUTY_50_PERCENT                (0.50f)
#define DUTY_75_PERCENT                (0.75f)
#define DUTY_STEP                      (0.10f)
#define DUTY_MIN                       (0.10f)
#define DUTY_MAX                       (0.90f)

/* Clock source for TOM timer */
#define GTM_TOM_MASTER                 IfxGtm_Tom_0              /* TOM instance selected to match P02.* availability */
#define GTM_TOM_MASTER_TIMER_CH        IfxGtm_Tom_Ch_0           /* Time base channel CH0 as required */

/* Port/pad configuration (from requirements) */
#define PAD_OUTPUT_MODE                IfxPort_OutputMode_pushPull
#define PAD_PAD_DRIVER                 IfxPort_PadDriver_cmosAutomotiveSpeed1

/* ========================= Pin mapping (per requirements) ========================= */
/*
 * P02.0/P02.7 -> Phase U (HS/LS)
 * P02.1/P02.4 -> Phase V (HS/LS)
 * P02.2/P02.5 -> Phase W (HS/LS)
 *
 * Notes:
 * - Mapping selected to TOM0 to satisfy availability of P02.* TOUTs on TC387 LFBGA-292.
 * - CH0 is used as time base (GTM_TOM_MASTER_TIMER_CH).
 * - If project constraints require TOM1 specifically and P02.* are not supported on TOM1,
 *   TOM instance selection must be revisited (pending user confirmation).
 */
#define PHASE_U_HS   (&IfxGtm_TOM0_0_TOUT0_P02_0_OUT)     /* P02.0 */
#define PHASE_U_LS   (&IfxGtm_TOM0_0N_TOUT7_P02_7_OUT)    /* P02.7 */
#define PHASE_V_HS   (&IfxGtm_TOM0_9_TOUT1_P02_1_OUT)     /* P02.1 (verify on target PinMap) */
#define PHASE_V_LS   (&IfxGtm_TOM0_12_TOUT4_P02_4_OUT)    /* P02.4 */
#define PHASE_W_HS   (&IfxGtm_TOM0_10_TOUT2_P02_2_OUT)    /* P02.2 */
#define PHASE_W_LS   (&IfxGtm_TOM0_13_TOUT5_P02_5_OUT)    /* P02.5 (verify on target PinMap) */

/* ========================= Driver state ========================= */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                      /* TOM timer handle */
    IfxGtm_Tom_PwmHl   pwm;                        /* PWMHL driver handle */
    Ifx_TimerValue     pwmOnTimes[NUM_OF_CHANNELS];/* On-time ticks per complementary pair */
    boolean            initialized;                /* Init state for runtime safety */
} GtmTom3PhaseOutput;

static GtmTom3PhaseOutput g_pwm3PhaseOutput = {0};

/* ========================= Public API implementation ========================= */
void initGtmTomPwm(void)
{
    g_pwm3PhaseOutput.initialized = FALSE;

    /* 1) Enable GTM module and configure CMU clocks for TOM */
    IfxGtm_enable(&MODULE_GTM);
    IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM));
    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* 2) Configure TOM timer */
    IfxGtm_Tom_Timer_Config timerConfig;
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);

    timerConfig.base.frequency = PWM_FREQ_HZ;
    timerConfig.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
    timerConfig.tom            = GTM_TOM_MASTER;
    timerConfig.timerChannel   = GTM_TOM_MASTER_TIMER_CH;

    if (IfxGtm_Tom_Timer_init(&g_pwm3PhaseOutput.timer, &timerConfig) == FALSE)
    {
        return; /* Early exit on failure */
    }

    /* 3) Configure pins via PinMap API (push-pull, cmosAutomotiveSpeed1) */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, PAD_OUTPUT_MODE, PAD_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, PAD_OUTPUT_MODE, PAD_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, PAD_OUTPUT_MODE, PAD_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, PAD_OUTPUT_MODE, PAD_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, PAD_OUTPUT_MODE, PAD_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, PAD_OUTPUT_MODE, PAD_PAD_DRIVER);

    /* 4) Configure PWMHL for 3 complementary channel pairs */
    IfxGtm_Tom_PwmHl_Config pwmHlConfig;

    /* Channel pin arrays (HS/LS) */
    IfxGtm_Tom_ToutMapP ccx[NUM_OF_CHANNELS] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
    IfxGtm_Tom_ToutMapP coutx[NUM_OF_CHANNELS] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

    IfxGtm_Tom_PwmHl_initConfig(&pwmHlConfig);

    pwmHlConfig.base.channelCount     = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
    pwmHlConfig.base.deadtime         = PWM_DEAD_TIME;            /* 0.5 us */
    pwmHlConfig.base.minPulse         = PWM_MIN_PULSE_TIME;       /* 1.0 us */
    pwmHlConfig.base.outputMode       = PAD_OUTPUT_MODE;
    pwmHlConfig.base.outputDriver     = PAD_PAD_DRIVER;
    pwmHlConfig.base.ccxActiveState   = Ifx_ActiveState_high;
    pwmHlConfig.base.coutxActiveState = Ifx_ActiveState_high;

    pwmHlConfig.ccx   = ccx;
    pwmHlConfig.coutx = coutx;
    pwmHlConfig.timer = &g_pwm3PhaseOutput.timer;
    pwmHlConfig.tom   = timerConfig.tom;

    if (IfxGtm_Tom_PwmHl_init(&g_pwm3PhaseOutput.pwm, &pwmHlConfig) == FALSE)
    {
        return; /* Early exit on failure */
    }

    if (IfxGtm_Tom_PwmHl_setMode(&g_pwm3PhaseOutput.pwm, Ifx_Pwm_Mode_centerAligned) == FALSE)
    {
        return; /* Early exit on failure */
    }

    /* Optional: ensure constraints (deadtime/minPulse) are applied and report errors if any */
    if (IfxGtm_Tom_PwmHl_setDeadtime(&g_pwm3PhaseOutput.pwm, PWM_DEAD_TIME) == FALSE)
    {
        return;
    }
    if (IfxGtm_Tom_PwmHl_setMinPulse(&g_pwm3PhaseOutput.pwm, PWM_MIN_PULSE_TIME) == FALSE)
    {
        return;
    }

    /* Update timer input frequency info and start timer */
    IfxGtm_Tom_Timer_updateInputFrequency(&g_pwm3PhaseOutput.timer);
    IfxGtm_Tom_Timer_run(&g_pwm3PhaseOutput.timer);

    /* 6) Compute and apply initial on-times (25% / 50% / 75% of period) atomically */
    {
        Ifx_TimerValue period = g_pwm3PhaseOutput.pwm.timer->base.period;
        g_pwm3PhaseOutput.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * DUTY_25_PERCENT);
        g_pwm3PhaseOutput.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * DUTY_50_PERCENT);
        g_pwm3PhaseOutput.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * DUTY_75_PERCENT);

        IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
        IfxGtm_Tom_PwmHl_setOnTime_Driver(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
    }

    g_pwm3PhaseOutput.initialized = TRUE;
}

void IfxGtm_Tom_PwmHl_setOnTime(void)
{
    if (g_pwm3PhaseOutput.initialized == FALSE)
    {
        return; /* Early exit if not initialized */
    }

    /* 1) Read current period */
    Ifx_TimerValue period = g_pwm3PhaseOutput.pwm.timer->base.period;

    /* 2) Compute step and boundaries from period */
    Ifx_TimerValue dutyStep   = (Ifx_TimerValue)((float32)period * DUTY_STEP);
    Ifx_TimerValue dutyStepMin= (Ifx_TimerValue)((float32)period * DUTY_MIN);
    Ifx_TimerValue dutyStepMax= (Ifx_TimerValue)((float32)period * DUTY_MAX);

    /* 3) Increment on-times and wrap at max to min */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_pwm3PhaseOutput.pwmOnTimes[i] += dutyStep;
        if (g_pwm3PhaseOutput.pwmOnTimes[i] >= dutyStepMax)
        {
            g_pwm3PhaseOutput.pwmOnTimes[i] = dutyStepMin;
        }
    }

    /* 4) Apply coherently */
    IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
    IfxGtm_Tom_PwmHl_setOnTime_Driver(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
    IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
}
