/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production implementation using unified IfxEgtm_Pwm driver
 */
#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* ----------------------------------------------------------------------------
 * Global driver instance (module-scope)
 * ---------------------------------------------------------------------------- */
IFX_STATIC EgtmAtom3phInv g_egtmAtom3phInv;

/* ----------------------------------------------------------------------------
 * ISR declaration and implementation (toggle LED on each PWM period)
 * ---------------------------------------------------------------------------- */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ----------------------------------------------------------------------------
 * Period event callback (linked in interrupt config; can be used for hooks)
 * ---------------------------------------------------------------------------- */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* No-op hook; ISR does the LED toggle */
}

/* ----------------------------------------------------------------------------
 * Initialization
 * ---------------------------------------------------------------------------- */
void initEgtmAtom3phInv(void)
{
    /* Mandatory EGTM module enable and CMU clock setup (must be first) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
        /* Configure CMU CLK0 to module frequency */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, frequency);
        /* Enable FXCLK and CLK0 as per requirements */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* Configuration containers */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;

    /* Initialize config defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Interrupt configuration: period event on base channel, CPU0, prio 20 */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Output configuration (complementary pairs via DTM) */
    output[0].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                 = Ifx_ActiveState_high;
    output[0].complementaryPolarity    = Ifx_ActiveState_low;
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_low;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_low;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration for each channel */
    dtmConfig[0].deadTime.rising  = DEADTIME_RISING_SEC;
    dtmConfig[0].deadTime.falling = DEADTIME_FALLING_SEC;
    dtmConfig[1].deadTime.rising  = DEADTIME_RISING_SEC;
    dtmConfig[1].deadTime.falling = DEADTIME_FALLING_SEC;
    dtmConfig[2].deadTime.rising  = DEADTIME_RISING_SEC;
    dtmConfig[2].deadTime.falling = DEADTIME_FALLING_SEC;

    /* Channel configurations: ATOM0 CH0/1/2, phase=0, independent duty */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;   /* Period ISR on base channel */

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
    config.cluster            = IfxEgtm_Cluster_1;                  /* Cluster_1 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;         /* ATOM */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;       /* Center-aligned */
    config.syncStart          = TRUE;                               /* Start on init */
    config.syncUpdateEnabled  = TRUE;                               /* Shadow updates */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                  /* ATOM clock source */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;  /* DTM clock source */

    /* Initialize the unified PWM driver (no return value) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store initial runtime copies of duty and dead-time values */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* Configure LED pin as push-pull output, initial state LOW */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(LED, IfxPort_State_low);
}

/* ----------------------------------------------------------------------------
 * Runtime duty update (ramp with wrap-around) and immediate apply
 * ---------------------------------------------------------------------------- */
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

    /* Immediate multi-channel duty update (coherent across channels) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
