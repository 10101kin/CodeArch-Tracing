/*
 * egtm_atom_3_phase_inverter_pwm.c
 * TC4xx eGTM ATOM 3-Phase Inverter PWM - Production Source
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

/* Driver/application state structure for TC4xx (EGTM ATOM) */
typedef struct {
    IfxEgtm_Pwm          pwm;                             /* PWM Driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];       /* Channel data after init */
    float32              dutyCycles[NUM_OF_CHANNELS];     /* Duty cycle values (%) */
    float32              phases[NUM_OF_CHANNELS];         /* Phase shift values (deg) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];      /* Dead time per channel */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;  /* Internal driver instance */

/* ISR declaration using reference macro name ISR_PRIORITY_ATOM */
IFX_INTERRUPT(EgtmAtomIsr_periodEvent, 0, ISR_PRIORITY_ATOM);
void EgtmAtomIsr_periodEvent(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback for unified driver interrupt configuration */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* No data; ISR toggles LED */
}

void initEgtmAtom3phInv(void)
{
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* Mandatory eGTM enable + CMU clock setup BEFORE driver init */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
        /* Select GCLK as input for CLK0 (ATOM uses Clk_0) */
        IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);
        /* Enable CLK0 */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Interrupt configuration: period event on base channel, CPU0 priority 20 */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Output configuration: complementary outputs, active-high HS and active-low LS */
    /* Phase U (Channel 0) */
    output[0].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS; /* TBD mapping */
    output[0].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS; /* TBD mapping */
    output[0].polarity                 = Ifx_ActiveState_high;
    output[0].complementaryPolarity    = Ifx_ActiveState_low;
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (Channel 1) */
    output[1].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS; /* TBD mapping */
    output[1].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS; /* TBD mapping */
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_low;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (Channel 2) */
    output[2].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS; /* TBD mapping */
    output[2].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS; /* TBD mapping */
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_low;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: 1.0 us rising and falling for each channel */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* Channel configurations: contiguous ATOM1 CH0/1/2, base channel 0 with ISR */
    /* Channel 0 - Phase U */
    /* Optional: IfxEgtm_Pwm_initChannelConfig(&channelConfig[0]); */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    /* Channel 1 - Phase V */
    /* Optional: IfxEgtm_Pwm_initChannelConfig(&channelConfig[1]); */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2 - Phase W */
    /* Optional: IfxEgtm_Pwm_initChannelConfig(&channelConfig[2]); */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Main PWM configuration fields */
    config.cluster              = IfxEgtm_Cluster_0;
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;
    config.alignment            = IfxEgtm_Pwm_Alignment_center;  /* center-aligned */
    config.syncStart            = TRUE;                          /* start after init */
    config.syncUpdateEnabled    = TRUE;                          /* shadow update */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                 /* 20 kHz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;             /* ATOM uses Clk enum */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM from CMU CLK0 */

    /* Initialize PWM driver (unified) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store initial runtime values for updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* Configure LED pin for ISR heartbeat */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Install ISR handler (routes to vector; ISR also declared via IFX_INTERRUPT) */
    IfxCpu_Irq_installInterruptHandler((void*)EgtmAtomIsr_periodEvent, ISR_PRIORITY_ATOM);

    /* Do NOT call start/stop or other redundant APIs here: syncStart handles channel start */
}

void updateEgtmAtom3phInvDuty(void)
{
    /* Independent duty ramp with wrap-around at 100% */
    uint8 i;
    for (i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        if ((g_egtmAtom3phInv.dutyCycles[i] + PHASE_DUTY_STEP) >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
    }

    for (i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_egtmAtom3phInv.dutyCycles[i] += PHASE_DUTY_STEP;
    }

    /* Apply new duties immediately to running PWM */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
