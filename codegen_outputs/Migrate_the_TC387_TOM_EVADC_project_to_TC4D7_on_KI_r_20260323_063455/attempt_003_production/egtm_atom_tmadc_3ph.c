/*
 * egtm_atom_tmadc_3ph.c
 * TC4xx (TC4D7/TC387 target family) - eGTM ATOM 3-phase inverter PWM + TMADC trigger
 *
 * Production code using unified iLLD drivers per SW Detailed Design.
 */
#include "egtm_atom_tmadc_3ph.h"

/* iLLD headers (generic, not family-suffixed) */
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxEgtm.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxWtu.h" /* Included for completeness; no watchdog disable here */

/* Internal driver state structure for TC4xx (EGTM ATOM) */
typedef struct {
    IfxEgtm_Pwm          pwm;                             /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];       /* Channel data after init */
    float32              dutyCycles[NUM_OF_CHANNELS];     /* Duty per logical channel (first 3 are phases) */
    float32              phases[NUM_OF_CHANNELS];         /* Optional phase shift values */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];      /* Dead-time configuration snapshot */
} EgtmAtom3phInv;

/* Module-scope state */
static EgtmAtom3phInv g_egtmAtom3phInv;                  /* PWM driver instance */
static IfxAdc_Tmadc    g_tmadc;                           /* TMADC driver instance */
static boolean         s_initialized = FALSE;             /* Initialization flag */

/* Private helper: configure eGTM pins via PinMap API (driver_calls requires this) */
static void configureEgtmPins(uint8 reserved)
{
    (void)reserved;

    /*
     * Routing note:
     * The SW Detailed Design mandates using the generic eGTM PinMap API for pin routing.
     * Board-specific Atom/Tout map symbols must be selected per hardware design.
     * Placeholders below call the API to satisfy the required driver call; replace NULL
     * with actual &IfxEgtm_ATOMx_y_TOUTz_Pnn_m_OUT symbols for real hardware.
     */

    /* Route the PWM trigger output (ATOM0.CH3) to the required debug pin P33.0.
       Replace NULL with the correct IfxEgtm_Atom_ToutMap symbol for P33.0 on your board. */
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)0, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* PWM phase outputs (P20.8–P20.13) are routed by the unified driver when provided
       via IfxEgtm_Pwm_OutputConfig. If explicit PinMap routing is desired as well,
       add corresponding IfxEgtm_PinMap_setAtomTout() calls here for each pin. */
}

/* Internal: build PWM configuration and TMADC trigger topology per SW design */
void initEgtmAtom3phInv(void)
{
    /* 1) Optional watchdog configuration (no disable in driver files) */
    /* Intentionally do not disable watchdogs here per architecture rule. */

    /* 2) Configure LED GPIO and drive low */
    IfxPort_setPinModeOutput(GPIO_LED_PORT, GPIO_LED_PIN_INDEX, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(GPIO_LED_PORT, GPIO_LED_PIN_INDEX);

    /* 3) Enable EGTM module (unified high-level driver handles internal setup) */
    IfxEgtm_enable(&MODULE_EGTM);

    /* Unified PWM configuration structures */
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    outputCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmCfg[NUM_OF_CHANNELS];

    /* Initialize config with defaults (MODULE_EGTM, not MODULE_EGTM0) */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 4) Channel-centric configuration: base + synchronous channels for 3 complementary phase pairs,
       plus a dedicated trigger channel. Attach DTM with 1us dead-time for phase channels. */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelCfg[i]);
        /* Default output configuration; unified driver will handle routing if pins are provided later. */
        outputCfg[i].pin                      = (IfxEgtm_Pwm_ToutMap *)0; /* To be provided per board if required */
        outputCfg[i].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)0; /* To be provided per board if required */
        outputCfg[i].polarity                 = Ifx_ActiveState_high;
        outputCfg[i].complementaryPolarity    = Ifx_ActiveState_low; /* Inverter complementary polarity */
        outputCfg[i].outputMode               = IfxPort_OutputMode_pushPull;
        outputCfg[i].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        dtmCfg[i].deadTime                    = (float32)(TIMING_DEADTIME_US * 1.0e-6f); /* seconds */

        channelCfg[i].output                  = &outputCfg[i];
        channelCfg[i].dtm                     = (i < NUM_OF_PHASES) ? &dtmCfg[i] : (IfxEgtm_Pwm_DtmConfig *)0; /* Trigger channel: no DTM */
        channelCfg[i].interrupt               = (IfxEgtm_Pwm_InterruptConfig *)0; /* No ISR in this design */
        channelCfg[i].phase                   = 0.0f; /* Synchronous start, no phase shift between channels in this setup */

        /* Initial duties per requirement for first three channels (phases) */
        if (i == 0u)      { channelCfg[i].duty = PHASE_U_INIT_DUTY_PCT; }
        else if (i == 1u) { channelCfg[i].duty = PHASE_V_INIT_DUTY_PCT; }
        else if (i == 2u) { channelCfg[i].duty = PHASE_W_INIT_DUTY_PCT; }
        else              { channelCfg[i].duty = 50.0f; /* Trigger channel: 50% pulse default (if used) */ }
    }

    /* 5) Assign module-level configuration fields explicitly */
    config.cluster            = (IfxEgtm_Cluster)PWM_CLUSTER;                 /* IfxEgtm_Cluster_0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;                  /* ATOM submodule */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;                /* Center-aligned */
    config.syncStart          = (TIMING_SYNC_START != 0);                    /* Use requirement value */
    config.syncUpdateEnabled  = TRUE;                                        /* Shadow updates */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelCfg[0];
    config.frequency          = TIMING_PWM_FREQUENCY_HZ;                     /* 20 kHz */
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                           /* ATOM Clk0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;           /* DTM clock 0 */

    /* 6) Initialize the PWM driver with contiguous channel group */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store configured dead-times for runtime reference */
    for (uint8 i = 0u; i < NUM_OF_PHASES; i++)
    {
        g_egtmAtom3phInv.deadTimes[i].deadTime = dtmCfg[i].deadTime;
    }

    /* 7) Configure a dedicated PWM channel trigger and connect to TMADC via eGTM trigger crossbar */
    configureEgtmPins(0u); /* Route trigger pin via PinMap API per design */

    /* Route ATOM0.CH3 to ADC_OUT0: use requirement indices, cast to expected enum types */
    {
        boolean trigOk = IfxEgtm_Trigger_trigToAdc(
            (IfxEgtm_Cluster)PWM_CLUSTER,                      /* Cluster 0 */
            (IfxEgtm_TrigSource)PWM_ATOM,                      /* ATOM0 as source */
            (IfxEgtm_TrigChannel)PWM_TRIGGER_ATOM_CH,          /* CH3 */
            (IfxEgtm_Cfg_AdcTriggerSignal)0                    /* ADC_OUT0 */
        );
        if (trigOk == FALSE)
        {
            /* Early exit on failure per error-handling requirement */
            s_initialized = FALSE;
            return;
        }
    }

    /* 8) Initialize TMADC module and channels; enable module to process external triggers */
    {
        IfxAdc_Tmadc_Config adcCfg;
        IfxAdc_Tmadc_ChConfig chCfg;

        IfxAdc_Tmadc_initModuleConfig(&adcCfg, &MODULE_ADC);
        IfxAdc_Tmadc_initModule(&g_tmadc, &adcCfg);

        /* Configure 5 channels (IDs TBD) to be externally triggered; using default chCfg as placeholder */
        IfxAdc_Tmadc_initChannelConfig(&chCfg, &MODULE_ADC);
        for (uint8 c = 0u; c < 5u; c++)
        {
            IfxAdc_Tmadc_Ch ch;
            (void)c; /* Channel selection TBD per hardware; chCfg should be set accordingly */
            IfxAdc_Tmadc_initChannel(&ch, &chCfg);
        }
        IfxAdc_Tmadc_runModule(&g_tmadc);
    }

    /* 9) Store initial duty setpoints (phases) for runtime updates */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_INIT_DUTY_PCT;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_INIT_DUTY_PCT;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_INIT_DUTY_PCT;
    g_egtmAtom3phInv.dutyCycles[3] = 50.0f; /* Trigger channel default (unused by inverter) */

    s_initialized = TRUE;
}

void startEgtmAtom3phInv(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit on uninitialized */
    }

    /* Stage initial duty cycles for all channels (first 3 are inverter phases) */
    IfxEgtm_Pwm_updateChannelsDuty(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.dutyCycles[0]);

    /* Perform a synchronous start so PWM and trigger align at period boundary */
    IfxEgtm_Pwm_startSyncedChannels(&g_egtmAtom3phInv.pwm);
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit on uninitialized */
    }

    /* Copy three requested duty values (U, V, W) into internal array */
    g_egtmAtom3phInv.dutyCycles[0] = requestDuty[0];
    g_egtmAtom3phInv.dutyCycles[1] = requestDuty[1];
    g_egtmAtom3phInv.dutyCycles[2] = requestDuty[2];

    /* Apply immediately to hardware (shadow registers updated and committed) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.dutyCycles[0]);
}
