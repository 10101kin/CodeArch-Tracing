#include "egtm_atom_3_phase_inverter_pwm.h"

/* ========================= Internal Driver State ========================= */
typedef struct
{
    IfxEgtm_Pwm             pwm;                                 /* PWM driver handle */
    IfxEgtm_Pwm_Channel     channels[NUM_OF_CHANNELS];           /* Channel descriptors */
    float32                 dutyCycles[NUM_OF_CHANNELS];          /* Duty in percent [0..100] */
    float32                 phases[NUM_OF_CHANNELS];              /* Phase shift in degrees (unused -> 0) */
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];           /* Dead-time copy */
    /* Status and error flags */
    boolean                 pwmInitialized;
    boolean                 dtmConfigured;
    boolean                 updateInProgress;
    boolean                 errorClock;
    boolean                 errorPinMap;
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv = {0};

/* Public software input: normalized duties [0.0 .. 1.0] for U,V,W */
float32 g_egtmAtom3phInv_requestedDuty[NUM_OF_CHANNELS] = {0.0f, 0.0f, 0.0f};

/* ========================= ISR and Period Callback ========================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    /* Period event callback (optional). No heavy processing here. */
    (void)data;
}

/* ========================= Local Helpers ========================= */
static float32 egtm_clamp01(float32 v)
{
    if (v < 0.0f) { return 0.0f; }
    if (v > 1.0f) { return 1.0f; }
    return v;
}

/* ========================= Initialization ========================= */
void IfxEgtm_Atom_Pwm_init(void)
{
    /* Clear status */
    g_egtmAtom3phInv.pwmInitialized  = FALSE;
    g_egtmAtom3phInv.dtmConfigured   = FALSE;
    g_egtmAtom3phInv.updateInProgress= FALSE;
    g_egtmAtom3phInv.errorClock      = FALSE;
    g_egtmAtom3phInv.errorPinMap     = FALSE;

    /* 1) Enable EGTM and CMU clocks as required (GCLK + CLK0 to 100 MHz) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);                    /* Enable and set GCLK */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, CLOCK_TARGET_FXCLK0_HZ); /* Set CLK0 = 100 MHz */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);            /* Enable CLK0 */
    }

    /* Verify functional FX clock presence if API provided */
    if (!IfxEgtm_Cmu_isFxClockEnabled(&MODULE_EGTM))
    {
        g_egtmAtom3phInv.errorClock = TRUE;
        return; /* Early exit on clock failure as per error-handling requirements */
    }

    /* 2) Initialize unified PWM config */
    IfxEgtm_Pwm_Config config;
    IfxEgtm_Pwm_ChannelConfig channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure outputs (complementary via CDTM) */
    /* Phase U */
    output[0].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                 = Ifx_ActiveState_high;
    output[0].complementaryPolarity    = Ifx_ActiveState_low;
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_low;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_low;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Configure hardware dead-time for complementary pairs */
    {
        const float32 dead_s = (TIMING_DEADTIME_US * 1.0e-6f);
        dtmConfig[0].deadTime.rising  = dead_s;
        dtmConfig[0].deadTime.falling = dead_s;
        dtmConfig[1].deadTime.rising  = dead_s;
        dtmConfig[1].deadTime.falling = dead_s;
        dtmConfig[2].deadTime.rising  = dead_s;
        dtmConfig[2].deadTime.falling = dead_s;
    }

    /* 5) Configure period interrupt (base channel only) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Configure channels (ATOM0: CH0/CH1/CH2) */
    /* Phase U -> CH0 */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = 0.0f; /* start disabled */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* base channel interrupt */

    /* Phase V -> CH1 */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = 0.0f;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Phase W -> CH2 */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = 0.0f;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Global PWM configuration */
    config.cluster              = IfxEgtm_Cluster_1;                       /* EGTM Cluster 1 */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;              /* ATOM */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;            /* center-aligned */
    config.syncStart            = TRUE;                                     /* synchronized start */
    config.syncUpdateEnabled    = TRUE;                                     /* shadow updates */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = TIMING_PWM_FREQUENCY_HZ;
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                        /* use CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;        /* CDTM/DTM clock */

    /* 8) Initialize the unified PWM driver (no return value) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 9) Store initial runtime values */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;
    g_egtmAtom3phInv.dtmConfigured = TRUE;

    /* 10) Configure LED pin and install ISR handler */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxCpu_Irq_installInterruptHandler((void*)interruptEgtmAtom, (uint32)ISR_PRIORITY_ATOM);

    g_egtmAtom3phInv.pwmInitialized = TRUE;
}

/* ========================= Runtime Duty Update ========================= */
void IfxEgtm_Atom_Pwm_setDutyCycle(void)
{
    if (!g_egtmAtom3phInv.pwmInitialized)
    {
        return; /* Early exit if init failed */
    }

    /* 1) Clamp requested normalized duties and convert to percent */
    boolean rangeError = FALSE;
    for (uint8 i = 0u; i < (uint8)NUM_OF_CHANNELS; i++)
    {
        float32 nrm = g_egtmAtom3phInv_requestedDuty[i];
        float32 clamped = egtm_clamp01(nrm);
        if (clamped != nrm)
        {
            rangeError = TRUE;
            g_egtmAtom3phInv_requestedDuty[i] = clamped; /* write-back clamp */
        }
        g_egtmAtom3phInv.dutyCycles[i] = clamped * 100.0f; /* percent for unified driver */
    }

    g_egtmAtom3phInv.updateInProgress = TRUE;

    /* 2) Apply synchronized multi-channel duty update (immediate to shadow) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);

    g_egtmAtom3phInv.updateInProgress = FALSE;

    /* Record range error (status only) */
    if (rangeError)
    {
        /* Keep last range error in errorPinMap flag bucket for lack of a dedicated flag */
        g_egtmAtom3phInv.errorPinMap = TRUE;
    }
}
