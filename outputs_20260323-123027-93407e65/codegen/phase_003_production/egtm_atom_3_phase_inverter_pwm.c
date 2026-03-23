#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxAdc_Tmadc.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxWtu.h"
#include "IfxPort.h"

/*============================
 * Internal state
 *============================*/
static EgtmAtom3phInv  g_egtmAtom3phInv;  /* 3-phase PWM context */
static EgtmAtomTrigCtx g_egtmAtomTrig;    /* Trigger PWM context */
static boolean         s_initialized = FALSE;

/*=====================================
 * Pin routing (EGTM TOUT pin symbols)
 * Note: Unified driver reads these to route pins internally.
 *=====================================*/
/* Assumed valid ATOM0 TOUT mappings for P20.x based on device routing. */
#define PHASE_U_HS   ((IfxEgtm_Pwm_ToutMap *)&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS   ((IfxEgtm_Pwm_ToutMap *)&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS   ((IfxEgtm_Pwm_ToutMap *)&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS   ((IfxEgtm_Pwm_ToutMap *)&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS   ((IfxEgtm_Pwm_ToutMap *)&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS   ((IfxEgtm_Pwm_ToutMap *)&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* Trigger export pin (ATOM0 CH3) to P33.0 - mapping symbol may vary per device */
/* If unavailable on target, keep NULL (driver will not export) and rely on ADC trigger route. */
#define TRIG_OUT_PIN   ((IfxEgtm_Pwm_ToutMap *)0)

/*============================
 * Private helpers
 *============================*/
static boolean configureEgtmAdcTrigger(void)
{
    /* Route EGTM.ATOM0.CH3 to ADC trigger bus: ADC_OUT0.SEL0 -> ADC_TRIG[36] */
    /* Types: (IfxEgtm_Cluster, IfxEgtm_TrigSource, IfxEgtm_TrigChannel, IfxEgtm_Cfg_AdcTriggerSignal) */
    const IfxEgtm_Cluster            cluster = IfxEgtm_Cluster_0;
    const IfxEgtm_TrigSource         source  = IfxEgtm_TrigSource_atom0;
    const IfxEgtm_TrigChannel        chan    = IfxEgtm_TrigChannel_3;
    const IfxEgtm_Cfg_AdcTriggerSignal sig   = IfxEgtm_Cfg_AdcTriggerSignal_36;

    boolean ok = IfxEgtm_Trigger_trigToAdc(cluster, source, chan, sig);
    return ok;
}

/*============================
 * Public API implementation
 *============================*/
void initEgtmAtom3phInv(void)
{
    /* 1) Enable EGTM and CMU clocks (CLK0) - mandatory for TC4xx */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* 2) Build PWM configs for 3-phase and trigger contexts */
    IfxEgtm_Pwm_Config           cfg3ph;
    IfxEgtm_Pwm_ChannelConfig   chCfg3ph[NUM_OF_PHASE_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    outCfg3ph[NUM_OF_PHASE_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmCfg3ph[NUM_OF_PHASE_CHANNELS];

    IfxEgtm_Pwm_Config           cfgTrig;
    IfxEgtm_Pwm_ChannelConfig   chCfgTrig[NUM_OF_TRIG_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    outCfgTrig[NUM_OF_TRIG_CHANNELS];

    IfxEgtm_Pwm_initConfig(&cfg3ph, &MODULE_EGTM);
    cfg3ph.cluster            = IfxEgtm_Cluster_0;
    cfg3ph.subModule          = IfxEgtm_Pwm_SubModule_atom;
    cfg3ph.alignment          = IfxEgtm_Pwm_Alignment_center;
    cfg3ph.syncStart          = FALSE; /* we'll start both groups together */
    cfg3ph.syncUpdateEnabled  = TRUE;
    cfg3ph.numChannels        = NUM_OF_PHASE_CHANNELS;
    cfg3ph.channels           = &chCfg3ph[0];
    cfg3ph.frequency          = TIMING_PWM_FREQUENCY_HZ;
    cfg3ph.clockSource.atom   = IfxEgtm_Cmu_Clk_0; /* ATOM uses Clk enum */
    cfg3ph.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* Initialize per-channel defaults and customize */
    for (uint8 i = 0U; i < NUM_OF_PHASE_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&chCfg3ph[i]);
        chCfg3ph[i].channel    = (uint8)i;               /* ATOM0 CH0..CH2 */
        chCfg3ph[i].duty       = 0.0f;                   /* percent */
        chCfg3ph[i].phase      = 0.0f;                   /* degrees */
        chCfg3ph[i].output     = &outCfg3ph[i];
        chCfg3ph[i].dtm        = &dtmCfg3ph[i];
        chCfg3ph[i].interrupt  = NULL_PTR;               /* No ISR */
        chCfg3ph[i].frequency  = cfg3ph.frequency;       /* for mocks */

        dtmCfg3ph[i].deadTime  = (TIMING_DEADTIME_US * 1.0e-6f); /* seconds */

        outCfg3ph[i].polarity              = Ifx_ActiveState_high; /* HS active high */
        outCfg3ph[i].complementaryPolarity = Ifx_ActiveState_low;  /* LS active low */
        outCfg3ph[i].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg3ph[i].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    }

    /* Assign pins per phase (primary + complementary) via OutputConfig */
    outCfg3ph[0].pin              = PHASE_U_HS; outCfg3ph[0].complementaryPin = PHASE_U_LS;
    outCfg3ph[1].pin              = PHASE_V_HS; outCfg3ph[1].complementaryPin = PHASE_V_LS;
    outCfg3ph[2].pin              = PHASE_W_HS; outCfg3ph[2].complementaryPin = PHASE_W_LS;

    /* Trigger PWM config: 50% duty, edge-aligned, same base frequency */
    IfxEgtm_Pwm_initConfig(&cfgTrig, &MODULE_EGTM);
    cfgTrig.cluster            = IfxEgtm_Cluster_0;
    cfgTrig.subModule          = IfxEgtm_Pwm_SubModule_atom;
    cfgTrig.alignment          = IfxEgtm_Pwm_Alignment_edge; /* edge-aligned trigger */
    cfgTrig.syncStart          = FALSE;                      /* will be started with 3ph */
    cfgTrig.syncUpdateEnabled  = TRUE;
    cfgTrig.numChannels        = NUM_OF_TRIG_CHANNELS;
    cfgTrig.channels           = &chCfgTrig[0];
    cfgTrig.frequency          = TIMING_PWM_FREQUENCY_HZ;
    cfgTrig.clockSource.atom   = IfxEgtm_Cmu_Clk_0;

    IfxEgtm_Pwm_initChannelConfig(&chCfgTrig[0]);
    chCfgTrig[0].channel    = (uint8)EGTM_TRIGGER_CHANNEL; /* ATOM0 CH3 */
    chCfgTrig[0].duty       = 50.0f;                       /* 50% */
    chCfgTrig[0].phase      = 0.0f;
    chCfgTrig[0].output     = &outCfgTrig[0];
    chCfgTrig[0].dtm        = NULL_PTR;                    /* no complementary */
    chCfgTrig[0].interrupt  = NULL_PTR;
    chCfgTrig[0].frequency  = cfgTrig.frequency;

    outCfgTrig[0].pin                   = TRIG_OUT_PIN; /* may be NULL if not routable */
    outCfgTrig[0].complementaryPin      = NULL_PTR;
    outCfgTrig[0].polarity              = Ifx_ActiveState_high;
    outCfgTrig[0].complementaryPolarity = Ifx_ActiveState_low;
    outCfgTrig[0].outputMode            = IfxPort_OutputMode_pushPull;
    outCfgTrig[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 3) Initialize unified PWM drivers */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &cfg3ph);
    IfxEgtm_Pwm_init(&g_egtmAtomTrig.pwm,   &g_egtmAtomTrig.channels[0],   &cfgTrig);

    /* 4) Route trigger to ADC and (optionally) export pin */
    if (!configureEgtmAdcTrigger())
    {
        /* ADC trigger routing failed: do not mark initialized */
        return;
    }

    /* 5) Initialize TMADC module (instance: TMADC0), start running */
    IfxAdc_Tmadc         tmadc;
    IfxAdc_Tmadc_Config  tmadcCfg;
    IfxAdc_Tmadc_initModuleConfig(&tmadcCfg, &MODULE_ADC);
    tmadcCfg.sampleRate  = (uint32)TIMING_PWM_FREQUENCY_HZ; /* align with PWM base */
    tmadcCfg.numChannels = 5U;                              /* TBD mapping */
    IfxAdc_Tmadc_initModule(&tmadc, &tmadcCfg);
    IfxAdc_Tmadc_runModule(&tmadc);

    /* 6) Configure GPIOs: heartbeat LED and trigger export pin as outputs */
    IfxPort_setPinModeOutput(PINS_HEARTBEAT_LED_PORT,
                             PINS_HEARTBEAT_LED_PIN,
                             IfxPort_OutputMode_pushPull,
                             IfxPort_OutputIdx_general);

    /* Export trigger pin as GPIO output as placeholder if EGTM mapping unavailable */
    IfxPort_setPinModeOutput(PINS_TRIGGER_OUT_PORT,
                             PINS_TRIGGER_OUT_PIN,
                             IfxPort_OutputMode_pushPull,
                             IfxPort_OutputIdx_general);

    /* 7) Watchdog preparation (no disable here; CPU files handle that policy) */
    {
        IfxWtu_Config wtuCfg;
        IfxWtu_initConfig(&wtuCfg);
        /* Initialize CPU watchdog with default cfg; pass NULL if handle unknown in this layer */
        IfxWtu_initCpuWatchdog((Ifx_WTU_WDTCPU *)0, &wtuCfg);
    }

    /* 8) Start PWM outputs synchronously (3-phase group + trigger channel) */
    IfxEgtm_Pwm_startSyncedGroups(&g_egtmAtom3phInv.pwm, &g_egtmAtomTrig.pwm);

    /* 9) Store initial runtime values for updates */
    for (uint8 i = 0U; i < NUM_OF_PHASE_CHANNELS; i++)
    {
        g_egtmAtom3phInv.dutyCycles[i] = chCfg3ph[i].duty;         /* percent */
        g_egtmAtom3phInv.phases[i]     = chCfg3ph[i].phase;        /* degrees */
        g_egtmAtom3phInv.deadTimes[i]  = dtmCfg3ph[i].deadTime;    /* seconds */
    }

    s_initialized = TRUE;
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    if (s_initialized == FALSE)
    {
        return; /* early exit if init failed */
    }

    /* 1) Clamp requested duties to valid percent range [0..100] and store */
    for (uint8 i = 0U; i < NUM_OF_PHASE_CHANNELS; i++)
    {
        float32 d = requestDuty[i];
        if (d < 0.0f)      { d = 0.0f; }
        else if (d > 100.0f) { d = 100.0f; }
        g_egtmAtom3phInv.dutyCycles[i] = d;
    }

    /* 2) Apply duties immediately to 3-phase group */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.dutyCycles[0]);

    /* 3) Toggle heartbeat */
    IfxPort_togglePin(PINS_HEARTBEAT_LED_PORT, PINS_HEARTBEAT_LED_PIN);
}
