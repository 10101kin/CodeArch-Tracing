#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* ============================= Module State ============================= */
IFX_STATIC EgtmAtom3phInv g_egtmAtom3phInv;

/* ============================= ISR and Callbacks ============================= */
/* Period event callback (linked via InterruptConfig) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* User-defined period event handling (optional) */
    (void)data;
}

/* ISR: Use EXACT macro name and priority symbol as per reference */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ============================= Initialization ============================= */
/*
 * Initialize the eGTM for 3-phase complementary PWM using ATOM with DTM dead-time.
 * Behavior: enable module + clocks, configure unified PWM for ATOM1 CH0/1/2,
 *            center-aligned @20kHz, 1us dead-time, complementary outputs,
 *            HS active-high, LS active-low, push-pull, cmosAutomotiveSpeed1.
 * Note: Using unified IfxEgtm_Pwm driver. SyncStart and SyncUpdate enabled via config.
 */
void initEgtmAtom3phInv(void)
{
    /* Mandatory EGTM module enable + CMU clock setup (must be first) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, frequency);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* Configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output configuration: complementary HS/LS with required polarity and pad */
    /* Phase U on ATOM1 CH0 */
    output[0].pin                        = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin           = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                   = Ifx_ActiveState_high;                  /* HS active-high */
    output[0].complementaryPolarity      = Ifx_ActiveState_low;                   /* LS active-low */
    output[0].outputMode                 = IfxPort_OutputMode_pushPull;
    output[0].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V on ATOM1 CH1 */
    output[1].pin                        = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin           = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                   = Ifx_ActiveState_high;
    output[1].complementaryPolarity      = Ifx_ActiveState_low;
    output[1].outputMode                 = IfxPort_OutputMode_pushPull;
    output[1].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W on ATOM1 CH2 */
    output[2].pin                        = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin           = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                   = Ifx_ActiveState_high;
    output[2].complementaryPolarity      = Ifx_ActiveState_low;
    output[2].outputMode                 = IfxPort_OutputMode_pushPull;
    output[2].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time per channel: 1 us rising/falling */
    dtmConfig[0].deadTime.rising         = DEAD_TIME_RISING_S;
    dtmConfig[0].deadTime.falling        = DEAD_TIME_FALLING_S;
    dtmConfig[1].deadTime.rising         = DEAD_TIME_RISING_S;
    dtmConfig[1].deadTime.falling        = DEAD_TIME_FALLING_S;
    dtmConfig[2].deadTime.rising         = DEAD_TIME_RISING_S;
    dtmConfig[2].deadTime.falling        = DEAD_TIME_FALLING_S;

    /* Interrupt configuration: period event callback on base channel */
    interruptConfig.mode                 = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider          = IfxSrc_Tos_cpu0;
    interruptConfig.priority             = ISR_PRIORITY_ATOM;
    interruptConfig.vmId                 = IfxSrc_VmId_0;
    interruptConfig.periodEvent          = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent            = NULL_PTR;

    /* Channel configurations (contiguous set: base CH0 + sync CH1/CH2) */
    /* CH0 - Phase U */
    channelConfig[0].timerCh             = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase               = 0.0f;
    channelConfig[0].duty                = PHASE_U_DUTY;
    channelConfig[0].dtm                 = &dtmConfig[0];
    channelConfig[0].output              = &output[0];
    channelConfig[0].mscOut              = NULL_PTR;
    channelConfig[0].interrupt           = &interruptConfig;      /* base channel interrupt */

    /* CH1 - Phase V */
    channelConfig[1].timerCh             = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase               = 0.0f;
    channelConfig[1].duty                = PHASE_V_DUTY;
    channelConfig[1].dtm                 = &dtmConfig[1];
    channelConfig[1].output              = &output[1];
    channelConfig[1].mscOut              = NULL_PTR;
    channelConfig[1].interrupt           = NULL_PTR;

    /* CH2 - Phase W */
    channelConfig[2].timerCh             = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase               = 0.0f;
    channelConfig[2].duty                = PHASE_W_DUTY;
    channelConfig[2].dtm                 = &dtmConfig[2];
    channelConfig[2].output              = &output[2];
    channelConfig[2].mscOut              = NULL_PTR;
    channelConfig[2].interrupt           = NULL_PTR;

    /* Main PWM configuration */
    config.cluster                       = IfxEgtm_Cluster_1;                     /* EGTM_CLUSTER_INDEX = 1 */
    config.subModule                     = IfxEgtm_Pwm_SubModule_atom;            /* ATOM submodule */
    config.alignment                     = IfxEgtm_Pwm_Alignment_center;          /* center-aligned */
    config.syncStart                     = TIMING_SYNCSTART;                       /* TRUE */
    config.syncUpdateEnabled             = TIMING_SYNCUPDATE;                      /* TRUE */
    config.numChannels                   = NUM_OF_CHANNELS;
    config.channels                      = channelConfig;
    config.frequency                     = PWM_FREQUENCY_HZ;                       /* 20 kHz */
    config.clockSource.atom              = IfxEgtm_Cmu_Clk_0;                      /* ATOM clock */
    config.dtmClockSource                = IfxEgtm_Dtm_ClockSource_cmuClock0;      /* DTM clock */

    /* Initialize the PWM driver (unified high-level) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* Store runtime values for updates (after init) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* Configure LED pin for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ============================= Runtime Update ============================= */
/*
 * Duty-cycle ramp update for 3 phases. For each duty: if (duty + STEP) >= 100, wrap to 0, then add STEP.
 * Apply the new duties via unified driver immediate update (sync handled internally by driver config).
 */
void updateEgtmAtom3phInvDuty(void)
{
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

    /* Update all channels immediately; unified driver maintains sync with shadow registers */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
