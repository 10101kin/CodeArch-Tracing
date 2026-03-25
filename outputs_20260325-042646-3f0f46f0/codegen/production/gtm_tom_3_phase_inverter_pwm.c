#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"

/*
 * Driver/application state
 */
typedef struct
{
    IfxGtm_Pwm          pwm;                                 /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];           /* Channel runtime handles */
    float32             dutyCycles[NUM_OF_CHANNELS];         /* Duty in percent [0..100) */
    float32             phases[NUM_OF_CHANNELS];             /* Phase shift (not used) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];          /* Dead-time (unused now) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;                          /* Module-local instance */
static boolean       s_initialized = FALSE;                   /* Init guard */

/*
 * Private helper: configure TOM output pins using generic PinMap and
 * set unused complementary pins to a safe GPIO state (push-pull, low)
 */
static void configureTomPins(void)
{
    /* Route TOM1 HS outputs to pins via PinMap API */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* Unused complementary outputs: drive low as safe state */
    IfxPort_setPinModeOutput(&MODULE_P00, 2U, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general); /* U_LS (P00.2) */
    IfxPort_setPinLow(&MODULE_P00, 2U);

    IfxPort_setPinModeOutput(&MODULE_P00, 4U, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general); /* V_LS (P00.4) */
    IfxPort_setPinLow(&MODULE_P00, 4U);

    IfxPort_setPinModeOutput(&MODULE_P00, 6U, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general); /* W_LS (P00.6) */
    IfxPort_setPinLow(&MODULE_P00, 6U);
}

/*
 * Initialize GTM TOM PWM for 3-phase inverter using unified IfxGtm_Pwm driver
 * - Enable GTM and FXCLK domain
 * - Configure pins via PinMap
 * - Build unified driver config for center-aligned 20 kHz, TOM1 channels
 * - Apply initial duties 25%/50%/75%
 * - Start synchronized channels
 */
void initGtmTomPwm(void)
{
    IfxGtm_Pwm_Config          config;
    IfxGtm_Pwm_ChannelConfig   channelCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    outputCfg[NUM_OF_CHANNELS];

    /* 1) Enable GTM and clocks (GCLK = module freq, CLK0 = GCLK, enable FXCLK) */
    IfxGtm_enable(&MODULE_GTM);
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Assign TOM outputs and safe-state unused complementary pins */
    configureTomPins();

    /* 3) Initialize main PWM config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 4) Build output configuration for HS pins only (no complementary) */
    outputCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;  /* U HS on TOM1_2 */
    outputCfg[0].complementaryPin      = NULL_PTR;
    outputCfg[0].polarity              = Ifx_ActiveState_high;
    outputCfg[0].complementaryPolarity = Ifx_ActiveState_low;
    outputCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    outputCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;  /* V HS on TOM1_4 */
    outputCfg[1].complementaryPin      = NULL_PTR;
    outputCfg[1].polarity              = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity = Ifx_ActiveState_low;
    outputCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    outputCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;  /* W HS on TOM1_6 */
    outputCfg[2].complementaryPin      = NULL_PTR;
    outputCfg[2].polarity              = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity = Ifx_ActiveState_low;
    outputCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) Initialize per-channel configuration and link outputs; TOM1 channels 2/4/6 */
    IfxGtm_Pwm_initChannelConfig(&channelCfg[0]);
    channelCfg[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;   /* TOM1 ch 2 */
    channelCfg[0].phase     = 0.0f;
    channelCfg[0].duty      = INITIAL_DUTY_PERCENT_U;
    channelCfg[0].dtm       = NULL_PTR;
    channelCfg[0].output    = &outputCfg[0];
    channelCfg[0].mscOut    = NULL_PTR;
    channelCfg[0].interrupt = NULL_PTR;

    IfxGtm_Pwm_initChannelConfig(&channelCfg[1]);
    channelCfg[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_4;   /* TOM1 ch 4 */
    channelCfg[1].phase     = 0.0f;
    channelCfg[1].duty      = INITIAL_DUTY_PERCENT_V;
    channelCfg[1].dtm       = NULL_PTR;
    channelCfg[1].output    = &outputCfg[1];
    channelCfg[1].mscOut    = NULL_PTR;
    channelCfg[1].interrupt = NULL_PTR;

    IfxGtm_Pwm_initChannelConfig(&channelCfg[2]);
    channelCfg[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_6;   /* TOM1 ch 6 */
    channelCfg[2].phase     = 0.0f;
    channelCfg[2].duty      = INITIAL_DUTY_PERCENT_W;
    channelCfg[2].dtm       = NULL_PTR;
    channelCfg[2].output    = &outputCfg[2];
    channelCfg[2].mscOut    = NULL_PTR;
    channelCfg[2].interrupt = NULL_PTR;

    /* 6) Configure top-level PWM parameters */
    config.cluster           = IfxGtm_Cluster_0;
    config.subModule         = IfxGtm_Pwm_SubModule_tom;         /* TOM */
    config.alignment         = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart         = TRUE;                              /* will also call start API explicitly */
    config.syncUpdateEnabled = TRUE;                              /* shadow transfer */
    config.numChannels       = NUM_OF_CHANNELS;
    config.channels          = &channelCfg[0];
    config.frequency         = PWM_FREQUENCY;
    config.clockSource.tom   = IfxGtm_Cmu_Fxclk_0;                /* TOM uses Fxclk enum */
    config.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0;  /* not used now */

    /* 7) Initialize driver, store runtime duty array, apply, and start */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    g_gtmTom3phInv.dutyCycles[0] = channelCfg[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelCfg[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelCfg[2].duty;

    /* Apply initial duties using unified API and start synced outputs */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
    IfxGtm_Pwm_startSyncedChannels(&g_gtmTom3phInv.pwm);

    s_initialized = TRUE;
}

/*
 * Runtime duty update: on each call, increment each duty by +10% and wrap within [0..100%),
 * ensuring exact 100% is avoided (wrap-before-add semantics prevents 100%).
 * Apply atomically via immediate multi-channel update.
 */
void updateGtmTomPwmDutyCycles(void)
{
    uint8 i;
    if (s_initialized == FALSE)
    {
        return; /* Early exit if not initialized */
    }

    for (i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        /* Wrap-before-add to avoid exact 100% */
        if ((g_gtmTom3phInv.dutyCycles[i] + UPDATE_POLICY_INCREMENT_PERCENT) >= 100.0f)
        {
            g_gtmTom3phInv.dutyCycles[i] = 0.0f;
        }
        g_gtmTom3phInv.dutyCycles[i] += UPDATE_POLICY_INCREMENT_PERCENT;
    }

    /* Apply updated duties atomically */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
}
