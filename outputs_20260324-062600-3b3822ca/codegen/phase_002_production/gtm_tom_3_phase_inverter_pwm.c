/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase Inverter PWM (single-ended outputs using IfxGtm_Pwm)
 * Target: TC3xx (TC387)
 *
 * Notes:
 * - Uses unified IfxGtm_Pwm high-level driver
 * - Pins assigned via OutputConfig (no direct PinMap calls)
 * - GTM clocks enabled explicitly via IfxGtm_Cmu_* per reference pattern
 * - No watchdog handling here (only allowed in CpuN_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ============================ Local state =================================== */
typedef struct
{
    IfxGtm_Pwm          pwm;                              /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];        /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];      /* Duty cycle percent [0..100] */
    float32             phases[NUM_OF_CHANNELS];          /* Optional phase shift (deg or %) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];       /* Not used for single-ended */
} GtmTom3phInv;

static GtmTom3phInv s_pwm3ph = {0};
static boolean s_initialized = FALSE;

/* ============================ Optional ISR ================================== */
IFX_INTERRUPT(interruptGtmTom, 0, ISR_PRIORITY_TOM);
void interruptGtmTom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback (linked via interrupt config if needed) */
static void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* No-op */
}

/* =============================== Init ======================================= */
void initGtmTom3PhaseInverterPwm(void)
{
    /* 1) Enable GTM and CMU clocks per reference pattern */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        float32 modFreq;
        IfxGtm_enable(&MODULE_GTM);
        modFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, modFreq);
        /* Route GCLK to CLK0 (not strictly needed for TOM Fxclk) and enable FXCLK */
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, modFreq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Prepare unified PWM configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     outCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Interrupt config (period notify; optional diagnostic LED toggle via ISR) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_TOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 4) Output configuration - single-ended (no complementary pin) */
    /* U - TOM1 CH2 -> P02.0 */
    outCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_TOM_PIN;
    outCfg[0].complementaryPin      = NULL_PTR;
    outCfg[0].polarity              = Ifx_ActiveState_high;
    outCfg[0].complementaryPolarity = Ifx_ActiveState_low; /* Ignored w/o comp pin */
    outCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V - TOM1 CH4 -> P02.1 */
    outCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_TOM_PIN;
    outCfg[1].complementaryPin      = NULL_PTR;
    outCfg[1].polarity              = Ifx_ActiveState_high;
    outCfg[1].complementaryPolarity = Ifx_ActiveState_low;
    outCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W - TOM1 CH6 -> P02.2 */
    outCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_TOM_PIN;
    outCfg[2].complementaryPin      = NULL_PTR;
    outCfg[2].polarity              = Ifx_ActiveState_high;
    outCfg[2].complementaryPolarity = Ifx_ActiveState_low;
    outCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) DTM config (not used for single-ended; initialize to 0) */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        dtmCfg[i].deadTime.rising  = 0.0f;
        dtmCfg[i].deadTime.falling = 0.0f;
    }

    /* 6) Channel configuration: TOM1 CH2/CH4/CH6; attach interrupt to first channel */
    /* Channel 0 -> TOM1 CH2, duty 25% */
    channelCfg[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelCfg[0].phase     = 0.0f;
    channelCfg[0].duty      = DUTY_INIT_U_PERCENT;
    channelCfg[0].dtm       = NULL_PTR;              /* No complementary output */
    channelCfg[0].output    = &outCfg[0];
    channelCfg[0].mscOut    = NULL_PTR;
    channelCfg[0].interrupt = &irqCfg;               /* Period ISR on first channel */

    /* Channel 1 -> TOM1 CH4, duty 50% */
    channelCfg[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_4;
    channelCfg[1].phase     = 0.0f;
    channelCfg[1].duty      = DUTY_INIT_V_PERCENT;
    channelCfg[1].dtm       = NULL_PTR;
    channelCfg[1].output    = &outCfg[1];
    channelCfg[1].mscOut    = NULL_PTR;
    channelCfg[1].interrupt = NULL_PTR;

    /* Channel 2 -> TOM1 CH6, duty 75% */
    channelCfg[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_6;
    channelCfg[2].phase     = 0.0f;
    channelCfg[2].duty      = DUTY_INIT_W_PERCENT;
    channelCfg[2].dtm       = NULL_PTR;
    channelCfg[2].output    = &outCfg[2];
    channelCfg[2].mscOut    = NULL_PTR;
    channelCfg[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster           = IfxGtm_Cluster_0;                   /* TOM1 resides in Cluster 0 on TC3xx */
    config.subModule         = IfxGtm_Pwm_SubModule_tom;           /* Use TOM */
    config.alignment         = TIMEBASE_ALIGNMENT;                 /* Center-aligned */
    config.syncStart         = FALSE;                              /* We'll start explicitly per SW design */
    config.syncUpdateEnabled = TRUE;                               /* Shadow update at period boundary */
    config.numChannels       = NUM_OF_CHANNELS;
    config.channels          = &channelCfg[0];
    config.frequency         = TIMING_PWM_FREQUENCY_HZ;            /* 20 kHz */
    config.clockSource.tom   = IfxGtm_Cmu_Fxclk_0;                 /* TOM uses Fxclk */
    /* Single-ended: dtmClockSource unused */

    /* 8) Initialize PWM driver */
    IfxGtm_Pwm_init(&s_pwm3ph.pwm, &s_pwm3ph.channels[0], &config);

    /* 9) Start synchronized channels (explicit call requested in SW design) */
    IfxGtm_Pwm_startSyncedChannels(&s_pwm3ph.pwm);

    /* 10) Store initial duty values and apply immediately to outputs */
    s_pwm3ph.dutyCycles[0] = DUTY_INIT_U_PERCENT;
    s_pwm3ph.dutyCycles[1] = DUTY_INIT_V_PERCENT;
    s_pwm3ph.dutyCycles[2] = DUTY_INIT_W_PERCENT;

    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwm3ph.pwm, &s_pwm3ph.dutyCycles[0]);

    /* 11) Configure LED pin for ISR diagnostic toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    s_initialized = TRUE;
}

/* ============================ Runtime update ================================ */
void updateGtmTom3PhaseDuty(void)
{
    /* Behavior: increment each duty by +10%; wrap 100->0; apply immediately */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        float32 next = s_pwm3ph.dutyCycles[i] + TIMING_DUTY_STEP_PERCENT;
        if (next >= 100.0f)
        {
            next = 0.0f;
        }
        s_pwm3ph.dutyCycles[i] = next;
    }

    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwm3ph.pwm, &s_pwm3ph.dutyCycles[0]);
}
