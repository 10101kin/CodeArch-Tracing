/*
 * File: gtm_tom_3_phase_inverter_pwm.c
 * Description: GTM TOM 3-Phase Inverter PWM - Implementation per SW Detailed Design
 * Target: TC3xx (TC387)
 * Notes:
 *  - Uses unified IfxGtm_Pwm high-level driver and explicit PinMap routing as required
 *  - No watchdog API is called here (must be handled only in CpuN_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD headers (generic) */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* =============================
 * Requirements-driven constants
 * ============================= */
#define NUM_OF_CHANNELS                     (3u)

/* Timing and behavior (from requirements) */
#define PWM_FREQUENCY_HZ                    (20000.0f)     /* 20 kHz */
#define PWM_DEAD_TIME_S                     (5.0e-7f)      /* 0.5 us */
#define PWM_MIN_PULSE_S                     (1.0e-6f)      /* 1.0 us (informational) */

/* Initial duty cycles in percent (unified driver expects percent) */
#define DUTY_25_PERCENT                     (25.0f)
#define DUTY_50_PERCENT                     (50.0f)
#define DUTY_75_PERCENT                     (75.0f)

/* Runtime step and bounds (reference behavior pattern) */
#define DUTY_STEP_PERCENT                   (10.0f)        /* step in % */
#define DUTY_MIN_PERCENT                    (10.0f)        /* min in % */
#define DUTY_MAX_PERCENT                    (90.0f)        /* max in % */

/* Output configuration (from requirements) */
#define OUTPUT_MODE                         IfxPort_OutputMode_pushPull
#define OUTPUT_PAD_DRIVER                   IfxPort_PadDriver_cmosAutomotiveSpeed1
#define OUTPUT_CCX_ACTIVE_STATE             Ifx_ActiveState_high
#define OUTPUT_COUTX_ACTIVE_STATE           Ifx_ActiveState_high

/* Master timer requirements (informational for configuration mapping) */
#define MASTER_TIMER_TOM                    IfxGtm_Pwm_SubModule_tom
#define MASTER_TIMER_CHANNEL_CH0            IfxGtm_Pwm_SubModule_Ch_0   /* TOM1_CH0 used as time base only */
#define MASTER_TIMER_CLOCK_SOURCE           IfxGtm_Cmu_Fxclk_0          /* Fxclk0 per requirements */

/* =====================
 * Validated pin mapping
 * =====================
 * KIT_A2G_TC387_5V_TFT mapping (reference-consistent):
 *  - HS: P00.3 (TOM1_2), P00.5 (TOM1_4), P00.7 (TOM1_6)
 *  - LS: P00.2 (TOM1_1), P00.4 (TOM1_3), P00.6 (TOM1_5)
 */
#define PHASE_U_HS      (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS      (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS      (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS      (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS      (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS      (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* =========================
 * Driver state and handles
 * ========================= */

typedef struct
{
    IfxGtm_Pwm          pwm;                                 /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];           /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];          /* Duty cycle values (percent) */
    float32             phases[NUM_OF_CHANNELS];              /* Phase shift values (not used here) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];          /* Dead-time values (s) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv; /* Module-private driver instance */

/* =============================================
 * Private helper: update duty vector (wrap logic)
 * ============================================= */
static void GTM_TOM_3_Phase_Inverter_PWM_updateDutyArrayInternal(void)
{
    uint8 i;
    for (i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        float32 next = g_gtmTom3phInv.dutyCycles[i] + DUTY_STEP_PERCENT;
        if (next >= DUTY_MAX_PERCENT)
        {
            next = DUTY_MIN_PERCENT; /* wrap to min bound */
        }
        g_gtmTom3phInv.dutyCycles[i] = next;
    }
}

/* ================================================================
 * Public API: Initialization per SW Detailed Design (init context)
 * ================================================================ */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* 1) Enable GTM module and configure CMU clocks required by TOM */
    IfxGtm_enable(&MODULE_GTM);

    /* Set GCLK and CLK0 to module frequency and enable FXCLK + CLK0 */
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFreq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Assign six output pins (complementary pairs) via generic PinMap API */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);

    /* 3) Prepare unified PWM configuration with complementary outputs */
    {
        IfxGtm_Pwm_Config        config;
        IfxGtm_Pwm_ChannelConfig channelCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig  outputCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_DtmConfig     dtmCfg[NUM_OF_CHANNELS];
        uint8                    i;

        /* Initialize top-level config with defaults */
        IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

        /* Output configuration: three complementary pairs (U,V,W) */
        outputCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        outputCfg[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        outputCfg[0].polarity              = OUTPUT_CCX_ACTIVE_STATE;
        outputCfg[0].complementaryPolarity = OUTPUT_COUTX_ACTIVE_STATE;
        outputCfg[0].outputMode            = OUTPUT_MODE;
        outputCfg[0].padDriver             = OUTPUT_PAD_DRIVER;

        outputCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        outputCfg[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        outputCfg[1].polarity              = OUTPUT_CCX_ACTIVE_STATE;
        outputCfg[1].complementaryPolarity = OUTPUT_COUTX_ACTIVE_STATE;
        outputCfg[1].outputMode            = OUTPUT_MODE;
        outputCfg[1].padDriver             = OUTPUT_PAD_DRIVER;

        outputCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        outputCfg[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        outputCfg[2].polarity              = OUTPUT_CCX_ACTIVE_STATE;
        outputCfg[2].complementaryPolarity = OUTPUT_COUTX_ACTIVE_STATE;
        outputCfg[2].outputMode            = OUTPUT_MODE;
        outputCfg[2].padDriver             = OUTPUT_PAD_DRIVER;

        /* Dead-time config for all complementary pairs */
        for (i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            dtmCfg[i].deadTime.rising  = PWM_DEAD_TIME_S;
            dtmCfg[i].deadTime.falling = PWM_DEAD_TIME_S;
        }

        /* Channel configurations (map to TOM1 channels: 1, 3, 5) */
        for (i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            IfxGtm_Pwm_initChannelConfig(&channelCfg[i]);
            channelCfg[i].phase     = 0.0f;            /* no phase shift */
            channelCfg[i].dtm       = &dtmCfg[i];      /* link DTM */
            channelCfg[i].output    = &outputCfg[i];   /* link outputs */
            channelCfg[i].interrupt = NULL_PTR;        /* no ISR for this design */
        }

        /* Explicitly select TOM1 channel indices for U,V,W base channels */
        channelCfg[0].timerCh = IfxGtm_Pwm_SubModule_Ch_1; /* U: TOM1_CH1/TOM1_CH2 pair */
        channelCfg[1].timerCh = IfxGtm_Pwm_SubModule_Ch_3; /* V: TOM1_CH3/TOM1_CH4 pair */
        channelCfg[2].timerCh = IfxGtm_Pwm_SubModule_Ch_5; /* W: TOM1_CH5/TOM1_CH6 pair */

        /* Initial duty (percent) */
        channelCfg[0].duty = DUTY_25_PERCENT; /* U */
        channelCfg[1].duty = DUTY_50_PERCENT; /* V */
        channelCfg[2].duty = DUTY_75_PERCENT; /* W */

        /* Top-level PWM config fields for center-aligned complementary TOM */
        config.cluster               = IfxGtm_Cluster_0;
        config.subModule             = IfxGtm_Pwm_SubModule_tom;
        config.alignment             = IfxGtm_Pwm_Alignment_center; /* center-aligned mode */
        config.syncStart             = TRUE;                        /* start channels after init */
        config.syncUpdateEnabled     = TRUE;                        /* synchronous shadow updates */
        config.numChannels           = NUM_OF_CHANNELS;
        config.channels              = &channelCfg[0];
        config.frequency             = PWM_FREQUENCY_HZ;            /* 20 kHz */
        config.clockSource.tom       = MASTER_TIMER_CLOCK_SOURCE;   /* Fxclk0 */
        config.dtmClockSource        = IfxGtm_Dtm_ClockSource_cmuClock0;

        /* Initialize PWM driver with channels array */
        IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

        /* 4) Program frequency immediately (explicit per SW design) */
        IfxGtm_Pwm_updateFrequencyImmediate(&g_gtmTom3phInv.pwm, PWM_FREQUENCY_HZ);

        /* 5) Configure a uniform hardware dead-time across all pairs (explicit) */
        {
            IfxGtm_Pwm_DeadTime reqDead[NUM_OF_CHANNELS];
            for (i = 0u; i < NUM_OF_CHANNELS; i++)
            {
                reqDead[i].rising  = PWM_DEAD_TIME_S;
                reqDead[i].falling = PWM_DEAD_TIME_S;
            }
            IfxGtm_Pwm_updateChannelsDeadTimeImmediate(&g_gtmTom3phInv.pwm, &reqDead[0]);
        }

        /* 6) Prepare initial duty vector and apply in one synchronous update */
        g_gtmTom3phInv.dutyCycles[0] = DUTY_25_PERCENT; /* U */
        g_gtmTom3phInv.dutyCycles[1] = DUTY_50_PERCENT; /* V */
        g_gtmTom3phInv.dutyCycles[2] = DUTY_75_PERCENT; /* W */
        IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);

        /* 7) Start synchronized channels and enable outputs explicitly */
        IfxGtm_Pwm_startSyncedChannels(&g_gtmTom3phInv.pwm);
        IfxGtm_Pwm_startChannelOutputs(&g_gtmTom3phInv.pwm);

        /* Store current dead-time values for potential runtime use */
        g_gtmTom3phInv.deadTimes[0] = dtmCfg[0].deadTime;
        g_gtmTom3phInv.deadTimes[1] = dtmCfg[1].deadTime;
        g_gtmTom3phInv.deadTimes[2] = dtmCfg[2].deadTime;
    }
}

/* ======================================================
 * Public API: Runtime duty update (loop context)
 * ====================================================== */
void GTM_TOM_3_Phase_Inverter_PWM_updateUVW(void)
{
    /* Update U/V/W duties using fixed step with wrap between bounds */
    GTM_TOM_3_Phase_Inverter_PWM_updateDutyArrayInternal();

    /* Single synchronous duty update across all channels */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
}
