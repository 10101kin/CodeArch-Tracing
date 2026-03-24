/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * TC4xx eGTM ATOM 3-Phase Inverter PWM - Production Source
 *
 * Implements exact unified-driver initialization and runtime update per the
 * authoritative iLLD patterns and SW Detailed Design. No watchdog handling is
 * performed here (must be in CpuN_Main.c only).
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxSrc.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* Driver state structure as mandated for TC4xx (EGTM ATOM) */
typedef struct {
    IfxEgtm_Pwm          pwm;                             /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];       /* Channel data post-init */
    float32              dutyCycles[NUM_OF_CHANNELS];     /* Duty cycle values (percent) */
    float32              phases[NUM_OF_CHANNELS];         /* Phase shift values (radians or percent of period as per use) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];      /* Dead-time values (rising/falling) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv; /* Internal driver instance */

/* ISR: Period event on ATOM, toggles LED per requirements */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback (linked via interrupt config) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Callback hook for period event if needed; ISR already toggles LED. */
    (void)data;
}

/* Initialization function: eGTM-based 3-phase inverter PWM */
void initEgtmAtom3phInv(void)
{
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    /* Mandatory EGTM enable + CMU clock setup BEFORE init */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);
        /* Use module frequency for GCLK and CLK0 to ensure known-good clocking */
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, frequency);
        /* Enable FXCLK and CLK0 as required */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* Initialize unified PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Interrupt configuration: period event on base channel, CPU0, priority per requirements */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Output configuration for complementary high-/low-side per phase */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration for each channel (1 us rising, 1 us falling) */
    dtmConfig[0].deadTime.rising  = DEAD_TIME_RISING;
    dtmConfig[0].deadTime.falling = DEAD_TIME_FALLING;
    dtmConfig[1].deadTime.rising  = DEAD_TIME_RISING;
    dtmConfig[1].deadTime.falling = DEAD_TIME_FALLING;
    dtmConfig[2].deadTime.rising  = DEAD_TIME_RISING;
    dtmConfig[2].deadTime.falling = DEAD_TIME_FALLING;

    /* Channel configuration: base + 2 sync channels, independent duty init */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig; /* Period ISR on base channel */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* Top-level PWM config fields */
    config.cluster            = IfxEgtm_Cluster_1;                  /* Cluster_1 per requirement */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;         /* ATOM submodule */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;       /* Center-aligned */
    config.syncStart          = TRUE;                                /* Start all channels synchronously */
    config.syncUpdateEnabled  = TRUE;                                /* Use shadow updates */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = channelConfig;                       /* Link channel configs */
    config.frequency          = PWM_FREQUENCY;                       /* 20 kHz */
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                   /* ATOM uses Clk enum */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;   /* DTM via CMU CLK0 */

    /* Initialize the PWM driver (unified driver reads all config and starts if syncStart=TRUE) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* Store initial runtime values (for coherent updates) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* Configure LED GPIO (P13.0) for ISR toggling and drive to known state */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(LED, IfxPort_State_low);
}

/* Runtime update: ramp duties with wrap-around, then update immediately */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-around check then increment per channel */
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

    /* Immediate multi-channel duty update (coherent with syncUpdate) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
