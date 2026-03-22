#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"   /* Generic PinMap header (family-agnostic) */
#include "IfxPort.h"
#include "IfxCpu_Irq.h"
#include "IfxSrc.h"
#include "IfxGtm.h"

/* =====================================================================================
 * Requirements-driven configuration values
 * ===================================================================================== */
#define TIMING_PWM_FREQUENCY_HZ   (20000.0f)   /* 20 kHz */
#define TIMING_DEAD_TIME_US       (0.5f)       /* 0.5 microseconds */

/* Allow 0%/100% operation: set minimum pulse policy to zero (handled via config policy). */
#define PWM_MIN_PULSE_TIME        (0.0f)

/* Initial duties (percent representation, not fraction) and step */
#define DUTY_25_PERCENT           (25.0f)
#define DUTY_50_PERCENT           (50.0f)
#define DUTY_75_PERCENT           (75.0f)
#define DUTY_STEP                 (10.0f)

/* Channel count for 3-phase inverter */
#define NUM_OF_CHANNELS           (3u)

/* ISR priority macro name must match TOM reference naming */
#define ISR_PRIORITY_TOM          (20)

/* LED for ISR diagnostics (TC3xx example: P13.0) */
#define LED                       &MODULE_P13, 0

/* =====================================================================================
 * Validated TOM1 pin assignments on P00.2 .. P00.7 (keep TOM1 pins per user request)
 * Map 3 complementary pairs: U, V, W (HS = ccx, LS = coutx/complementary)
 * ===================================================================================== */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* =====================================================================================
 * Driver/application state
 * ===================================================================================== */
typedef struct
{
    IfxGtm_Pwm          pwm;                               /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];         /* Channel handles populated by init */
    float32             dutyCycles[NUM_OF_CHANNELS];       /* Duty cycle values (percent) */
    float32             phases[NUM_OF_CHANNELS];           /* Phase shift values (not used here) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];        /* Dead-time values (if queried later) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;

/* =====================================================================================
 * Period event callback (optional)
 * ===================================================================================== */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =====================================================================================
 * TOM period ISR (diagnostic LED toggle)
 * ===================================================================================== */
IFX_INTERRUPT(interruptGtmTom, 0, ISR_PRIORITY_TOM);
void interruptGtmTom(void)
{
    IfxPort_togglePin(LED);
}

/* =====================================================================================
 * Local helper - wrap 0..100 inclusive range
 * ===================================================================================== */
static float32 GTM_TOM_3_Phase_Inverter_PWM_wrapDuty(float32 duty)
{
    return (duty > 100.0f) ? 0.0f : duty;
}

/* =====================================================================================
 * Public API - Initialization
 * ===================================================================================== */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* -----------------------------------------------------------------------------
     * Mandatory GTM enable + CMU clock setup (TC3xx/GTM)
     * ----------------------------------------------------------------------------- */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        float32 frequency;
        IfxGtm_enable(&MODULE_GTM);
        frequency = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, frequency);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, frequency);
        /* Enable FXCLK and CLK0 (mask is device-specific; keep both enabled) */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* -----------------------------------------------------------------------------
     * Unified PWM configuration using IfxGtm_Pwm (high-level driver)
     * ----------------------------------------------------------------------------- */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Interrupt configuration (bind to channel 0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_TOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Output pin routing via OutputConfig - complementary polarity for LS */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time (seconds). Requirement: 0.5 us */
    {
        const float32 deadTime_s = (TIMING_DEAD_TIME_US * 1.0e-6f);
        dtmConfig[0].deadTime = deadTime_s;
        dtmConfig[1].deadTime = deadTime_s;
        dtmConfig[2].deadTime = deadTime_s;
    }

    /* Per-channel configuration (3 synchronized channels) */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0; /* TOM channel index group */
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = DUTY_25_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig; /* Period ISR on base channel */

    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = DUTY_50_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = DUTY_75_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* Core PWM configuration fields */
    config.cluster           = IfxGtm_Cluster_0;
    config.subModule         = IfxGtm_Pwm_SubModule_tom;
    config.alignment         = IfxGtm_Pwm_Alignment_center;  /* center-aligned */
    config.syncStart         = TRUE;                          /* auto-start after init */
    config.syncUpdateEnabled = TRUE;                          /* period-synced shadow updates */
    config.numChannels       = NUM_OF_CHANNELS;
    config.channels          = &channelConfig[0];
    config.frequency         = TIMING_PWM_FREQUENCY_HZ;
    config.clockSource.tom   = IfxGtm_Cmu_Fxclk_0;            /* TOM uses Fxclk source */
    config.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0;
    /* Explicit 0%/100% allowance: min pulse policy is application-side = 0.0f. */

    /* Initialize PWM driver (no return value per unified driver contract) */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* Store runtime state for duty/dead-time */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.deadTimes[0].deadTime = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1].deadTime = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2].deadTime = dtmConfig[2].deadTime;

    /* Configure LED output for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =====================================================================================
 * Public API - Runtime duty stepping (+10%, wrap >100% to 0%)
 * ===================================================================================== */
void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void)
{
    /* Algorithm per SW Detailed Design: increment, wrap, then apply synchronized update */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_gtmTom3phInv.dutyCycles[i] += DUTY_STEP;
        g_gtmTom3phInv.dutyCycles[i]  = GTM_TOM_3_Phase_Inverter_PWM_wrapDuty(g_gtmTom3phInv.dutyCycles[i]);
    }

    /* Synchronized immediate duty update for all channels */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);
}
