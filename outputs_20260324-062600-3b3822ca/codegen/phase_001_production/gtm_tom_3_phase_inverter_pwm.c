/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm (single-output)
 * Implements the API contract from SW Detailed Design.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm_Pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/*
 * Pin assignment per SW Detailed Design (single-ended outputs on P02.0/1/2)
 * Channels: TOM1 CH2 -> P02.0, TOM1 CH4 -> P02.1, TOM1 CH6 -> P02.2
 * NOTE: Using unified driver, pin routing is handled via OutputConfig; do not call PinMap APIs directly.
 */
#define PHASE_U_PIN    (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_V_PIN    (&IfxGtm_TOM1_4N_TOUT1_P02_1_OUT)
#define PHASE_W_PIN    (&IfxGtm_TOM1_10_TOUT2_P02_2_OUT)

/* Driver state structure */
typedef struct
{
    IfxGtm_Pwm          pwm;                               /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];         /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];       /* Duty cycle values in percent [0..100] */
    float32             phases[NUM_OF_CHANNELS];           /* Phase shift values in percent/deg (unused) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];        /* Dead-time values (unused: single-ended) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv; /* Static driver instance */

void initGtmTom3PhaseInverterPwm(void)
{
    /* 1) Enable GTM and required CMU clocks (module and functional clocks) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
    }

    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        /* Align GCLK to module frequency, set CLK0, and enable FXCLK/CLK0 */
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFreq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Prepare unified IfxGtm_Pwm configuration */
    IfxGtm_Pwm_Config          config;
    IfxGtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Global PWM config per requirements */
    config.cluster           = IfxGtm_Cluster_0;                   /* Cluster selection */
    config.subModule         = IfxGtm_Pwm_SubModule_tom;           /* TOM submodule */
    config.alignment         = IfxGtm_Pwm_Alignment_center;        /* Center-aligned */
    config.syncStart         = TRUE;                                /* Start channels together after init */
    config.syncUpdateEnabled = TRUE;                                /* Shadow updates */
    config.frequency         = TIMING_PWM_FREQUENCY_HZ;             /* 20 kHz */
    config.clockSource.tom   = IfxGtm_Cmu_Fxclk_0;                  /* Use CMU Fxclk0 */
    config.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0;    /* DTM clock (not used for single-ended) */

    /* 3) Output pin routing via OutputConfig array (single-ended, ignore complementary) */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_PIN;  /* TOM1 CH2 */
    output[0].complementaryPin      = NULL_PTR;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_PIN;  /* TOM1 CH4 */
    output[1].complementaryPin      = NULL_PTR;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_PIN;  /* TOM1 CH6 */
    output[2].complementaryPin      = NULL_PTR;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Configure channels: exactly TOM1 channel indices 2, 4, 6 */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_2; /* TOM1 CH2 */
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = 0.0f;                      /* Will be set after init via immediate update */
    channelConfig[0].dtm       = NULL_PTR;                  /* Single-ended */
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR;

    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_4; /* TOM1 CH4 */
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = 0.0f;
    channelConfig[1].dtm       = NULL_PTR;
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_6; /* TOM1 CH6 */
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = 0.0f;
    channelConfig[2].dtm       = NULL_PTR;
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    config.numChannels = NUM_OF_CHANNELS;
    config.channels    = &channelConfig[0];

    /* 5) Initialize PWM driver with prepared configuration */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 6) Start synchronized channels so they run in lock-step on the shared timebase */
    IfxGtm_Pwm_startSyncedChannels(&g_gtmTom3phInv.pwm);

    /* 7) Initialize duty values and apply immediately (single multi-channel update) */
    g_gtmTom3phInv.dutyCycles[0] = DUTY_INIT_U_PERCENT;
    g_gtmTom3phInv.dutyCycles[1] = DUTY_INIT_V_PERCENT;
    g_gtmTom3phInv.dutyCycles[2] = DUTY_INIT_W_PERCENT;

    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);
}

void updateGtmTom3PhaseDuty(void)
{
    /* Increment each duty by 10%; if reach/exceed 100%, wrap to 0% */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_gtmTom3phInv.dutyCycles[i] += TIMING_DUTY_STEP_PERCENT;
        if (g_gtmTom3phInv.dutyCycles[i] >= 100.0f)
        {
            g_gtmTom3phInv.dutyCycles[i] = 0.0f;
        }
    }

    /* Apply immediately as a single multi-channel duty update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);
}
