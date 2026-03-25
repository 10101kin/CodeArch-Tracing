/***************************************************************************
 * File: gtm_tom_3_phase_inverter_pwm.c
 * Description: GTM TOM 3-Phase Inverter PWM - Unified IfxGtm_Pwm driver API
 * Target: AURIX TC3xx
 *
 * CRITICAL RULES IMPLEMENTED:
 * - Unified IfxGtm_Pwm driver with Config/ChannelConfig/OutputConfig arrays
 * - GTM/CMU clock enable prior to PWM init
 * - Center-aligned mode, syncStart + syncUpdate enabled
 * - No watchdog handling here (handled only in CpuN_Main.c)
 * - Complementary outputs not required: not routed by PWM, set safe GPIO low
 * - Pin assignment via OutputConfig (no explicit PinMap routing for PWM pins)
 * - Runtime duty update via IfxGtm_Pwm_updateChannelsDutyImmediate
 ***************************************************************************/
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm_Pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

/* ===================== Configuration Macros (from Requirements) ===================== */
#define NUM_OF_CHANNELS                          (3u)
#define PWM_FREQUENCY_HZ                         (20000.0f)         /* 20 kHz */

#define INITIAL_DUTY_PERCENT_U                   (25.0f)
#define INITIAL_DUTY_PERCENT_V                   (50.0f)
#define INITIAL_DUTY_PERCENT_W                   (75.0f)

#define UPDATE_POLICY_INTERVAL_MS                (500u)
#define UPDATE_POLICY_INCREMENT_PERCENT          (10.0f)
#define UPDATE_TASK_PERIOD_MS                    (10u)              /* Caller period (Cpu0 loop) */

/* Clock expectations (informational) */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ           (300u)
#define CLOCK_GTM_CMU_FXCLK_MHZ                  (100u)

/* Complementary outputs are NOT required per requirements */
#define COMPLEMENTARY_OUTPUTS_REQUIRED           (0u)

/* ISR configuration */
#define ISR_PRIORITY_TOM                         (20)
#define LED                                      &MODULE_P13, 0

/* ============================== Validated Pin Assignments ============================== */
/* Keep existing TOM1 / P00.2–P00.7 mapping (validated for TC38x family) */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ============================== Driver Structure ============================== */
typedef struct {
    IfxGtm_Pwm          pwm;                                      /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];                /* Channel runtime data */
    float32             dutyCycles[NUM_OF_CHANNELS];              /* Duty arrays in percent */
    float32             phases[NUM_OF_CHANNELS];                  /* Phase shifts (unused -> 0) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];               /* Dead time values (unused) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;                               /* Module instance */
static boolean      s_initialized = FALSE;                         /* Init state */
static uint32       s_msAccumulator = 0u;                          /* Update interval accumulator */

/* ============================== ISR for diagnostic ============================== */
IFX_INTERRUPT(interruptGtmTom, 0, ISR_PRIORITY_TOM);
void interruptGtmTom(void)
{
    IfxPort_togglePin(LED);
}

/* Optional period event callback (linked via interruptConfig if used) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* No-op */
}

/* ============================== Initialization ============================== */
/**
 * Initialize unified IfxGtm_Pwm for 3-phase TOM1 center-aligned PWM at 20 kHz.
 * - Enables GTM + CMU FXCLK
 * - Routes TOM1 HS pins via unified driver OutputConfig
 * - Complementary pins configured as GPIO low (safe state)
 * - Synchronous start/update enabled, initial duty set to 25/50/75%
 */
void initGtmTomPwm(void)
{
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 1) GTM module enable + CMU clock setup (GCLK/CLK0 + FXCLK enable) */
    IfxGtm_enable(&MODULE_GTM);
    {
        float32 modFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, modFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Initialize config defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Interrupt configuration (bind to first channel if needed) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_TOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 4) Output pin routing via unified driver (no explicit PinMap_setTomTout) */
    /* Channel 0 -> Phase U HS (P00.3) */
    output[0].pin                      = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin         = NULL_PTR; /* Complementary not required */
    output[0].polarity                 = Ifx_ActiveState_high;
    output[0].complementaryPolarity    = Ifx_ActiveState_low;
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 1 -> Phase V HS (P00.5) */
    output[1].pin                      = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin         = NULL_PTR;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_low;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 2 -> Phase W HS (P00.7) */
    output[2].pin                      = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin         = NULL_PTR;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_low;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) Channel configuration (per-channel) */
    /* Initialize defaults then override fields */
    IfxGtm_Pwm_initChannelConfig(&channelConfig[0]);
    IfxGtm_Pwm_initChannelConfig(&channelConfig[1]);
    IfxGtm_Pwm_initChannelConfig(&channelConfig[2]);

    /* Map to TOM1 channels 2,4,6 for HS outputs */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = INITIAL_DUTY_PERCENT_U;
    channelConfig[0].dtm       = NULL_PTR;                /* DTM not used */
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;        /* Bind ISR to first channel */

    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_4;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = INITIAL_DUTY_PERCENT_V;
    channelConfig[1].dtm       = NULL_PTR;
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_6;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = INITIAL_DUTY_PERCENT_W;
    channelConfig[2].dtm       = NULL_PTR;
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 6) Global PWM configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;  /* Center-aligned */
    config.syncStart            = TRUE;                          /* Auto-start channels */
    config.syncUpdateEnabled    = TRUE;                          /* Shadow transfer */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;            /* TOM uses FXCLK_0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 7) Initialize PWM driver */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 8) Cache initial duty values for runtime updates */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;  /* U */
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;  /* V */
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;  /* W */
    g_gtmTom3phInv.phases[0]     = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1]     = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2]     = channelConfig[2].phase;

    /* 9) Configure complementary pins to safe GPIO low (not used currently) */
    IfxPort_setPinModeOutput(&MODULE_P00, 2u, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general); /* P00.2 */
    IfxPort_setPinLow(&MODULE_P00, 2u);
    IfxPort_setPinModeOutput(&MODULE_P00, 4u, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general); /* P00.4 */
    IfxPort_setPinLow(&MODULE_P00, 4u);
    IfxPort_setPinModeOutput(&MODULE_P00, 6u, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general); /* P00.6 */
    IfxPort_setPinLow(&MODULE_P00, 6u);

    /* 10) LED for ISR diagnostic */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(LED);

    /* 11) Apply initial duty via unified immediate update and start synced outputs */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
    IfxGtm_Pwm_startSyncedChannels(&g_gtmTom3phInv.pwm);

    s_msAccumulator = 0u;
    s_initialized   = TRUE;
}

/* ============================== Runtime Update ============================== */
/**
 * Update three phase duties every 500 ms by +10% (wrap 0–100%, clamp 100% to below-period).
 * - Maintains an internal ms accumulator (assumes caller period = UPDATE_TASK_PERIOD_MS)
 * - Applies atomic multi-channel update via IfxGtm_Pwm_updateChannelsDutyImmediate
 */
void updateGtmTomPwmDutyCycles(void)
{
    uint8 i;

    if (s_initialized == FALSE)
    {
        return; /* Early exit if init not completed */
    }

    /* Accumulate elapsed time (software counter) */
    s_msAccumulator += (uint32)UPDATE_TASK_PERIOD_MS;
    if (s_msAccumulator < (uint32)UPDATE_POLICY_INTERVAL_MS)
    {
        return; /* Not time yet */
    }

    /* Interval reached: step all phases by +10% */
    for (i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        float32 d = g_gtmTom3phInv.dutyCycles[i] + (float32)UPDATE_POLICY_INCREMENT_PERCENT;
        if (d >= 100.0f)
        {
            /* Wrap to 0%; also ensure we never command exact 100% */
            d = 0.0f;
        }
        /* Extra safety: if any path ever yields exactly 100, clamp just below full period */
        if (d >= 100.0f)
        {
            d = 99.999f; /* Equivalent to period-1-tick in percent domain */
        }
        g_gtmTom3phInv.dutyCycles[i] = d;
    }

    /* Atomic multi-channel update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);

    /* Preserve remainder if caller period doesn't divide 500 ms evenly */
    s_msAccumulator -= (uint32)UPDATE_POLICY_INTERVAL_MS;
}
