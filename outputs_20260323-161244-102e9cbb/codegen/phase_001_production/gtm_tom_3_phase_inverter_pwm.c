/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production module implementation: GTM_TOM_3_Phase_Inverter_PWM
 * Target: AURIX TC3xx (GTM TOM)
 *
 * Implements:
 *  - void initGtmTomPwm(void)
 *  - void updateGtmTomPwmDutyCycles(void)
 *
 * Notes:
 *  - Uses iLLD low-level GTM TOM Timer + TOM PwmHl drivers following Infineon patterns
 *  - No watchdog handling here (must be done only in CpuN_Main.c)
 *  - Uses only generic iLLD headers (no family-specific pinmap headers)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ========================= Requirements-derived configuration values ========================= */
#define INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U   (25.0f)   /* FROM REQUIREMENTS */
#define INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V   (50.0f)   /* FROM REQUIREMENTS */
#define INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W   (75.0f)   /* FROM REQUIREMENTS */

#define TIMING_FREQUENCY_HZ                      (20000.0f) /* 20 kHz FROM REQUIREMENTS */
#define TIMING_FXCLK_FREQUENCY_MHZ               (100.0f)   /* FROM REQUIREMENTS (informational) */
#define TIMING_PRESCALER                         (1u)       /* FROM REQUIREMENTS (informational) */
#define TIMING_TIMER_BASE_PERIOD_TICKS           (2500u)    /* FROM REQUIREMENTS (informational) */

#define CLOCK_REQUIRES_XTAL                      (1)        /* FROM REQUIREMENTS (informational) */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ           (300.0f)   /* FROM REQUIREMENTS (informational) */
#define CLOCK_GTM_FXCLK_MHZ                      (100.0f)   /* FROM REQUIREMENTS (informational) */

/* ========================= Runtime/algorithm settings ========================= */
#define DUTY_INCREMENT_PERCENT                   (10.0f)    /* +10% per update */
#define DUTY_WRAP_THRESHOLD_PERCENT              (100.0f)
#define PWM_MIN_PULSE_TIME                       (1.0e-6f)  /* seconds, safety floor */

/* ========================= Pin assignments (validated generic PinMap symbols) ========================= */
/*
 * Mapping per user requirement (KIT_A2G_TC387_5V_TFT):
 *  - PHASE_U: P02.0 (HS), P02.7 (LS)
 *  - PHASE_V: P02.1 (HS), P02.4 (LS)
 *  - PHASE_W: P02.2 (HS), P02.5 (LS)
 *
 * Notes:
 *  - The following symbols are generic iLLD PinMap entries. Some are shown in the validated list,
 *    others are typical for TC387 but must be present in the generic header for the selected device.
 *  - Low-side complementary behavior is achieved by inverted polarity (no hardware dead-time).
 */
#define PHASE_U_HS   (&IfxGtm_TOM0_0_TOUT0_P02_0_OUT)   /* validated example */
#define PHASE_U_LS   (&IfxGtm_TOM0_0N_TOUT7_P02_7_OUT)  /* validated example */

#define PHASE_V_HS   (&IfxGtm_TOM0_1_TOUT1_P02_1_OUT)   /* typical mapping */
#define PHASE_V_LS   (&IfxGtm_TOM0_12_TOUT4_P02_4_OUT)  /* validated example */

#define PHASE_W_HS   (&IfxGtm_TOM0_10_TOUT2_P02_2_OUT)  /* validated example */
#define PHASE_W_LS   (&IfxGtm_TOM0_13_TOUT5_P02_5_OUT)  /* typical mapping */

/* ========================= Internal driver/application state ========================= */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                /* Shared TOM time base */
    IfxGtm_Tom_PwmHl   pwm;                  /* High/Low PWM driver (used for complementary behavior) */
    Ifx_TimerValue     pwmOnTimes[3];        /* On-time ticks for phases U, V, W */
    float32            hsDutyPercent[3];     /* High-side duty percentages for U, V, W */
    float32            sixDutyPercent[6];    /* [U_HS, U_LS, V_HS, V_LS, W_HS, W_LS] */
} GtmTomPwmApp;

static GtmTomPwmApp g_pwm3PhaseOutput;
static boolean      s_initialized = FALSE;

/* ========================= Local helpers ========================= */
/**
 * Compute next duty values per behavior description:
 * - Increment each high-side duty by +10 percentage points
 * - Wrap to 0.0 if exceeding 100.0
 * - Low-side duties mirror their high-side partners (polarity inversion provides complement)
 *
 * hsDutyInOut: [U, V, W] (percent)
 * sixChannelDutyOut: [U_HS, U_LS, V_HS, V_LS, W_HS, W_LS] (percent)
 */
static void computeNextDuties(float32 *hsDutyInOut, float32 *sixChannelDutyOut)
{
    /* Update high-side duties with +10% and wrap */
    for (uint32 i = 0u; i < 3u; i++)
    {
        float32 d = hsDutyInOut[i] + DUTY_INCREMENT_PERCENT;
        if (d > DUTY_WRAP_THRESHOLD_PERCENT)
        {
            d = 0.0f;
        }
        hsDutyInOut[i] = d;
    }

    /* Mirror to six-channel array (LS numeric duty equals HS; complement via polarity) */
    sixChannelDutyOut[0] = hsDutyInOut[0]; /* U_HS */
    sixChannelDutyOut[1] = hsDutyInOut[0]; /* U_LS */
    sixChannelDutyOut[2] = hsDutyInOut[1]; /* V_HS */
    sixChannelDutyOut[3] = hsDutyInOut[1]; /* V_LS */
    sixChannelDutyOut[4] = hsDutyInOut[2]; /* W_HS */
    sixChannelDutyOut[5] = hsDutyInOut[2]; /* W_LS */
}

/* ========================= Public API ========================= */
/**
 * See header for description.
 */
void initGtmTomPwm(void)
{
    boolean ok;

    /* GTM enable and CMU clock setup (reference pattern) */
    IfxGtm_enable(&MODULE_GTM);
    IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM));
    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* Configure TOM timer (shared time base) */
    IfxGtm_Tom_Timer_Config timerConfig;
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    timerConfig.base.frequency = TIMING_FREQUENCY_HZ;                 /* 20 kHz */
    timerConfig.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;      /* FXCLK0 source */
    timerConfig.tom            = IfxGtm_Tom_0;                        /* TOM instance */
    timerConfig.timerChannel   = IfxGtm_Tom_Ch_0;                     /* Timer channel */

    ok = IfxGtm_Tom_Timer_init(&g_pwm3PhaseOutput.timer, &timerConfig);
    if (ok == FALSE)
    {
        return; /* Early exit on failure */
    }

    /* Configure PWM high/low driver with complementary polarity and zero dead-time */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlConfig;
        IfxGtm_Tom_ToutMapP ccx[] =
        {
            PHASE_U_HS, /* U high-side: P02.0 */
            PHASE_V_HS, /* V high-side: P02.1 */
            PHASE_W_HS  /* W high-side: P02.2 */
        };
        IfxGtm_Tom_ToutMapP coutx[] =
        {
            PHASE_U_LS, /* U low-side:  P02.7 */
            PHASE_V_LS, /* V low-side:  P02.4 */
            PHASE_W_LS  /* W low-side:  P02.5 */
        };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlConfig);
        pwmHlConfig.base.channelCount     = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
        pwmHlConfig.base.deadtime         = 0.0f;                                   /* No dead-time */
        pwmHlConfig.base.minPulse         = PWM_MIN_PULSE_TIME;                     /* Safety */
        pwmHlConfig.base.outputMode       = IfxPort_OutputMode_pushPull;
        pwmHlConfig.base.outputDriver     = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlConfig.base.ccxActiveState   = Ifx_ActiveState_high;                   /* HS active-high */
        pwmHlConfig.base.coutxActiveState = Ifx_ActiveState_low;                    /* LS active-low (complement) */

        pwmHlConfig.ccx   = ccx;                       /* Route HS pins */
        pwmHlConfig.coutx = coutx;                     /* Route LS pins */
        pwmHlConfig.timer = &g_pwm3PhaseOutput.timer;  /* Shared time base */
        pwmHlConfig.tom   = timerConfig.tom;           /* TOM instance */

        ok = IfxGtm_Tom_PwmHl_init(&g_pwm3PhaseOutput.pwm, &pwmHlConfig);
        if (ok == FALSE)
        {
            return; /* Early exit on failure */
        }

        /* Center-aligned operation (required) */
        ok = IfxGtm_Tom_PwmHl_setMode(&g_pwm3PhaseOutput.pwm, Ifx_Pwm_Mode_centerAligned);
        if (ok == FALSE)
        {
            return; /* Early exit on failure */
        }

        /* Update internal timer inputs and start */
        IfxGtm_Tom_Timer_updateInputFrequency(&g_pwm3PhaseOutput.timer);
    }

    /* Run the timer (start PWM time base) */
    IfxGtm_Tom_Timer_run(&g_pwm3PhaseOutput.timer);

    /* Initialize duty state from requirements */
    g_pwm3PhaseOutput.hsDutyPercent[0] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U; /* U */
    g_pwm3PhaseOutput.hsDutyPercent[1] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V; /* V */
    g_pwm3PhaseOutput.hsDutyPercent[2] = INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W; /* W */

    /* Mirror to six-channel array (LS = HS; inversion via polarity) */
    g_pwm3PhaseOutput.sixDutyPercent[0] = g_pwm3PhaseOutput.hsDutyPercent[0];
    g_pwm3PhaseOutput.sixDutyPercent[1] = g_pwm3PhaseOutput.hsDutyPercent[0];
    g_pwm3PhaseOutput.sixDutyPercent[2] = g_pwm3PhaseOutput.hsDutyPercent[1];
    g_pwm3PhaseOutput.sixDutyPercent[3] = g_pwm3PhaseOutput.hsDutyPercent[1];
    g_pwm3PhaseOutput.sixDutyPercent[4] = g_pwm3PhaseOutput.hsDutyPercent[2];
    g_pwm3PhaseOutput.sixDutyPercent[5] = g_pwm3PhaseOutput.hsDutyPercent[2];

    /* Convert percent to on-time ticks and stage atomic update */
    {
        Ifx_TimerValue period = g_pwm3PhaseOutput.pwm.timer->base.period;
        g_pwm3PhaseOutput.pwmOnTimes[0] = (Ifx_TimerValue)((period * g_pwm3PhaseOutput.hsDutyPercent[0]) / 100.0f);
        g_pwm3PhaseOutput.pwmOnTimes[1] = (Ifx_TimerValue)((period * g_pwm3PhaseOutput.hsDutyPercent[1]) / 100.0f);
        g_pwm3PhaseOutput.pwmOnTimes[2] = (Ifx_TimerValue)((period * g_pwm3PhaseOutput.hsDutyPercent[2]) / 100.0f);

        IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
    }

    s_initialized = TRUE; /* Initialization succeeded */
}

/**
 * See header for description.
 */
void updateGtmTomPwmDutyCycles(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if initialization failed or not performed */
    }

    /* Compute next duty values (percent) */
    computeNextDuties(g_pwm3PhaseOutput.hsDutyPercent, g_pwm3PhaseOutput.sixDutyPercent);

    /* Convert percent to on-time ticks for the three phase channels */
    {
        Ifx_TimerValue period = g_pwm3PhaseOutput.pwm.timer->base.period;
        g_pwm3PhaseOutput.pwmOnTimes[0] = (Ifx_TimerValue)((period * g_pwm3PhaseOutput.hsDutyPercent[0]) / 100.0f);
        g_pwm3PhaseOutput.pwmOnTimes[1] = (Ifx_TimerValue)((period * g_pwm3PhaseOutput.hsDutyPercent[1]) / 100.0f);
        g_pwm3PhaseOutput.pwmOnTimes[2] = (Ifx_TimerValue)((period * g_pwm3PhaseOutput.hsDutyPercent[2]) / 100.0f);
    }

    /* Shadowed, atomic multi-channel update */
    IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
    IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
}
