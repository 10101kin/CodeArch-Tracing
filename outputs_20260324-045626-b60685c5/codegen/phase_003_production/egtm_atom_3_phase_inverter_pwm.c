/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for TC4xx EGTM ATOM 3-Phase Inverter PWM using unified IfxEgtm_Pwm driver.
 *
 * Notes:
 * - Follows unified driver init pattern: initConfig -> populate -> init
 * - Pin routing via IfxEgtm_Pwm_OutputConfig (no direct PinMap calls)
 * - Mandatory EGTM CMU enable block included before driver init
 * - ISR installed via channelConfig interrupt settings; IFX_INTERRUPT toggles LED
 * - No watchdog API in this file (watchdog handled only in Cpu0_Main.c)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxCpu.h"  /* For IFX_INTERRUPT macro */
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* Global driver instance */
EgtmAtom3phInv g_egtmAtom3phInv;

/* ISR: toggle LED each PWM period event */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback (optional; kept for unified driver completeness) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Application-specific period callback can be added here if needed */
    (void)data;
}

/* Initialize the eGTM-based 3-phase inverter PWM */
void initEgtmAtom3phInv(void)
{
    /* Mandatory EGTM module enable + CMU clock setup BEFORE PWM init */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
        /* Configure ATOM CLK0 to module frequency (requirement: enable CLK0) */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, frequency);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* Local configuration containers */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;

    /* Initialize config defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Interrupt configuration: period event on base channel, CPU0, priority 20 */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Output configuration for complementary PWM via DTM */
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

    /* Dead-time configuration: 1 us rising/falling for each phase */
    dtmConfig[0].deadTime.rising  = DEAD_TIME_RISING_S;
    dtmConfig[0].deadTime.falling = DEAD_TIME_FALLING_S;
    dtmConfig[1].deadTime.rising  = DEAD_TIME_RISING_S;
    dtmConfig[1].deadTime.falling = DEAD_TIME_FALLING_S;
    dtmConfig[2].deadTime.rising  = DEAD_TIME_RISING_S;
    dtmConfig[2].deadTime.falling = DEAD_TIME_FALLING_S;

    /* Channel configuration: ATOM0 channels 0/1/2, phase=0, independent duties */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* Period ISR on base channel */

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Unified driver top-level configuration */
    config.cluster              = IfxEgtm_Cluster_1;                    /* Requirement: Cluster_1 */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;           /* ATOM submodule */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;         /* Center-aligned */
    config.syncStart            = TRUE;                                 /* Auto-start synced */
    config.syncUpdateEnabled    = TRUE;                                 /* Shadow updates */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = channelConfig;
    config.frequency            = PWM_FREQUENCY;                        /* 20 kHz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                    /* ATOM uses Clk_0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;    /* DTM uses CLK0 */

    /* Initialize the PWM driver (reads all config and starts due to syncStart=TRUE) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* Store initial runtime values for updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* Configure LED GPIO and drive to known state (low) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(LED, IfxPort_State_low);
}

/* Runtime duty ramp with wrap-around, then immediate multi-channel update */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap if next step would reach or exceed 100% */
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

    /* Apply step */
    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate coherent update across channels (syncUpdate honored by config) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.dutyCycles);
}
