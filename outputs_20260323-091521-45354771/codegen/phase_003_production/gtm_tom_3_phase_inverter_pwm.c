#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"   /* Generic header - family-agnostic */
#include "IfxPort.h"

/* ===================== Requirements-driven configuration ===================== */
#define TIMING_PWM_FREQUENCY_HZ            (20000.0f)     /* 20 kHz */
#define TIMING_DUTY_STEP_PERCENT           (10.0f)        /* +10% per update */
#define TIMING_STEP_INTERVAL_MS            (500u)         /* Caller timing */
#define TIMING_APPLY_UPDATES_AT_PERIOD_BOUNDARY   (1)
#define CLOCK_REQUIRES_XTAL               (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ    (300u)

/* Complementary pair pins (KIT_A2G_TC387_5V_TFT per requirements) */
#define PIN_PAIRS_U_HIGH_PIN              P00_3
#define PIN_PAIRS_U_LOW_PIN               P00_2
#define PIN_PAIRS_V_HIGH_PIN              P00_5
#define PIN_PAIRS_V_LOW_PIN               P00_4
#define PIN_PAIRS_W_HIGH_PIN              P00_7
#define PIN_PAIRS_W_LOW_PIN               P00_6

/* ===================== Phase-to-TOM TOUT mappings (validated) ===================== */
/* Use TOM1 and route to P00.x per requirements. These symbols are provided by IfxGtm_PinMap.h */
#define PHASE_U_HS   &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS   &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS   &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS   &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS   &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS   &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* ===================== Runtime behavior macros (follow reference naming) ===================== */
#define PWM_DEAD_TIME        (0.5e-6f)   /* 0.5 us (seconds) */
#define PWM_MIN_PULSE_TIME   (1.0e-6f)   /* 1.0 us (seconds) */
#define DUTY_25_PERCENT      (0.25f)
#define DUTY_50_PERCENT      (0.50f)
#define DUTY_75_PERCENT      (0.75f)
#define DUTY_STEP            (0.10f)     /* +10% per update in fraction */

#define NUM_PHASES           (3u)

/* ===================== Module state ===================== */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                /* TOM time base driver */
    IfxGtm_Tom_PwmHl   pwm;                  /* Complementary PWM driver */
    Ifx_TimerValue     pwmOnTimes[NUM_PHASES];/* On-time ticks for U,V,W */
    boolean            initialized;          /* Init status */
} GtmTom3PhasePwm_t;

static GtmTom3PhasePwm_t g_pwm3PhaseOutput = {0};

/* ===================== Local helpers ===================== */
static void configureTom1Pins(void)
{
    /* Map TOM1 outputs to required P00 pins using the generic PinMap API */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/* ===================== Public functions ===================== */
/**
 * See header for description.
 */
void initGtmTom3phInv(void)
{
    /* Early clear state */
    g_pwm3PhaseOutput.initialized = FALSE;

    /* 1) Enable GTM and CMU clocks (FXCLK0 used for TOM1) */
    IfxGtm_enable(&MODULE_GTM);
    /* Derive and enable GCLK and CLK0 from module frequency */
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Configure TOM1 time base for 20 kHz center-aligned PWM */
    {
        IfxGtm_Tom_Timer_Config timerConfig;
        IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);

        timerConfig.base.frequency = TIMING_PWM_FREQUENCY_HZ;
        timerConfig.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;   /* Use FXCLK0 */
        timerConfig.tom            = IfxGtm_Tom_1;                     /* Keep TOM1 */
        timerConfig.timerChannel   = IfxGtm_Tom_Ch_0;                  /* Master time base on CH0 */

        if (IfxGtm_Tom_Timer_init(&g_pwm3PhaseOutput.timer, &timerConfig) == FALSE)
        {
            return; /* Error: do not proceed */
        }
    }

    /* 3) Route pins explicitly (complementary pairs on P00.x) */
    configureTom1Pins();

    /* 4) Configure complementary PWM HL with 3 channel pairs */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlConfig;
        IfxGtm_Tom_ToutMapP ccx[NUM_PHASES] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
        IfxGtm_Tom_ToutMapP coutx[NUM_PHASES] = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlConfig);

        pwmHlConfig.base.channelCount     = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
        pwmHlConfig.base.deadtime         = PWM_DEAD_TIME;                 /* 0.5 us */
        pwmHlConfig.base.minPulse         = PWM_MIN_PULSE_TIME;           /* 1.0 us */
        pwmHlConfig.base.outputMode       = IfxPort_OutputMode_pushPull;
        pwmHlConfig.base.outputDriver     = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlConfig.base.ccxActiveState   = Ifx_ActiveState_high;
        pwmHlConfig.base.coutxActiveState = Ifx_ActiveState_high;

        pwmHlConfig.ccx   = ccx;
        pwmHlConfig.coutx = coutx;
        pwmHlConfig.timer = &g_pwm3PhaseOutput.timer;
        pwmHlConfig.tom   = IfxGtm_Tom_1;     /* Keep TOM1 */

        if (IfxGtm_Tom_PwmHl_init(&g_pwm3PhaseOutput.pwm, &pwmHlConfig) == FALSE)
        {
            return; /* Error */
        }

        if (IfxGtm_Tom_PwmHl_setMode(&g_pwm3PhaseOutput.pwm, Ifx_Pwm_Mode_centerAligned) == FALSE)
        {
            return; /* Error */
        }

        /* Explicitly apply constraints after init as per SW design */
        if (IfxGtm_Tom_PwmHl_setDeadtime(&g_pwm3PhaseOutput.pwm, PWM_DEAD_TIME) == FALSE)
        {
            return; /* Error */
        }
        if (IfxGtm_Tom_PwmHl_setMinPulse(&g_pwm3PhaseOutput.pwm, PWM_MIN_PULSE_TIME) == FALSE)
        {
            return; /* Error */
        }

        /* Update derived input frequency if required by the driver */
        IfxGtm_Tom_Timer_updateInputFrequency(&g_pwm3PhaseOutput.timer);
    }

    /* 5) Start the time base */
    IfxGtm_Tom_Timer_run(&g_pwm3PhaseOutput.timer);

    /* 6) Compute and program initial on-times: U=25%, V=50%, W=75% */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_pwm3PhaseOutput.timer);
        g_pwm3PhaseOutput.pwmOnTimes[0] = period * DUTY_25_PERCENT;
        g_pwm3PhaseOutput.pwmOnTimes[1] = period * DUTY_50_PERCENT;
        g_pwm3PhaseOutput.pwmOnTimes[2] = period * DUTY_75_PERCENT;

        IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
    }

    g_pwm3PhaseOutput.initialized = TRUE;
}

/**
 * See header for description.
 */
void updateGtmTom3phInvDuty(void)
{
    if (g_pwm3PhaseOutput.initialized == FALSE)
    {
        return; /* Early exit on uninitialized driver */
    }

    /* Obtain current period and compute +10% step */
    {
        Ifx_TimerValue period    = IfxGtm_Tom_Timer_getPeriod(&g_pwm3PhaseOutput.timer);
        Ifx_TimerValue dutyStep  = period * DUTY_STEP; /* 10% of period in ticks */

        /* Increment each phase and wrap at 100% (i.e., period) */
        for (uint8 i = 0u; i < NUM_PHASES; i++)
        {
            g_pwm3PhaseOutput.pwmOnTimes[i] += dutyStep;
            if (g_pwm3PhaseOutput.pwmOnTimes[i] >= period)
            {
                g_pwm3PhaseOutput.pwmOnTimes[i] = 0.0f;
            }
        }

        /* Apply synchronously at next period boundary */
        IfxGtm_Tom_Timer_disableUpdate(&g_pwm3PhaseOutput.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_pwm3PhaseOutput.pwm, g_pwm3PhaseOutput.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&g_pwm3PhaseOutput.timer);
    }
}
