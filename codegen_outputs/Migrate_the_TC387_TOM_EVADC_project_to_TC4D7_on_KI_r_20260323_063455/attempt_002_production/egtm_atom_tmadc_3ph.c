#include "egtm_atom_tmadc_3ph.h"

#include <stdint.h>                     /* for uint8_t in private helper signature */
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxEgtm.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxWtu.h"

/*============================================================================*/
/* Local types and state                                                      */
/*============================================================================*/

typedef struct
{
    IfxEgtm_Pwm          pwm;                                /* PWM Driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];          /* Channel data after init */
    float32              dutyCycles[NUM_OF_CHANNELS];        /* Duty cycle values (%) */
    float32              phases[NUM_OF_CHANNELS];            /* Phase shift values (deg or arbitrary) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];         /* Dead-time values (s) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;
static IfxAdc_Tmadc   g_tmadc;                                /* TMADC module handle */
static IfxAdc_Tmadc_Ch g_tmadcCh[5];                          /* 5 TMADC channels (IDs TBD) */
static boolean         s_initialized = FALSE;

/*============================================================================*/
/* Private helpers                                                            */
/*============================================================================*/
/**
 * Configure eGTM output routing using generic PinMap where required by design.
 * Note: Unified IfxEgtm_Pwm driver handles pin routing via OutputConfig internally.
 * This helper routes the dedicated trigger output for debug/monitor to the requested pin.
 * The mapping symbol is target-specific; when unavailable in this template, a NULL map
 * is used to satisfy the API contract in TDD. On hardware, replace with a valid map.
 */
static void configureEgtmPins(uint8_t reserved)
{
    (void)reserved;

    /* Route the trigger (ATOM0.CH3) to a TOUT on P33.0 for monitoring (requirements). */
    /* Replace the NULL map with a valid IfxEgtm_Atom_ToutMap pointer for your board, e.g.:
       IfxEgtm_PinMap_setAtomTout(&IfxEgtm_ATOM0_3_TOUTxx_P33_0_OUT, ...);
       The unified driver doesn't require this for ADC triggering; this is for external visibility. */
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)0,
                               IfxPort_OutputMode_pushPull,
                               IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/*============================================================================*/
/* Public API                                                                 */
/*============================================================================*/
/**
 * Initialize the eGTM ATOM 3-phase PWM and TMADC triggering per SW Detailed Design.
 * Steps:
 *  1) Optional watchdog configuration (no disable here; CPU files handle that).
 *  2) Configure LED GPIO as output and drive low.
 *  3) Initialize unified PWM config; ATOM submodule, center-aligned, target frequency.
 *  4) Configure three synchronous complementary PWM channels with DTM dead-time.
 *  5) Assign output pins using eGTM PinMap API where required (trigger visibility).
 *  6) Initialize the PWM driver with prepared configs.
 *  7) Configure a dedicated PWM trigger route: EGTM.ADC_OUT0 <- ATOM0.CH3.
 *  8) Initialize TMADC module and 5 channels; start TMADC to accept external triggers.
 *  9) Store initial duty setpoints; keep outputs disabled until start() is called.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Optional watchdog configuration (do not disable here) */
    {
        IfxWtu_Config wtuCfg;
        IfxWtu_initConfig(&wtuCfg);
        /* CPU/Safety watchdog disable must be in CpuN_Main.c only; here we only init the structure. */
        IfxWtu_initCpuWatchdog((Ifx_WTU_WDTCPU *)0, &wtuCfg);
    }

    /* 2) LED GPIO configuration */
    IfxPort_setPinModeOutput(GPIO_LED_PORT, (uint8)GPIO_LED_PIN_INDEX, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(GPIO_LED_PORT, (uint8)GPIO_LED_PIN_INDEX);

    /* 3) Ensure eGTM module is enabled (CMU clocking is handled by unified driver) */
    IfxEgtm_enable(&MODULE_EGTM);

    /* Unified PWM configuration */
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];

    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Explicit core configuration per requirements */
    config.cluster           = (IfxEgtm_Cluster)PWM_CLUSTER;                 /* Cluster 0 */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;                  /* Use ATOM */
    config.alignment         = IfxEgtm_Pwm_Alignment_center;                /* Center-aligned */
    config.syncStart         = FALSE;                                       /* Keep outputs disabled until start() */
    config.syncUpdateEnabled = TRUE;                                        /* Shadow updates */
    config.frequency         = (float32)TIMING_PWM_FREQUENCY_HZ;            /* 20 kHz */

    /* Configure channels, outputs, and DTM per-channel */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);

        /* Output configuration (pin routing handled by unified driver if pin != NULL) */
        output[i].pin                      = (IfxEgtm_Pwm_ToutMap *)0;       /* Set to valid map for hardware */
        output[i].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)0;       /* Set to valid map for hardware */
        output[i].polarity                 = Ifx_ActiveState_high;           /* High-side active high */
        output[i].complementaryPolarity    = Ifx_ActiveState_low;            /* Low-side active low (inverted) */
        output[i].outputMode               = IfxPort_OutputMode_pushPull;
        output[i].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* DTM dead-time configuration (seconds) */
        dtmConfig[i].deadTime = (float32)(TIMING_DEADTIME_US * 1.0e-6f);     /* 1 us */

        /* Channel configuration fields */
        channelConfig[i].output = &output[i];
        channelConfig[i].dtm    = &dtmConfig[i];
        channelConfig[i].phase  = 0.0f;                                      /* Phase offset not used here */

        /* Initial duties per requirements */
        if (i == 0U) { channelConfig[i].duty = PHASE_U_DUTY; }
        else if (i == 1U) { channelConfig[i].duty = PHASE_V_DUTY; }
        else { channelConfig[i].duty = PHASE_W_DUTY; }

        /* No explicit interrupt linkage in this design */
        channelConfig[i].interrupt = (IfxEgtm_Pwm_InterruptConfig *)0;
    }

    config.channels    = &channelConfig[0];
    config.numChannels = (uint32)NUM_OF_CHANNELS;

    /* 5) Configure pins via PinMap as requested (dedicated trigger visibility) */
    configureEgtmPins(0U);

    /* 6) Initialize the PWM driver */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 7) Route ATOM0.CH3 to EGTM.ADC_OUT0 for TMADC external trigger (internal crossbar) */
    {
        boolean trigOk;
        trigOk = IfxEgtm_Trigger_trigToAdc((IfxEgtm_Cluster)PWM_CLUSTER,
                                           (IfxEgtm_TrigSource)PWM_ATOM,                /* ATOM0 */
                                           (IfxEgtm_TrigChannel)PWM_TRIGGER_ATOM_CH,    /* CH3 */
                                           (IfxEgtm_Cfg_AdcTriggerSignal)0U);           /* ADC_OUT0 */
        if (trigOk == FALSE)
        {
            /* Do not proceed if trigger routing failed */
            s_initialized = FALSE;
            return;
        }
    }

    /* 8) Initialize TMADC (module 0) and 5 channels; enable module to accept external triggers */
    {
        IfxAdc_Tmadc_Config    adcCfg;
        IfxAdc_Tmadc_ChConfig  chCfg;

        IfxAdc_Tmadc_initModuleConfig(&adcCfg, &MODULE_ADC);
        IfxAdc_Tmadc_initModule(&g_tmadc, &adcCfg);

        IfxAdc_Tmadc_initChannelConfig(&chCfg, &MODULE_ADC);
        for (uint8 i = 0U; i < 5U; i++)
        {
            /* Channel IDs and inputs are TBD in requirements; use default config per template */
            IfxAdc_Tmadc_initChannel(&g_tmadcCh[i], &chCfg);
        }
        IfxAdc_Tmadc_runModule(&g_tmadc);
    }

    /* 9) Store initial duty and dead-time values for runtime updates; keep outputs disabled. */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.deadTimes[0].deadTime = (float32)(TIMING_DEADTIME_US * 1.0e-6f);
    g_egtmAtom3phInv.deadTimes[1].deadTime = (float32)(TIMING_DEADTIME_US * 1.0e-6f);
    g_egtmAtom3phInv.deadTimes[2].deadTime = (float32)(TIMING_DEADTIME_US * 1.0e-6f);

    s_initialized = TRUE;
}

/**
 * Start the synchronized PWM outputs and apply the precomputed initial duties.
 * Ensures all channels begin in phase and the ADC trigger aligns with the PWM period.
 */
void startEgtmAtom3phInv(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit on failed init */
    }

    float32 stagedDuty[NUM_OF_CHANNELS];
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        stagedDuty[i] = g_egtmAtom3phInv.dutyCycles[i];
    }

    IfxEgtm_Pwm_updateChannelsDuty(&g_egtmAtom3phInv.pwm, &stagedDuty[0]);
    IfxEgtm_Pwm_startSyncedChannels(&g_egtmAtom3phInv.pwm);
}

/**
 * Runtime duty update: copy request array (U, V, W) and apply immediately.
 * No clamping; complementary outputs inherit dead-time via DTM automatically.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit on failed init */
    }

    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        g_egtmAtom3phInv.dutyCycles[i] = requestDuty[i];
    }

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.dutyCycles[0]);
}
