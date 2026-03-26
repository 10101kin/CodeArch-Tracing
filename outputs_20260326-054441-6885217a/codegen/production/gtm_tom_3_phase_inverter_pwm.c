/*
 * GTM_TOM_3_Phase_Inverter_PWM.c
 * Production code implementing 3-phase inverter PWM using IfxGtm_Pwm unified API
 *
 * Requirements satisfied:
 *  - TC3xx family (TC387)
 *  - GTM TOM-based complementary PWM with dead-time and min pulse
 *  - Center-aligned operation, 20 kHz, synchronous updates
 *  - Pins: KIT_A2G_TC387_5V_TFT mapping on P00.x as specified
 *
 * Watchdog note: No watchdog API usage in this driver (handled only in CpuN_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD includes (generic headers only) */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* =====================================================================
 * Validated pin mapping for KIT_A2G_TC387_5V_TFT (TOM1 complementary pairs)
 * HS: P00.3 (TOM1_2), P00.5 (TOM1_4), P00.7 (TOM1_6)
 * LS: P00.2 (TOM1_1), P00.4 (TOM1_3), P00.6 (TOM1_5)
 * ===================================================================== */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* =====================================================================
 * Driver/application state
 * ===================================================================== */

typedef struct
{
    IfxGtm_Pwm           pwm;                                 /* PWM driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];           /* Populated by init */
    float32              dutyCycles[NUM_OF_CHANNELS];          /* Duty in percent */
} GTM_TOM_3Ph_Driver;

static GTM_TOM_3Ph_Driver s_drv;
static boolean            s_initialized = FALSE;

/* Private helper (prototype) */
static void GTM_TOM_3_Phase_Inverter_PWM_updateDutyArrayInternal(void);

/* =====================================================================
 * Public functions (exact signatures from SW Detailed Design)
 * ===================================================================== */

/*
 * Enable GTM and clocks, configure unified PWM (IfxGtm_Pwm) for TOM1 complementary outputs,
 * set 20 kHz center-aligned mode, program dead-time and initial duties, then start outputs.
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* 1) Enable GTM and functional clocks (FXCLK) for TOM operation */
    IfxGtm_enable(&MODULE_GTM);

    /* Set GCLK and CLK0 to module frequency; enable FXCLK clock domain */
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFreq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Assign six output pins using the generic PinMap API (push-pull, specified pad driver) */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, OUTPUT_PORT_OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, OUTPUT_PORT_OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, OUTPUT_PORT_OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, OUTPUT_PORT_OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, OUTPUT_PORT_OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, OUTPUT_PORT_OUTPUT_MODE, OUTPUT_PAD_DRIVER);

    /* 3) Build unified PWM configuration and per-channel configuration */
    {
        IfxGtm_Pwm_Config         config;
        IfxGtm_Pwm_ChannelConfig  chCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig   outCfg[NUM_OF_CHANNELS];

        /* Initialize config with defaults for GTM module */
        IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

        /* Complementary output configuration per phase (active high on both) */
        /* Phase U on index 0 */
        outCfg[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        outCfg[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        outCfg[0].polarity               = Ifx_ActiveState_high;   /* CCX active high */
        outCfg[0].complementaryPolarity  = Ifx_ActiveState_high;   /* COUTX active high (per requirement) */
        outCfg[0].outputMode             = OUTPUT_PORT_OUTPUT_MODE;
        outCfg[0].padDriver              = OUTPUT_PAD_DRIVER;
        /* Phase V on index 1 */
        outCfg[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        outCfg[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        outCfg[1].polarity               = Ifx_ActiveState_high;
        outCfg[1].complementaryPolarity  = Ifx_ActiveState_high;
        outCfg[1].outputMode             = OUTPUT_PORT_OUTPUT_MODE;
        outCfg[1].padDriver              = OUTPUT_PAD_DRIVER;
        /* Phase W on index 2 */
        outCfg[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        outCfg[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        outCfg[2].polarity               = Ifx_ActiveState_high;
        outCfg[2].complementaryPolarity  = Ifx_ActiveState_high;
        outCfg[2].outputMode             = OUTPUT_PORT_OUTPUT_MODE;
        outCfg[2].padDriver              = OUTPUT_PAD_DRIVER;

        /* Initialize each channel config and set TOM1 pair channels as synchronized channels.
           MASTER_TIMER_CHANNEL = TOM1_CH0 (time base only) per requirements. */
        {
            uint8 i;
            for (i = 0U; i < NUM_OF_CHANNELS; i++)
            {
                IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
            }
            /* Use TOM1 even channels for HS (2,4,6) with LS (1,3,5) as complementary outputs */
            chCfg[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;  /* U phase base */
            chCfg[0].phase     = 0.0f;
            chCfg[0].duty      = INITIAL_DUTY_CYCLE_PERCENT_U;
            chCfg[0].dtm       = NULL_PTR; /* DTM updated via runtime API as requested */
            chCfg[0].output    = &outCfg[0];
            chCfg[0].mscOut    = NULL_PTR;
            chCfg[0].interrupt = NULL_PTR;

            chCfg[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_4;  /* V phase */
            chCfg[1].phase     = 0.0f;
            chCfg[1].duty      = INITIAL_DUTY_CYCLE_PERCENT_V;
            chCfg[1].dtm       = NULL_PTR;
            chCfg[1].output    = &outCfg[1];
            chCfg[1].mscOut    = NULL_PTR;
            chCfg[1].interrupt = NULL_PTR;

            chCfg[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_6;  /* W phase */
            chCfg[2].phase     = 0.0f;
            chCfg[2].duty      = INITIAL_DUTY_CYCLE_PERCENT_W;
            chCfg[2].dtm       = NULL_PTR;
            chCfg[2].output    = &outCfg[2];
            chCfg[2].mscOut    = NULL_PTR;
            chCfg[2].interrupt = NULL_PTR;
        }

        /* Unified PWM top-level configuration */
        config.cluster            = IfxGtm_Cluster_0;                 /* Cluster selection */
        config.subModule          = IfxGtm_Pwm_SubModule_tom;         /* TOM submodule */
        config.alignment          = IfxGtm_Pwm_Alignment_center;      /* Center-aligned */
        config.syncStart          = TRUE;                              /* Start after init */
        config.syncUpdateEnabled  = TRUE;                              /* Shadow updates */
        config.numChannels        = NUM_OF_CHANNELS;
        config.channels           = &chCfg[0];
        config.frequency          = TIMING_PWM_FREQUENCY_HZ;
        config.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;                /* Fxclk0 per requirement */
        config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM clock */

        /* Initialize PWM driver (no return value per API) */
        IfxGtm_Pwm_init(&s_drv.pwm, &s_drv.channels[0], &config);
    }

    /* 4) Program target PWM frequency immediately (explicit per SW Detailed Design) */
    IfxGtm_Pwm_updateFrequencyImmediate(&s_drv.pwm, TIMING_PWM_FREQUENCY_HZ);

    /* 5) Configure uniform dead-time across all complementary pairs */
    {
        IfxGtm_Pwm_DeadTime reqDeadTime[NUM_OF_CHANNELS];
        uint8 i;
        for (i = 0U; i < NUM_OF_CHANNELS; i++)
        {
            /* Apply same dead-time for rising and falling edges */
            reqDeadTime[i].rising  = TIMING_DEAD_TIME_SECONDS;
            reqDeadTime[i].falling = TIMING_DEAD_TIME_SECONDS;
        }
        IfxGtm_Pwm_updateChannelsDeadTimeImmediate(&s_drv.pwm, &reqDeadTime[0]);
    }

    /* 6) Prepare and apply initial duties (single synchronous update) */
    {
        float32 duties[NUM_OF_CHANNELS];
        duties[0] = INITIAL_DUTY_CYCLE_PERCENT_U;
        duties[1] = INITIAL_DUTY_CYCLE_PERCENT_V;
        duties[2] = INITIAL_DUTY_CYCLE_PERCENT_W;
        IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, &duties[0]);

        /* Store for runtime updates */
        s_drv.dutyCycles[0] = duties[0];
        s_drv.dutyCycles[1] = duties[1];
        s_drv.dutyCycles[2] = duties[2];
    }

    /* 7) Start synchronized channels and enable outputs (explicit per SW Detailed Design) */
    IfxGtm_Pwm_startSyncedChannels(&s_drv.pwm);
    IfxGtm_Pwm_startChannelOutputs(&s_drv.pwm);

    s_initialized = TRUE;
}

/*
 * Update UVW duties: compute step as fixed fraction of period (percent),
 * add to each duty, apply min/max bounds derived from min pulse, wrap to min if >= max,
 * then apply as a single synchronous update across all channels.
 */
void GTM_TOM_3_Phase_Inverter_PWM_updateUVW(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if initialization failed or not performed */
    }

    /* Compute min/max duty bounds from min pulse requirement */
    const float32 period_s  = 1.0f / TIMING_PWM_FREQUENCY_HZ;
    const float32 minPct    = (TIMING_MIN_PULSE_SECONDS / period_s) * 100.0f; /* percent */
    const float32 maxPct    = 100.0f - minPct;
    const float32 stepPct   = DUTY_STEP_PERCENT;

    /* Increment and wrap per channel */
    uint8 i;
    for (i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        float32 d = s_drv.dutyCycles[i] + stepPct;
        if (d >= maxPct)
        {
            d = minPct; /* wrap to min bound */
        }
        /* Enforce lower bound if needed */
        if (d < minPct)
        {
            d = minPct;
        }
        s_drv.dutyCycles[i] = d;
    }

    /* Apply as one synchronous duty update (complementary handling internal to driver) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, &s_drv.dutyCycles[0]);
}

/* =====================================================================
 * Private helpers
 * ===================================================================== */
static void GTM_TOM_3_Phase_Inverter_PWM_updateDutyArrayInternal(void)
{
    /* Intentionally left as a placeholder for future extensions (e.g., modulation profiles).
       Current algorithm is implemented directly in the public update function per SW design. */
}
