/*
 * gtm_tom_3_phase_inverter_pwm.c
 * TC3xx (TC387) - 3-Phase Inverter PWM using IfxGtm_Pwm (TOM)
 *
 * Implementation strictly follows SW Detailed Design behavior and iLLD patterns:
 * - Unified IfxGtm_Pwm high-level driver
 * - 20 kHz center-aligned, complementary pairs with dead-time
 * - TOM1 pins on P00.2..P00.7 (KIT_A2G_TC387_5V_TFT)
 * - Initial duties: U=25%, V=50%, W=75%
 * - Runtime: every call steps each duty by +10% with wrap to 0% if > 100%
 * - Min pulse policy: allow 0%/100% (no clamping)
 *
 * Notes:
 * - Watchdog handling MUST NOT be here (Cpu0_Main.c only).
 * - Pin routing is handled by IfxGtm_Pwm via OutputConfig; do not call PinMap set APIs.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* =============================
 * Requirements-driven constants
 * ============================= */
#define TIMING_PWM_FREQUENCY_HZ          (20000.0f)     /* 20 kHz */
#define TIMING_DEAD_TIME_US              (0.5f)         /* 0.5 microseconds */
#define PWM_MIN_PULSE_TIME               (0.0f)         /* Allow 0% / 100% */

/* Runtime behavior macros (percent representation) */
#define DUTY_25_PERCENT                  (25.0f)
#define DUTY_50_PERCENT                  (50.0f)
#define DUTY_75_PERCENT                  (75.0f)
#define DUTY_STEP                        (10.0f)
#define DUTY_MAX_PERCENT                 (100.0f)

#define NUM_OF_CHANNELS                  (3u)

/* =============================
 * TOM1 complementary pin macros
 * P00.2..P00.7 (validated for TC387 reference board family)
 * ============================= */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* =============================
 * Driver/application state
 * ============================= */
typedef struct
{
    IfxGtm_Pwm          pwm;                                /* PWM Driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];          /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];        /* Duty cycle values in percent */
    float32             phases[NUM_OF_CHANNELS];            /* Phase shift values (not used, keep 0) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];         /* Dead time values (for bookkeeping) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;
static boolean s_initialized = FALSE;

/* =============================
 * Local helpers
 * ============================= */
static float32 GTM_TOM_3_Phase_Inverter_PWM_wrapDuty(float32 duty)
{
    /* Inclusive 0..100 wrap-around: if exceeds 100.0, wrap to 0.0 */
    if (duty > DUTY_MAX_PERCENT)
    {
        return 0.0f;
    }
    return duty;
}

/* =============================
 * Public API
 * ============================= */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* 1) Enable GTM and set CMU clocks (mandatory for TOM PWM operation) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        float32 modFreq;
        IfxGtm_enable(&MODULE_GTM);
        modFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, modFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, modFreq);
        /* Enable FXCLK and CLK0 for TOM */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Build unified IfxGtm_Pwm configuration for 3 complementary channels */
    IfxGtm_Pwm_Config          config;                /* Intentionally set fields explicitly (no initConfig in contract) */
    IfxGtm_Pwm_ChannelConfig   channelCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    outputCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmCfg[NUM_OF_CHANNELS];

    /* 2a) Output pin routing via OutputConfig (no direct PinMap API calls!) */
    /* Phase U */
    outputCfg[0].pin                       = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    outputCfg[0].complementaryPin          = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    outputCfg[0].polarity                  = Ifx_ActiveState_high;
    outputCfg[0].complementaryPolarity     = Ifx_ActiveState_low;
    outputCfg[0].outputMode                = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase V */
    outputCfg[1].pin                       = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    outputCfg[1].complementaryPin          = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    outputCfg[1].polarity                  = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity     = Ifx_ActiveState_low;
    outputCfg[1].outputMode                = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase W */
    outputCfg[2].pin                       = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    outputCfg[2].complementaryPin          = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    outputCfg[2].polarity                  = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity     = Ifx_ActiveState_low;
    outputCfg[2].outputMode                = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 2b) Dead-time configuration (seconds). Requirement: 0.5 us */
    {
        const float32 deadTimeS = (TIMING_DEAD_TIME_US * 1.0e-6f);
        dtmCfg[0].deadTime = deadTimeS;
        dtmCfg[1].deadTime = deadTimeS;
        dtmCfg[2].deadTime = deadTimeS;
    }

    /* 2c) Per-channel configuration: duty in percent, 0 phase, link dtm/output */
    /* Channel 0: Phase U */
    channelCfg[0].phase     = 0.0f;
    channelCfg[0].duty      = DUTY_25_PERCENT;  /* Will be updated immediately below as well */
    channelCfg[0].dtm       = &dtmCfg[0];
    channelCfg[0].output    = &outputCfg[0];
    channelCfg[0].mscOut    = NULL_PTR;
    channelCfg[0].interrupt = NULL_PTR;         /* No ISR binding in this module */
    /* Channel 1: Phase V */
    channelCfg[1].phase     = 0.0f;
    channelCfg[1].duty      = DUTY_50_PERCENT;
    channelCfg[1].dtm       = &dtmCfg[1];
    channelCfg[1].output    = &outputCfg[1];
    channelCfg[1].mscOut    = NULL_PTR;
    channelCfg[1].interrupt = NULL_PTR;
    /* Channel 2: Phase W */
    channelCfg[2].phase     = 0.0f;
    channelCfg[2].duty      = DUTY_75_PERCENT;
    channelCfg[2].dtm       = &dtmCfg[2];
    channelCfg[2].output    = &outputCfg[2];
    channelCfg[2].mscOut    = NULL_PTR;
    channelCfg[2].interrupt = NULL_PTR;

    /* 2d) Top-level PWM config */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;   /* Center-aligned */
    config.syncStart            = TRUE;                           /* Start channels after init */
    config.syncUpdateEnabled    = TRUE;                           /* Shadow update */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelCfg[0];
    config.frequency            = TIMING_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;             /* TOM uses Fxclk enum */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 3) Initialize the PWM driver (unified high-level) */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 4) Store initial runtime state and apply initial duty immediately (synchronized) */
    g_gtmTom3phInv.dutyCycles[0] = DUTY_25_PERCENT;
    g_gtmTom3phInv.dutyCycles[1] = DUTY_50_PERCENT;
    g_gtmTom3phInv.dutyCycles[2] = DUTY_75_PERCENT;
    g_gtmTom3phInv.phases[0]     = 0.0f;
    g_gtmTom3phInv.phases[1]     = 0.0f;
    g_gtmTom3phInv.phases[2]     = 0.0f;
    g_gtmTom3phInv.deadTimes[0]  = dtmCfg[0].deadTime;
    g_gtmTom3phInv.deadTimes[1]  = dtmCfg[1].deadTime;
    g_gtmTom3phInv.deadTimes[2]  = dtmCfg[2].deadTime;

    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);

    /* syncStart=TRUE in config starts the synchronized channels after init */

    s_initialized = TRUE;
}

void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void)
{
    if (s_initialized == FALSE)
    {
        return;
    }

    /* Increment each duty by +10%, wrap to 0.0 if exceeds 100.0 */
    g_gtmTom3phInv.dutyCycles[0] = GTM_TOM_3_Phase_Inverter_PWM_wrapDuty(g_gtmTom3phInv.dutyCycles[0] + DUTY_STEP);
    g_gtmTom3phInv.dutyCycles[1] = GTM_TOM_3_Phase_Inverter_PWM_wrapDuty(g_gtmTom3phInv.dutyCycles[1] + DUTY_STEP);
    g_gtmTom3phInv.dutyCycles[2] = GTM_TOM_3_Phase_Inverter_PWM_wrapDuty(g_gtmTom3phInv.dutyCycles[2] + DUTY_STEP);

    /* Apply synchronized immediate update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);
}
