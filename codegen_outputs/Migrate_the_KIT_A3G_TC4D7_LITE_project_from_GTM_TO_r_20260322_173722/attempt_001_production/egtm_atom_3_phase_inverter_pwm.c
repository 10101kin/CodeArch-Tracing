/*
 * egtm_atom_3_phase_inverter_pwm.c
 * TC4xx eGTM ATOM 3-Phase Inverter PWM (production)
 */
#include "egtm_atom_3_phase_inverter_pwm.h"

#include "IfxEgtm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

/* =========================
 * Driver state structures
 * ========================= */
typedef struct {
    IfxEgtm_Pwm          pwm;                            /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];      /* Channel data after init */
    float32              dutyCycles[NUM_OF_CHANNELS];    /* Duty cycle values (%) */
    float32              phases[NUM_OF_CHANNELS];        /* Phase shift values */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];     /* Dead time values */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;
volatile EgtmAtom3phInv_Status g_egtmAtom3phInvStatus = {
    TRUE,   /* basicStatusEnabled */
    FALSE,  /* clk0Enabled */
    FALSE,  /* dtmClkFromClk0 */
    0U,     /* isrMissedCount */
    0U,     /* lastErrorCode */
    0U      /* phaseEnabledMask */
};

/* =========================
 * ISR (period event route)
 * ========================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
    /* Optional: status/diagnostic updates can be added here if needed */
}

/* =========================
 * Period event callback
 * ========================= */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* not used */
    /* Called on PWM period event; application-specific handling can be placed here. */
}

/* =========================
 * Initialization
 * ========================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Enable eGTM and CMU CLK0 (100 MHz) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Set GCLK to module frequency (assume clock tree already at expected system value) */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
    }
    /* Select CLK0 to use GCLK and enable CLK0 */
    IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);
    IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);

    /* Mark status flags */
    g_egtmAtom3phInvStatus.clk0Enabled  = TRUE;
    g_egtmAtom3phInvStatus.dtmClkFromClk0 = TRUE;

    /* 2) Prepare unified PWM configuration */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Interrupt configuration (period event on base channel, CPU0, prio 20) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 4) Output configuration (complementary) - pins left TBD (NULL) per requirements */
    /* Phase V (index 0) */
    output[0].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS; /* TBD */
    output[0].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS; /* TBD */
    output[0].polarity                  = Ifx_ActiveState_high;
    output[0].complementaryPolarity     = Ifx_ActiveState_low;
    output[0].outputMode                = IfxPort_OutputMode_pushPull;
    output[0].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase U (index 1) */
    output[1].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS; /* TBD */
    output[1].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS; /* TBD */
    output[1].polarity                  = Ifx_ActiveState_high;
    output[1].complementaryPolarity     = Ifx_ActiveState_low;
    output[1].outputMode                = IfxPort_OutputMode_pushPull;
    output[1].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (index 2) */
    output[2].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS; /* TBD */
    output[2].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS; /* TBD */
    output[2].polarity                  = Ifx_ActiveState_high;
    output[2].complementaryPolarity     = Ifx_ActiveState_low;
    output[2].outputMode                = IfxPort_OutputMode_pushPull;
    output[2].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) Dead-time configuration: 1 us rising/falling (hardware complementary via DTM) */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 6) Channel configuration: EGTM0.ATOM0 using pairs: U=0/1, V=2/3, W=4/5 */
    /* Base channel CH0 (index 0: V phase in reference pattern) */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_V_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* period ISR on base channel */

    /* Sync channel CH2 (index 1: U phase) */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_U_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Sync channel CH4 (index 2: W phase) */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_4;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main PWM configuration fields */
    config.cluster              = IfxEgtm_Cluster_0;                    /* EGTM0 */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;           /* ATOM */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;         /* center-aligned */
    config.syncStart            = TRUE;                                  /* auto-start after init */
    config.syncUpdateEnabled    = TRUE;                                  /* shadow update on period */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                         /* 20 kHz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                     /* CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;     /* DTM from CLK0 */

    /* 8) Initialize PWM driver (unified). Do not call redundant post-init APIs. */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 9) Store runtime values */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]     = channelConfig[2].phase;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* 10) Install ISR on CPU0 with required priority */
    IfxCpu_Irq_installInterruptHandler((void*)interruptEgtmAtom, (uint32)ISR_PRIORITY_ATOM);

    /* 11) Configure LED pin for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 12) Initialize status fields */
    g_egtmAtom3phInvStatus.isrMissedCount = 0U;
    g_egtmAtom3phInvStatus.lastErrorCode  = 0U;
    g_egtmAtom3phInvStatus.phaseEnabledMask = 0x07U; /* U/V/W enabled */
}

/* =========================
 * Runtime duty update
 * ========================= */
void updateEgtmAtom3phInvDuty(void)
{
    /* Step-and-wrap each phase duty per design */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        float32 next = g_egtmAtom3phInv.dutyCycles[i] + PHASE_DUTY_STEP;
        if (next >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
    }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately; complementary outputs handled by hardware DTM */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
