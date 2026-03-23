#include "egtm_atom_3_phase_inverter_pwm.h"

#include "IfxEgtm.h"          /* IfxEgtm_enable / IfxEgtm_isEnabled */
#include "IfxEgtm_Cmu.h"      /* CMU clock control */
#include "IfxPort.h"          /* GPIO control */
#include "IfxCpu_Irq.h"       /* Interrupt handler installation */
#include "IfxEgtm_Pwm.h"

/* =============================
 * Internal driver state structure (TC4xx eGTM ATOM)
 * ============================= */
typedef struct
{
    IfxEgtm_Pwm           pwm;                              /* PWM Driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];        /* Channel runtime data */
    float32               dutyCycles[NUM_OF_CHANNELS];      /* Duty cycle values (%) */
    float32               phases[NUM_OF_CHANNELS];          /* Phase shift values */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];       /* Dead-time values (s) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv; /* Module-global state */

/* =============================
 * Period event callback (unified driver hook)
 * ============================= */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Optional: user period event handling. Kept minimal per best practices. */
    (void)data;
}

/* =============================
 * ISR: Period-event interrupt (toggle LED only)
 * ============================= */
IFX_INTERRUPT(EgtmAtomIsr_periodEvent, 0, ISR_PRIORITY_ATOM);
void EgtmAtomIsr_periodEvent(void)
{
    IfxPort_togglePin(LED);
}

/* =============================
 * Initialization function (unified IfxEgtm_Pwm pattern)
 * ============================= */
void initEgtmAtom3phInv(void)
{
    /* ---- Mandatory eGTM module enable + CMU clock setup ---- */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);                                        /* Enable module */
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);           /* Module reference freq */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);              /* Set GCLK to module freq */
        IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);   /* CLK0 driven by GCLK */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);      /* Enable CLK0 */
    }

    /* ---- Configuration structures ---- */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* ---- Initialize main config with defaults ---- */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* ---- Interrupt configuration (period event on base channel) ---- */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM; /* exact macro name per reference */
    interruptConfig.vmId        = IfxSrc_VmId_0;     /* TC4xx specific */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* ---- Output configuration per channel (complementary) ---- */
    /* Phase U: ATOM1 CH0, complementary 0N */
    output[0].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS; /* Replace NULL with real TOUT mapping */
    output[0].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS; /* Replace NULL with real TOUT mapping */
    output[0].polarity                  = Ifx_ActiveState_high;
    output[0].complementaryPolarity     = Ifx_ActiveState_low;              /* Inverter: complementary active-low */
    output[0].outputMode                = IfxPort_OutputMode_pushPull;
    output[0].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V: ATOM1 CH1, complementary 1N */
    output[1].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                  = Ifx_ActiveState_high;
    output[1].complementaryPolarity     = Ifx_ActiveState_low;
    output[1].outputMode                = IfxPort_OutputMode_pushPull;
    output[1].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W: ATOM1 CH2, complementary 2N */
    output[2].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                  = Ifx_ActiveState_high;
    output[2].complementaryPolarity     = Ifx_ActiveState_low;
    output[2].outputMode                = IfxPort_OutputMode_pushPull;
    output[2].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* ---- Dead-time configuration per channel (1.0 us both edges) ---- */
    {
        const float32 deadTimeSeconds = (TIMING_DEAD_TIME_US * 1.0e-6f);
        dtmConfig[0].deadTime.rising  = deadTimeSeconds;
        dtmConfig[0].deadTime.falling = deadTimeSeconds;
        dtmConfig[1].deadTime.rising  = deadTimeSeconds;
        dtmConfig[1].deadTime.falling = deadTimeSeconds;
        dtmConfig[2].deadTime.rising  = deadTimeSeconds;
        dtmConfig[2].deadTime.falling = deadTimeSeconds;
    }

    /* ---- Channel configuration (ATOM1 CH0/1/2; base channel 0) ---- */
    /* Channel 0: Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;    /* Period ISR on base channel */

    /* Channel 1: Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Channel 2: Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* ---- Main PWM configuration ---- */
    config.cluster             = IfxEgtm_Cluster_0;
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;
    config.alignment           = IfxEgtm_Pwm_Alignment_center;  /* Center-aligned */
    config.syncStart           = TRUE;                          /* Auto-start channels after init */
    config.syncUpdateEnabled   = TRUE;                          /* Shadow register updates */
    config.numChannels         = NUM_OF_CHANNELS;
    config.channels            = channelConfig;                 /* Array of channel configs */
    config.frequency           = PWM_FREQUENCY;                 /* 20 kHz */
    config.clockSource.atom    = IfxEgtm_Cmu_Clk_0;             /* ATOM uses Clk enum */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* ---- Initialize PWM driver (reads full config and applies) ---- */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* ---- Store initial runtime values for updates ---- */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* ---- Configure LED pin and install ISR ---- */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxCpu_Irq_installInterruptHandler((void*)EgtmAtomIsr_periodEvent, ISR_PRIORITY_ATOM);

    /* NOTE:
     * Do NOT call IfxEgtm_Pwm_startSyncedChannels() or enable sync update here.
     * The unified driver starts channels and enables sync updates based on the
     * config.syncStart and config.syncUpdateEnabled fields set above.
     */
}

/* =============================
 * Runtime duty update (independent ramp + immediate apply)
 * ============================= */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-around check then increment per channel (percent-based) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_egtmAtom3phInv.dutyCycles[0] = 0.0f;
    }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_egtmAtom3phInv.dutyCycles[1] = 0.0f;
    }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_egtmAtom3phInv.dutyCycles[2] = 0.0f;
    }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply duty changes immediately across all channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
