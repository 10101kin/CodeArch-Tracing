/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief GTM TOM 3-Phase Inverter PWM driver (TC3xx, unified IfxGtm_Pwm)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm_Pwm.h"      /* Unified GTM PWM driver */
#include "IfxGtm_PinMap.h"   /* Generic pin map */
#include "IfxGtm_Cmu.h"      /* CMU clock configuration */
#include "IfxGtm.h"          /* GTM module enable */
#include "IfxPort.h"         /* Port/pad configuration */

/* ========================= Requirements mapping (constants) ========================= */
#define NUM_OF_CHANNELS                (3u)

/* Timing requirements */
#define PWM_FREQUENCY_HZ               (20000.0f)          /* TIMING_PWM_FREQUENCY_HZ = 20 kHz */
#define DEADTIME_SECONDS               (0.5e-6f)           /* TIMING_DEADTIME_US = 0.5 us */
#define MIN_PULSE_SECONDS              (1.0e-6f)           /* TIMING_MIN_PULSE_US = 1.0 us (constraint) */

/* Initial duties (percent representation per unified driver expectations) */
#define DUTY_25_PERCENT                (25.0f)             /* INITIAL_DUTY_PERCENT_U */
#define DUTY_50_PERCENT                (50.0f)             /* INITIAL_DUTY_PERCENT_V */
#define DUTY_75_PERCENT                (75.0f)             /* INITIAL_DUTY_PERCENT_W */

/* Runtime ramp parameters (percent) */
#define DUTY_STEP                      (10.0f)             /* Step per update call */
#define DUTY_MIN                       (10.0f)             /* Lower threshold percent */
#define DUTY_MAX                       (90.0f)             /* Upper threshold percent */

/* Master timebase selections (requirements) */
#define MASTER_TIMEBASE_TOM_MODULE     (1u)                /* TOM1 */
#define MASTER_TIMEBASE_CHANNEL        (0u)                /* CH0 */
/* MASTER_TIMEBASE_CLOCK = cmuFxclk0; MASTER_TIMEBASE_CLOCK_SOURCE = GCLK */

/* ========================= Pin mapping ========================= */
/*
 * NOTE: For TC387, valid TOM1 routes are available on Port 00 (reference-verified).
 * The following mappings provide three complementary pairs on TOM1:
 *   - Phase U: HS=TOM1_2 -> TOUT12 P00.3, LS=TOM1_1 -> TOUT11 P00.2
 *   - Phase V: HS=TOM1_4 -> TOUT14 P00.5, LS=TOM1_3 -> TOUT13 P00.4
 *   - Phase W: HS=TOM1_6 -> TOUT16 P00.7, LS=TOM1_5 -> TOUT15 P00.6
 * These pins are generic-header symbols (IfxGtm_PinMap.h) and valid on TC3xx.
 */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ========================= Driver context ========================= */
typedef struct
{
    IfxGtm_Pwm          pwm;                                  /* PWM Driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];            /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];          /* Duty cycle values (percent) */
    float32             phases[NUM_OF_CHANNELS];              /* Phase shift values (deg or percent phase), unused here */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];           /* Dead-time values (seconds) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;
static boolean      s_initialized = FALSE;

/* ========================= Local helpers ========================= */
static void gtmTom3phInv_buildOutputConfig(IfxGtm_Pwm_OutputConfig output[NUM_OF_CHANNELS])
{
    /* Phase U */
    output[0].pin                      = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin         = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                 = Ifx_ActiveState_high;
    output[0].complementaryPolarity    = Ifx_ActiveState_low;   /* Inverter complementary */
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                      = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin         = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_low;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                      = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin         = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_low;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;
}

/* ========================= Public API ========================= */
void GTM_TOM_3PhaseInverterPWM_init(void)
{
    IfxGtm_Pwm_Config          config;
    IfxGtm_Pwm_ChannelConfig   channelCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmCfg[NUM_OF_CHANNELS];

    /* 1) Enable GTM and configure CMU clocks (use GCLK as source, enable FXCLK and CLK0) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
    }
    /* Derive GCLK from module frequency (requirement: CLOCK_EXPECTED_GTM_GCLK_MHZ = 100 MHz typical) */
    IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM));
    /* Route GCLK into CLK0 (MASTER_TIMEBASE_CLOCK_SOURCE = GCLK) */
    IfxGtm_Cmu_selectClkInput(&MODULE_GTM, IfxGtm_Cmu_Clk_0, TRUE);
    /* Set CLK0 = GCLK frequency (MASTER_TIMEBASE_CLOCK = cmuFxclk0 uses CLK0 domain) */
    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
    /* Enable FXCLK and CLK0 for TOM operation */
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));

    /* 2) Initialize unified PWM config */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration via unified driver (no direct PinMap calls) */
    gtmTom3phInv_buildOutputConfig(output);

    /* 4) Dead-time configuration per channel (seconds) */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
    {
        dtmCfg[i].deadTime.rising  = DEADTIME_SECONDS;
        dtmCfg[i].deadTime.falling = DEADTIME_SECONDS;
    }

    /* 5) Channel configuration (TOM1 CH0 master timebase, center-aligned) */
    /* Channel 0: Phase U */
    IfxGtm_Pwm_initChannelConfig(&channelCfg[0]);
    channelCfg[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;       /* MASTER_TIMEBASE_CHANNEL = 0 */
    channelCfg[0].phase     = 0.0f;
    channelCfg[0].duty      = DUTY_25_PERCENT;
    channelCfg[0].dtm       = &dtmCfg[0];
    channelCfg[0].output    = &output[0];
    channelCfg[0].mscOut    = NULL_PTR;
    channelCfg[0].interrupt = NULL_PTR;                        /* No ISR required */

    /* Channel 1: Phase V */
    IfxGtm_Pwm_initChannelConfig(&channelCfg[1]);
    channelCfg[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelCfg[1].phase     = 0.0f;
    channelCfg[1].duty      = DUTY_50_PERCENT;
    channelCfg[1].dtm       = &dtmCfg[1];
    channelCfg[1].output    = &output[1];
    channelCfg[1].mscOut    = NULL_PTR;
    channelCfg[1].interrupt = NULL_PTR;

    /* Channel 2: Phase W */
    IfxGtm_Pwm_initChannelConfig(&channelCfg[2]);
    channelCfg[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelCfg[2].phase     = 0.0f;
    channelCfg[2].duty      = DUTY_75_PERCENT;
    channelCfg[2].dtm       = &dtmCfg[2];
    channelCfg[2].output    = &output[2];
    channelCfg[2].mscOut    = NULL_PTR;
    channelCfg[2].interrupt = NULL_PTR;

    /* 6) Top-level PWM configuration */
    config.cluster               = IfxGtm_Cluster_0;                 /* GTM Cluster 0 */
    config.subModule             = IfxGtm_Pwm_SubModule_tom;         /* TOM */
    config.alignment             = IfxGtm_Pwm_Alignment_center;      /* Center-aligned */
    config.syncStart             = TRUE;                              /* Auto-start on init */
    config.syncUpdateEnabled     = TRUE;                              /* Shadow register update */
    config.numChannels           = NUM_OF_CHANNELS;
    config.channels              = &channelCfg[0];
    config.frequency             = PWM_FREQUENCY_HZ;
    config.clockSource.tom       = IfxGtm_Cmu_Fxclk_0;                /* MASTER_TIMEBASE_CLOCK = cmuFxclk0 */
    config.dtmClockSource        = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM clock source */

    /* 7) Initialize the unified PWM driver */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 8) Store initial values for runtime */
    g_gtmTom3phInv.dutyCycles[0] = channelCfg[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelCfg[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelCfg[2].duty;

    g_gtmTom3phInv.deadTimes[0]  = dtmCfg[0].deadTime;
    g_gtmTom3phInv.deadTimes[1]  = dtmCfg[1].deadTime;
    g_gtmTom3phInv.deadTimes[2]  = dtmCfg[2].deadTime;

    /* 9) Enable PWM channel outputs */
    IfxGtm_Pwm_startChannelOutputs(&g_gtmTom3phInv.pwm);

    s_initialized = TRUE;
}

void GTM_TOM_3PhaseInverterPWM_updateDuties(void)
{
    /* Early-exit if not initialized (safety for TDD/error handling) */
    if (s_initialized == FALSE)
    {
        return;
    }

    /* Ramp duties with wrap-around between DUTY_MIN and DUTY_MAX */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
    {
        g_gtmTom3phInv.dutyCycles[i] += DUTY_STEP;
        if (g_gtmTom3phInv.dutyCycles[i] >= DUTY_MAX)
        {
            g_gtmTom3phInv.dutyCycles[i] = DUTY_MIN;
        }
    }

    /* Apply updated duty set synchronously using unified driver (shadow update) */
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
}
