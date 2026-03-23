/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production code for TC4xx eGTM ATOM 3-phase inverter PWM with ADC trigger routing.
 *
 * Follows unified IfxEgtm_Pwm high-level driver initialization pattern and
 * requirement-driven configuration. No watchdog disable in this module.
 */
#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxAdc_Tmadc.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"

/* ===============================
 * Local types and state
 * =============================== */

typedef struct {
    IfxEgtm_Pwm          pwm;                              /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];        /* Runtime channel handles */
    float32              dutyCycles[NUM_OF_CHANNELS];       /* Duty in percent */
    float32              phases[NUM_OF_CHANNELS];           /* Optional phase offsets */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];        /* Dead-time values (s) */
} EgtmAtom3phInv;

typedef struct {
    IfxEgtm_Pwm          pwm;                              /* PWM driver handle for trigger */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_TRIG_CHANNELS];
    float32              dutyCycles[NUM_OF_TRIG_CHANNELS];
    float32              phases[NUM_OF_TRIG_CHANNELS];
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_TRIG_CHANNELS];
} EgtmAtomTrig;

static EgtmAtom3phInv g_egtmAtom3phInv;    /* 3-phase group */
static EgtmAtomTrig   g_egtmAtomTrig;      /* trigger group */

static IfxAdc_Tmadc   g_tmadc;             /* TMADC module handle */

static boolean        s_initialized = FALSE;        /* module initialized flag */
static boolean        s_adcTrigConfigured = FALSE;  /* ADC trigger routing status */

/* ===============================
 * Local helpers
 * =============================== */
static void configureEgtmAdcTrigger(void)
{
    /* Route eGTM ATOM0.CH3 to ADC_TRIG[36] via EGTM ADC_OUT0.SEL0 */
    boolean ok = IfxEgtm_Trigger_trigToAdc(
        (IfxEgtm_Cluster)EGTM_CLUSTER,
        IfxEgtm_TrigSource_atom0,
        (IfxEgtm_TrigChannel)EGTM_TRIGGER_CHANNEL,
        IfxEgtm_Cfg_AdcTriggerSignal_adcTrig36);
    s_adcTrigConfigured = ok;
}

/* ===============================
 * Public functions
 * =============================== */
void initEgtmAtom3phInv(void)
{
    /* 1) eGTM enable + CMU clock setup (must be before PWM init) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
    }
    /* Set GCLK to 100 MHz per requirement and enable CLK0 */
    IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, 100000000.0f);
    IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);

    /* 2) Configure heartbeat GPIO P03.9 as push-pull output */
    IfxPort_setPinModeOutput(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 3) Initialize unified PWM configuration for 3-phase */
    IfxEgtm_Pwm_Config           cfg3;
    IfxEgtm_Pwm_ChannelConfig    chCfg3[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     out3[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtm3[NUM_OF_CHANNELS];

    IfxEgtm_Pwm_initConfig(&cfg3, &MODULE_EGTM);

    /* Output config: complementary pairs, HS active-high, LS active-low */
    out3[0].pin                       = EGTM_ATOM0_CH0_HS;
    out3[0].complementaryPin          = EGTM_ATOM0_CH0_LS;
    out3[0].polarity                  = Ifx_ActiveState_high;
    out3[0].complementaryPolarity     = Ifx_ActiveState_low;
    out3[0].outputMode                = IfxPort_OutputMode_pushPull;
    out3[0].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    out3[1].pin                       = EGTM_ATOM0_CH1_HS;
    out3[1].complementaryPin          = EGTM_ATOM0_CH1_LS;
    out3[1].polarity                  = Ifx_ActiveState_high;
    out3[1].complementaryPolarity     = Ifx_ActiveState_low;
    out3[1].outputMode                = IfxPort_OutputMode_pushPull;
    out3[1].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    out3[2].pin                       = EGTM_ATOM0_CH2_HS;
    out3[2].complementaryPin          = EGTM_ATOM0_CH2_LS;
    out3[2].polarity                  = Ifx_ActiveState_high;
    out3[2].complementaryPolarity     = Ifx_ActiveState_low;
    out3[2].outputMode                = IfxPort_OutputMode_pushPull;
    out3[2].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration (same for rising/falling by unified driver) */
    const float32 deadTimeSec = (TIMING_DEADTIME_US * 1.0e-6f);
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
    {
        dtm3[i].deadTime = deadTimeSec;
    }

    /* Channel configurations */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
    {
        IfxEgtm_Pwm_initChannelConfig(&chCfg3[i]);
        chCfg3[i].duty      = 50.0f;            /* initial 50% duty */
        chCfg3[i].phase     = 0.0f;             /* no phase shift at init */
        chCfg3[i].dtm       = &dtm3[i];
        chCfg3[i].output    = &out3[i];
        chCfg3[i].interrupt = NULL_PTR;         /* no ISR in this module */
    }

    /* Main PWM configuration fields */
    cfg3.cluster            = IfxEgtm_Cluster_0;
    cfg3.subModule          = IfxEgtm_Pwm_SubModule_atom;
    cfg3.alignment          = IfxEgtm_Pwm_Alignment_center;
    cfg3.syncStart          = FALSE;                        /* will start with startSyncedGroups */
    cfg3.syncUpdateEnabled  = TRUE;
    cfg3.numChannels        = NUM_OF_CHANNELS;
    cfg3.channels           = chCfg3;
    cfg3.frequency          = TIMING_PWM_FREQUENCY_HZ;
    cfg3.clockSource.atom   = IfxEgtm_Cmu_Clk_0;            /* ATOM clock */
    cfg3.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* Initialize 3-phase PWM driver */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &cfg3);

    /* Store runtime copies */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
    {
        g_egtmAtom3phInv.dutyCycles[i] = chCfg3[i].duty;
        g_egtmAtom3phInv.phases[i]     = chCfg3[i].phase;
        g_egtmAtom3phInv.deadTimes[i]  = dtm3[i].deadTime;
    }

    /* 4) Configure a single-channel PWM for trigger (ATOM0.CH3) */
    IfxEgtm_Pwm_Config           cfgT;
    IfxEgtm_Pwm_ChannelConfig    chCfgT[NUM_OF_TRIG_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     outT[NUM_OF_TRIG_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmT[NUM_OF_TRIG_CHANNELS];

    IfxEgtm_Pwm_initConfig(&cfgT, &MODULE_EGTM);

    /* Trigger output pin (export) */
    outT[0].pin                       = EGTM_ATOM0_CH3_TRIG;   /* export trigger to P33.0 */
    outT[0].complementaryPin          = NULL_PTR;
    outT[0].polarity                  = Ifx_ActiveState_high;
    outT[0].complementaryPolarity     = Ifx_ActiveState_low;
    outT[0].outputMode                = IfxPort_OutputMode_pushPull;
    outT[0].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    dtmT[0].deadTime = 0.0f;  /* no dead-time for trigger */

    IfxEgtm_Pwm_initChannelConfig(&chCfgT[0]);
    chCfgT[0].duty      = 50.0f;        /* 50% duty */
    chCfgT[0].phase     = 0.0f;
    chCfgT[0].dtm       = &dtmT[0];
    chCfgT[0].output    = &outT[0];
    chCfgT[0].interrupt = NULL_PTR;

    cfgT.cluster            = IfxEgtm_Cluster_0;
    cfgT.subModule          = IfxEgtm_Pwm_SubModule_atom;
    cfgT.alignment          = IfxEgtm_Pwm_Alignment_edge;   /* edge-aligned trigger */
    cfgT.syncStart          = FALSE;                        /* start with group sync */
    cfgT.syncUpdateEnabled  = TRUE;
    cfgT.numChannels        = NUM_OF_TRIG_CHANNELS;
    cfgT.channels           = chCfgT;
    cfgT.frequency          = TIMING_PWM_FREQUENCY_HZ;      /* same base frequency */
    cfgT.clockSource.atom   = IfxEgtm_Cmu_Clk_0;
    cfgT.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    IfxEgtm_Pwm_init(&g_egtmAtomTrig.pwm, g_egtmAtomTrig.channels, &cfgT);

    g_egtmAtomTrig.dutyCycles[0] = chCfgT[0].duty;
    g_egtmAtomTrig.phases[0]     = chCfgT[0].phase;
    g_egtmAtomTrig.deadTimes[0]  = dtmT[0].deadTime;

    /* 5) Route eGTM trigger to ADC trigger bus */
    configureEgtmAdcTrigger();

    /* 6) Initialize TMADC module (module + run); channel mapping TBD */
    IfxAdc_Tmadc_Config tmadcCfg;
    IfxAdc_Tmadc_initModuleConfig(&tmadcCfg, (Ifx_ADC*)NULL_PTR);
    IfxAdc_Tmadc_initModule(&g_tmadc, &tmadcCfg);
    IfxAdc_Tmadc_runModule(&g_tmadc);

    /* 7) Start PWM outputs coherently (3-phase group + trigger group) */
    if (s_adcTrigConfigured != FALSE)
    {
        IfxEgtm_Pwm_startSyncedGroups(&g_egtmAtom3phInv.pwm, &g_egtmAtomTrig.pwm);
    }

    s_initialized = TRUE;
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Early-exit if module not initialized */
    if (s_initialized == FALSE)
    {
        return;
    }

    /* 1) Clamp and store into internal buffer (percent [0..100]) */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
    {
        float32 d = requestDuty[i];
        if (d < DUTY_MIN_PERCENT) { d = DUTY_MIN_PERCENT; }
        if (d > DUTY_MAX_PERCENT) { d = DUTY_MAX_PERCENT; }
        g_egtmAtom3phInv.dutyCycles[i] = d;
    }

    /* 2) Apply new duties immediately to 3-phase group */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.dutyCycles);

    /* 3) Toggle heartbeat LED */
    IfxPort_togglePin(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
}
