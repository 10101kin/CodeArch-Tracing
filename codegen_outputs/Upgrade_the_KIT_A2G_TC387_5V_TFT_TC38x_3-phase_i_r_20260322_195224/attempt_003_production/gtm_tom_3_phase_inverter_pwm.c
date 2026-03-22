/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production implementation using unified IfxGtm_Pwm per SW Detailed Design.
 * - Enables GTM and functional clocks
 * - Configures three complementary PWM pairs (TOM1) with center alignment
 * - Dead-time = 0.5 us, frequency = 20 kHz
 * - Initial duties: U=25%, V=50%, W=75%
 * - Runtime step: +10% with wrap-around 0..100%, synchronous update
 *
 * Notes:
 * - Watchdog handling must be done only in CpuN_Main.c files (not here).
 * - Uses only generic PinMap headers.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"

/* Internal driver state structure matching unified IfxGtm_Pwm usage */
typedef struct
{
    IfxGtm_Pwm          pwm;                                /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];          /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];         /* Duty cycle values (percent) */
    float32             phases[NUM_OF_CHANNELS];             /* Phase shift values (percent of period) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];         /* Dead-time bookkeeping */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;
static boolean s_initialized = FALSE;

/* Private helper: inclusive wrap-around across 0..100% */
static float32 GTM_TOM_3_Phase_Inverter_PWM_wrapDuty(float32 duty)
{
    return (duty > 100.0f) ? 0.0f : duty;
}

void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* Enable GTM and configure CMU clocks (FXCLK + CLK0) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        float32 moduleFreq;
        IfxGtm_enable(&MODULE_GTM);
        moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFreq);
        /* Enable functional clocks: FXCLK and CLK0 */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* Unified PWM configuration */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];

    /* Initialize configuration with defaults and module reference */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output configuration (complementary per phase) */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration (seconds) - requirement: 0.5 us */
    dtmConfig[0].deadTime = (TIMING_DEAD_TIME_US * 1.0e-6f);
    dtmConfig[1].deadTime = (TIMING_DEAD_TIME_US * 1.0e-6f);
    dtmConfig[2].deadTime = (TIMING_DEAD_TIME_US * 1.0e-6f);

    /* Per-channel configuration: TOM submodule, center-aligned, sync group */
    /* Channel 0 - Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = DUTY_25_PERCENT; /* U */
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR;        /* no ISR in this module */

    /* Channel 1 - Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = DUTY_50_PERCENT; /* V */
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Channel 2 - Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = DUTY_75_PERCENT; /* W */
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* Main PWM config */
    config.cluster           = IfxGtm_Cluster_0;
    config.subModule         = IfxGtm_Pwm_SubModule_tom;
    config.alignment         = IfxGtm_Pwm_Alignment_center;
    config.syncStart         = TRUE;                       /* start after init */
    config.syncUpdateEnabled = TRUE;                       /* shadow update */
    config.numChannels       = NUM_OF_CHANNELS;
    config.channels          = &channelConfig[0];
    config.frequency         = TIMING_PWM_FREQUENCY_HZ;
    config.clockSource.tom   = IfxGtm_Cmu_Fxclk_0;         /* TOM uses Fxclk enum */
    config.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* Initialize PWM driver */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* Store initial runtime state */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_gtmTom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;
    g_gtmTom3phInv.phases[0]     = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1]     = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2]     = channelConfig[2].phase;

    /* Apply initial duties immediately in a synchronized way */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);

    s_initialized = TRUE;
}

void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit on not-initialized per error-handling guidance */
    }

    /* Step all three duties by +10 percentage points with 0..100 wrap-around */
    g_gtmTom3phInv.dutyCycles[0] = GTM_TOM_3_Phase_Inverter_PWM_wrapDuty(g_gtmTom3phInv.dutyCycles[0] + DUTY_STEP_PERCENT);
    g_gtmTom3phInv.dutyCycles[1] = GTM_TOM_3_Phase_Inverter_PWM_wrapDuty(g_gtmTom3phInv.dutyCycles[1] + DUTY_STEP_PERCENT);
    g_gtmTom3phInv.dutyCycles[2] = GTM_TOM_3_Phase_Inverter_PWM_wrapDuty(g_gtmTom3phInv.dutyCycles[2] + DUTY_STEP_PERCENT);

    /* Synchronous immediate update across all channels */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);
}
