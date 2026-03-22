/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production eGTM ATOM 3-phase inverter PWM driver implementation.
 *
 * Follows unified IfxEgtm_Pwm API with strict configuration structure usage:
 *  - IfxEgtm_Pwm_Config
 *  - IfxEgtm_Pwm_ChannelConfig
 *  - IfxEgtm_Pwm_OutputConfig
 *  - IfxEgtm_Pwm_DtmConfig
 *  - IfxEgtm_Pwm_InterruptConfig
 *
 * No watchdog operations here (must be in CpuN_Main.c only).
 */
#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* =============================
 * Internal state instance
 * ============================= */
IFX_STATIC EgtmAtom3phInv g_egtmAtom3phInv;  /* NOLINT(readability-identifier-naming) */

/* =============================
 * ISR and period event callback
 * ============================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    /* Diagnostic pulse to observe ISR activity */
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    /* Optional: user code on PWM period event (not used in this module) */
    (void)data;
}

/* =============================
 * Initialization routine
 * ============================= */
void initEgtmAtom3phInv(void)
{
    /* Mandatory eGTM module enable + CMU clock setup (before PWM init) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
        /* Enable CMU CLK0 (ATOM uses Clk enum) */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* Configuration containers (stack-local as per iLLD patterns) */
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    /* Initialize PWM config defaults for the eGTM module */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output configuration (complementary) */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;                    /* HS active-high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;                     /* LS active-low */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration (1 us both edges for all channels) */
    dtmConfig[0].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmConfig[0].deadTime.falling = DEAD_TIME_SECONDS;
    dtmConfig[1].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmConfig[1].deadTime.falling = DEAD_TIME_SECONDS;
    dtmConfig[2].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmConfig[2].deadTime.falling = DEAD_TIME_SECONDS;

    /* Interrupt configuration: attach to base channel (CH0) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel configurations: base + two sync channels (CH0/1/2) */
    /* CH0 - Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;   /* Period ISR on base channel */

    /* CH1 - Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* CH2 - Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* Main PWM configuration */
    config.cluster           = IfxEgtm_Cluster_1;                /* EGTM_CLUSTER_INDEX = 1 */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;       /* ATOM submodule */
    config.alignment         = IfxEgtm_Pwm_Alignment_center;     /* Center-aligned */
    config.syncStart         = TRUE;                              /* Start channels after init */
    config.syncUpdateEnabled = TRUE;                              /* Shadow updates */
    config.numChannels       = NUM_OF_CHANNELS;
    config.channels          = channelConfig;
    config.frequency         = PWM_FREQUENCY;                     /* 20 kHz */
    config.clockSource.atom  = IfxEgtm_Cmu_Clk_0;                 /* ATOM from CMU CLK0 */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM from CMU CLK0 */

    /* Initialize PWM driver; init returns void (no return value to check) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* Store initial runtime values for updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1] = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2] = channelConfig[2].dtm->deadTime;

    /* Configure LED pin as output for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =============================
 * Duty ramp update (runtime)
 * ============================= */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-to-zero before increment when step would reach/exceed 100% */
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

    /* Apply increment */
    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Latch new duties via immediate update API (coherent at period boundary with syncUpdate) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
