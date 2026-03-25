/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * TC3xx (TC387) unified GTM TOM 3-phase inverter PWM using IfxGtm_Pwm.
 *
 * Key rules implemented:
 * - High-level IfxGtm_Pwm driver with Config/ChannelConfig/OutputConfig arrays
 * - No PinMap setTomTout() calls; routing via OutputConfig.pin
 * - No redundant startSyncedChannels/update calls post init; use config.syncStart/SyncUpdate
 * - GTM/CMU clock enable and FXCLK setup
 * - Complementary outputs disabled per requirements; low-side pins driven low as GPIO
 * - Runtime duty update: +10% per call, wrap within 0..100%, clamp 100% to (period-1 tick)
 *
 * Watchdog policy: never disable watchdogs here (only in CpuN_Main.c).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"   /* Generic family-safe PinMap */
#include "IfxPort.h"

/* ========================= Requirements-derived macros (DO NOT CHANGE) ========================= */
#define NUM_OF_CHANNELS                               (3u)
#define PWM_FREQUENCY                                 (20000.0f)  /* 20 kHz */

#define INITIAL_DUTY_PERCENT_U                        (25.0f)
#define INITIAL_DUTY_PERCENT_V                        (50.0f)
#define INITIAL_DUTY_PERCENT_W                        (75.0f)

#define UPDATE_POLICY_INTERVAL_MS                      (500u)     /* Caller should invoke update every 500 ms */
#define UPDATE_POLICY_INCREMENT_PERCENT                (10.0f)
#define UPDATE_POLICY_CLAMP_100_TO_PERIOD_MINUS_1_TICK (1u)
#define UPDATE_POLICY_UPDATE_API                       IfxGtm_Pwm_updateChannelsDutyImmediate

#define COMPLEMENTARY_OUTPUTS_REQUIRED                 (0u)
#define COMPLEMENTARY_OUTPUTS_DEADTIME_US              (0.5f)
#define COMPLEMENTARY_OUTPUTS_ENABLE_TOM_CDTM_DTM      (0u)

#define TIMING_CENTER_ALIGNED                          (1u)
#define TIMING_SYNCHRONOUS_SHADOW_TRANSFER             (1u)

#define CLOCK_REQUIRES_XTAL                            (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ                 (300u)
#define CLOCK_GTM_CMU_FXCLK_MHZ                        (100u)

/* ========================= Pin assignments (KIT_A2G_TC387 P00.2..P00.7 TOM1) =========================
 * Maintain existing TOM1 pin map (HS only for now). Low-side pins configured as GPIO low.
 * Note: Use generic IfxGtm_PinMap.h symbols.
 */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ========================= Driver state structure (per unified IfxGtm_Pwm pattern) ========================= */
typedef struct {
    IfxGtm_Pwm          pwm;                                   /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];             /* Channel data post init */
    float32             dutyCycles[NUM_OF_CHANNELS];            /* Duty cycle percent values */
    float32             phases[NUM_OF_CHANNELS];                /* Phase shift values (deg or percent of period) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];            /* Unused now (no complementary) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;                            /* Static driver instance */
static boolean       s_initialized = FALSE;                    /* Init flag */

/* ========================= Internal helpers ========================= */
/**
 * Compute the duty percentage equivalent to (period - 1 tick).
 * Uses configured PWM_FREQUENCY and current FXCLK frequency for TOM.
 */
static float32 prv_getDutyPercentMinusOneTick(void)
{
    float32 fxclkHz = IfxGtm_Cmu_getFxclkFrequency(&MODULE_GTM, 0u, TRUE);
    if (fxclkHz <= 0.0f) {
        return 99.0f; /* Fallback clamp if clock info not available */
    }
    float32 periodTicks = fxclkHz / PWM_FREQUENCY;
    if (periodTicks < 1.0f) {
        return 99.0f;
    }
    /* 100 * (period-1)/period = 100 - (100/period) */
    float32 duty = 100.0f - (100.0f / periodTicks);
    /* Bound within [0,100) */
    if (duty >= 100.0f) {
        duty = 99.999f;
    } else if (duty < 0.0f) {
        duty = 0.0f;
    }
    return duty;
}

/* ========================= Public API implementation ========================= */
/**
 * Initialize unified IfxGtm_Pwm for 3-phase TOM PWM at 20 kHz, center-aligned, sync shadow transfers.
 * - Enables GTM, sets GCLK and CLK0, enables FXCLK and CLK0
 * - Routes phase HS pins via OutputConfig; LS pins left unused by PWM and forced low via GPIO
 * - Sets initial duties U/V/W = 25/50/75% via channelConfig; no redundant update call
 * - Starts channels synchronously via config.syncStart = TRUE
 */
void initGtmTomPwm(void)
{
    /* 1) Enable GTM and configure CMU clocks (GCLK and FXCLK/CLK0) */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
    }
    /* Set GCLK to module frequency (derived from system clock) */
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
    }
    /* Set CMU CLK0 to 100 MHz per requirement and enable FXCLK + CLK0 */
    {
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, (float32)(CLOCK_GTM_CMU_FXCLK_MHZ * 1000000u));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Prepare unified PWM configuration and per-channel configs */
    IfxGtm_Pwm_Config          config;
    IfxGtm_Pwm_ChannelConfig   channelCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    outputCfg[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output pin routing via OutputConfig (no explicit PinMap calls) */
    /* U phase (channel 0 in our logical array) */
    outputCfg[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    outputCfg[0].complementaryPin      = NULL_PTR; /* Complementary disabled */
    outputCfg[0].polarity              = Ifx_ActiveState_high;
    outputCfg[0].complementaryPolarity = Ifx_ActiveState_low;
    outputCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase (channel 1) */
    outputCfg[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    outputCfg[1].complementaryPin      = NULL_PTR;
    outputCfg[1].polarity              = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity = Ifx_ActiveState_low;
    outputCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase (channel 2) */
    outputCfg[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    outputCfg[2].complementaryPin      = NULL_PTR;
    outputCfg[2].polarity              = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity = Ifx_ActiveState_low;
    outputCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Initialize per-channel defaults and set required fields */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++) {
        IfxGtm_Pwm_initChannelConfig(&channelCfg[i]);
        channelCfg[i].output    = &outputCfg[i];
        channelCfg[i].dtm       = NULL_PTR;    /* No DTM since complementary disabled */
        channelCfg[i].mscOut    = NULL_PTR;    /* Not used */
        channelCfg[i].interrupt = NULL_PTR;    /* No ISR requested */
        channelCfg[i].phase     = 0.0f;        /* No phase shift */
    }

    /* Assign timer channel indices (logical mapping; unified driver resolves with pin map) */
    channelCfg[0].timerCh = IfxGtm_Pwm_SubModule_Ch_0; /* U */
    channelCfg[1].timerCh = IfxGtm_Pwm_SubModule_Ch_1; /* V */
    channelCfg[2].timerCh = IfxGtm_Pwm_SubModule_Ch_2; /* W */

    /* Set initial duty cycle (%) per requirement */
    channelCfg[0].duty = INITIAL_DUTY_PERCENT_U;
    channelCfg[1].duty = INITIAL_DUTY_PERCENT_V;
    channelCfg[2].duty = INITIAL_DUTY_PERCENT_W;

    /* Configure top-level PWM parameters */
    config.cluster             = IfxGtm_Cluster_0;
    config.subModule           = IfxGtm_Pwm_SubModule_tom;          /* TOM submodule */
    config.alignment           = IfxGtm_Pwm_Alignment_center;       /* Center-aligned */
    config.syncStart           = TRUE;                              /* Auto-start channels after init */
    config.syncUpdateEnabled   = TRUE;                              /* Shadow transfers synchronized */
    config.numChannels         = NUM_OF_CHANNELS;
    config.channels            = &channelCfg[0];
    config.frequency           = PWM_FREQUENCY;
    config.clockSource.tom     = IfxGtm_Cmu_Fxclk_0;                /* TOM uses Fxclk enum */
    /* No DTM clock configured since complementary outputs are disabled */

    /* Initialize PWM driver (no return value per API contract) */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* Store runtime duty values */
    g_gtmTom3phInv.dutyCycles[0] = channelCfg[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelCfg[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelCfg[2].duty;

    /* Force unused complementary (LS) pins to safe state (GPIO push-pull, low) */
    IfxPort_setPinModeOutput(&MODULE_P00, 2u, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general); /* U_LS P00.2 */
    IfxPort_setPinLow(&MODULE_P00, 2u);
    IfxPort_setPinModeOutput(&MODULE_P00, 4u, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general); /* V_LS P00.4 */
    IfxPort_setPinLow(&MODULE_P00, 4u);
    IfxPort_setPinModeOutput(&MODULE_P00, 6u, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general); /* W_LS P00.6 */
    IfxPort_setPinLow(&MODULE_P00, 6u);

    s_initialized = TRUE;
}

/**
 * Runtime duty maintenance per detailed design:
 * - Each invocation increments U/V/W by +10 percentage points
 * - Wrap within 0..100%; if an exact 100% request occurs, clamp to (period-1 tick) percent
 * - Apply atomically using IfxGtm_Pwm_updateChannelsDutyImmediate
 *
 * Note: The caller must invoke this function every UPDATE_POLICY_INTERVAL_MS (500 ms).
 */
void updateGtmTomPwmDutyCycles(void)
{
    if (!s_initialized) {
        return; /* Early exit if init not completed */
    }

    float32 dutyMaxLess1Tick = prv_getDutyPercentMinusOneTick();

    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        float32 d = g_gtmTom3phInv.dutyCycles[i];
        /* Increment */
        float32 newD = d + UPDATE_POLICY_INCREMENT_PERCENT;

        /* Exact 100% clamp path (handle the 90% -> 100% step explicitly) */
        if ((d <= (100.0f - UPDATE_POLICY_INCREMENT_PERCENT + 1e-6f)) && (newD > 100.0f))
        {
            /* Crossed above 100 due to step (e.g., 95->105): wrap */
            newD = newD - 100.0f; /* Wrap into 0..100 */
        }
        else if ((d >= (100.0f - UPDATE_POLICY_INCREMENT_PERCENT - 1e-6f)) && (newD >= 100.0f - 1e-6f) && (newD <= 100.0f + 1e-6f))
        {
            /* Hit exactly 100% (e.g., 90->100): clamp to period-1 tick */
            newD = dutyMaxLess1Tick;
        }
        else if (newD > 100.0f)
        {
            /* Generic wrap for values >100 */
            newD = newD - 100.0f;
        }

        /* Ensure bounds [0, 100) after adjustments */
        if (newD >= 100.0f) {
            newD = dutyMaxLess1Tick;
        }
        if (newD < 0.0f) {
            newD = 0.0f;
        }

        g_gtmTom3phInv.dutyCycles[i] = newD;
    }

    /* Apply updated duties atomically */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
