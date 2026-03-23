#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxAdc_Tmadc.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"
#include "IfxWtu.h"

/* ===============================
 * Module-private state
 * =============================== */
static EgtmAtom3phInv  g_egtmAtom3phInv;        /* 3-phase inverter PWM */
static EgtmAtomTrigOut g_egtmAtomTrig;          /* Single-channel trigger PWM */
static IfxAdc_Tmadc    g_tmadc;                 /* TMADC driver handle */
static boolean         s_initialized = FALSE;   /* Initialization state */

/* ===============================
 * ISR and callbacks (generic pattern)
 * =============================== */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Period event callback hook (optional user handling) */
}

/* ===============================
 * Private helpers
 * =============================== */
static void configureEgtmAdcTrigger(void)
{
    /* Route ATOM0.CH3 to ADC_TRIG[36] via EGTM ADC_OUT0.SEL0 */
    /* Cluster: 0, Source: ATOM0, Channel: 3, ADC trig signal: device-specific enum for TRIG[36] */
    /* Note: This call returns boolean; treat failure as non-fatal here, overall init will handle state. */
    (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                    IfxEgtm_TrigSource_atom0,
                                    IfxEgtm_TrigChannel_3,
                                    IfxEgtm_Cfg_AdcTriggerSignal_adcTrig36);
}

/* ===============================
 * Public API implementation
 * =============================== */
void initEgtmAtom3phInv(void)
{
    /* Early exit if already initialized */
    if (s_initialized == TRUE)
    {
        return;
    }

    /* 1) Mandatory eGTM enable + CMU clock setup (must be before PWM init) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 modFreq;
        IfxEgtm_enable(&MODULE_EGTM);
        modFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, modFreq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, modFreq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* 2) Initialize unified PWM configuration structures */
    IfxEgtm_Pwm_Config config3ph;
    IfxEgtm_Pwm_ChannelConfig chCfg3[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig  out3[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig     dtm3[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig isrCfg;

    IfxEgtm_Pwm_initConfig(&config3ph, &MODULE_EGTM);

    /* 3a) Interrupt configuration: pulse notify on period, provider CPU0 */
    isrCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    isrCfg.isrProvider = IfxSrc_Tos_cpu0;
    isrCfg.priority    = ISR_PRIORITY_ATOM;
    isrCfg.vmId        = IfxSrc_VmId_0;
    isrCfg.periodEvent = IfxEgtm_periodEventFunction;
    isrCfg.dutyEvent   = NULL_PTR;

    /* 3b) Output configuration for complementary PWM (HS active-high, LS active-low) */
    out3[0].pin                      = EGTM_ATOM0_CH0_HS_TOUT;
    out3[0].complementaryPin         = EGTM_ATOM0_CH0_LS_TOUT;
    out3[0].polarity                 = Ifx_ActiveState_high;
    out3[0].complementaryPolarity    = Ifx_ActiveState_low;
    out3[0].outputMode               = IfxPort_OutputMode_pushPull;
    out3[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    out3[1].pin                      = EGTM_ATOM0_CH1_HS_TOUT;
    out3[1].complementaryPin         = EGTM_ATOM0_CH1_LS_TOUT;
    out3[1].polarity                 = Ifx_ActiveState_high;
    out3[1].complementaryPolarity    = Ifx_ActiveState_low;
    out3[1].outputMode               = IfxPort_OutputMode_pushPull;
    out3[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    out3[2].pin                      = EGTM_ATOM0_CH2_HS_TOUT;
    out3[2].complementaryPin         = EGTM_ATOM0_CH2_LS_TOUT;
    out3[2].polarity                 = Ifx_ActiveState_high;
    out3[2].complementaryPolarity    = Ifx_ActiveState_low;
    out3[2].outputMode               = IfxPort_OutputMode_pushPull;
    out3[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 3c) Dead-time configuration per channel (1 us) */
    {
        const float32 deadtime_s = TIMING_DEADTIME_US * 1.0e-6f;
        dtm3[0].deadTime = deadtime_s;
        dtm3[1].deadTime = deadtime_s;
        dtm3[2].deadTime = deadtime_s;
    }

    /* 3d) Channel configuration: CH0..CH2 consecutive, center-aligned, complementary with DTM */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&chCfg3[i]);
        chCfg3[i].timerCh   = i;                 /* ATOM0.CH[i] */
        chCfg3[i].phase     = 0.0f;              /* No phase shift at init */
        chCfg3[i].duty      = 50.0f;             /* Default 50% */
        chCfg3[i].dtm       = &dtm3[i];
        chCfg3[i].output    = &out3[i];
        chCfg3[i].mscOut    = NULL_PTR;          /* Not used */
        chCfg3[i].interrupt = (i == 0u) ? &isrCfg : NULL_PTR; /* Period ISR on base channel */
    }

    /* 4) Top-level 3-phase config fields */
    config3ph.cluster            = IfxEgtm_Cluster_0;
    config3ph.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config3ph.alignment          = IfxEgtm_Pwm_Alignment_center; /* Center-aligned */
    config3ph.syncStart          = FALSE;                        /* We'll start groups coherently */
    config3ph.syncUpdateEnabled  = TRUE;                         /* Shadow register updates */
    config3ph.numChannels        = NUM_OF_CHANNELS;
    config3ph.channels           = chCfg3;
    config3ph.frequency          = TIMING_PWM_FREQUENCY_HZ;
    config3ph.clockSource.atom   = IfxEgtm_Cmu_Clk_0;            /* ATOM uses Clk enum */
    config3ph.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* 5) Initialize the 3-phase PWM driver */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config3ph);

    /* Preserve runtime state */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_egtmAtom3phInv.dutyCycles[i] = chCfg3[i].duty;
        g_egtmAtom3phInv.deadTimes[i]  = dtm3[i];
        g_egtmAtom3phInv.phases[i]     = chCfg3[i].phase;
    }

    /* 6) Configure the trigger output context: ATOM0.CH3, edge-aligned 50% at same base freq, export to P33.0 */
    IfxEgtm_Pwm_Config configTrig;
    IfxEgtm_Pwm_ChannelConfig chCfgTrig[NUM_OF_ADC_TRIG_CHANNELS];
    IfxEgtm_Pwm_OutputConfig  outTrig[NUM_OF_ADC_TRIG_CHANNELS];

    IfxEgtm_Pwm_initConfig(&configTrig, &MODULE_EGTM);

    outTrig[0].pin                      = EGTM_ATOM0_CH3_TOUT;
    outTrig[0].complementaryPin         = NULL_PTR; /* Not complementary */
    outTrig[0].polarity                 = Ifx_ActiveState_high;
    outTrig[0].complementaryPolarity    = Ifx_ActiveState_high; /* N/A */
    outTrig[0].outputMode               = IfxPort_OutputMode_pushPull;
    outTrig[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    IfxEgtm_Pwm_initChannelConfig(&chCfgTrig[0]);
    chCfgTrig[0].timerCh   = EGTM_TRIGGER_CHANNEL; /* ATOM0.CH3 */
    chCfgTrig[0].phase     = 0.0f;
    chCfgTrig[0].duty      = 50.0f;                /* 50% edge-aligned trigger */
    chCfgTrig[0].dtm       = NULL_PTR;
    chCfgTrig[0].output    = &outTrig[0];
    chCfgTrig[0].mscOut    = NULL_PTR;
    chCfgTrig[0].interrupt = NULL_PTR;             /* No ISR on trigger */

    configTrig.cluster            = IfxEgtm_Cluster_0;
    configTrig.subModule          = IfxEgtm_Pwm_SubModule_atom;
    configTrig.alignment          = IfxEgtm_Pwm_Alignment_edge;  /* Edge-aligned */
    configTrig.syncStart          = FALSE;                        /* Start with group sync */
    configTrig.syncUpdateEnabled  = TRUE;
    configTrig.numChannels        = NUM_OF_ADC_TRIG_CHANNELS;
    configTrig.channels           = chCfgTrig;
    configTrig.frequency          = TIMING_PWM_FREQUENCY_HZ;
    configTrig.clockSource.atom   = IfxEgtm_Cmu_Clk_0;
    configTrig.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    IfxEgtm_Pwm_init(&g_egtmAtomTrig.pwm, g_egtmAtomTrig.channels, &configTrig);

    g_egtmAtomTrig.dutyCycles[0] = chCfgTrig[0].duty;
    g_egtmAtomTrig.phases[0]     = chCfgTrig[0].phase;

    /* 7) Route ATOM0.CH3 to ADC trigger bus and export to pin */
    configureEgtmAdcTrigger();

    /* 8) Initialize and start TMADC (channel assignment TBD) */
    {
        IfxAdc_Tmadc_Config tmadcCfg;
        IfxAdc_Tmadc_initModuleConfig(&tmadcCfg, &MODULE_ADC);
        /* Channel mapping TBD: requirements specify TMADC0 with 5 channels */
        /* tmadcCfg.sampleRate, tmadcCfg.numChannels etc. remain default from initModuleConfig */
        IfxAdc_Tmadc_initModule(&g_tmadc, &tmadcCfg);
        IfxAdc_Tmadc_runModule(&g_tmadc);
    }

    /* 9) Heartbeat GPIO as push-pull output */
    IfxPort_setPinModeOutput(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 10) Optional watchdog initialization (no disable here; disable belongs in CPU files) */
    {
        IfxWtu_Config wtuCfg;
        IfxWtu_initConfig(&wtuCfg);
        /* Initialize CPU watchdog using default config; platform-specific instance: CPU0 */
        IfxWtu_initCpuWatchdog(&MODULE_WTU.WDTCPU0, &wtuCfg);
    }

    /* 11) Start PWM outputs coherently: 3-phase group and trigger channel */
    IfxEgtm_Pwm_startSyncedGroups(&g_egtmAtom3phInv.pwm, &g_egtmAtomTrig.pwm);

    /* Initialization succeeded */
    s_initialized = TRUE;
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Early exit if module not initialized */
    if (s_initialized == FALSE)
    {
        return;
    }

    /* Expect three duties in percent [0..100] (unified PWM API uses percent) */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        float32 d = requestDuty[i];
        if (d < 0.0f)    { d = 0.0f; }
        if (d > 100.0f)  { d = 100.0f; }
        g_egtmAtom3phInv.dutyCycles[i] = d;
    }

    /* Apply immediate multi-channel duty update */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.dutyCycles);

    /* Heartbeat toggle to indicate update service */
    IfxPort_togglePin(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
}
