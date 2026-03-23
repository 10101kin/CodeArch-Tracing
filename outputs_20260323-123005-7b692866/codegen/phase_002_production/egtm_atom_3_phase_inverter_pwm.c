#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* ==========================
 * Driver State Structure
 * ========================== */
typedef struct {
    IfxEgtm_Pwm           pwm;                                 /* PWM driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];           /* Channel data populated by init */
    float32               dutyCycles[NUM_OF_CHANNELS];         /* Duty cycle values (percent) */
    float32               phases[NUM_OF_CHANNELS];             /* Phase shift values (deg or percent, unused here) */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];          /* Dead-time values cached from config */
} EgtmAtom3phInv;

IFX_STATIC EgtmAtom3phInv g_egtmAtom3phInv;

/* ==========================
 * ISR (period event) - exact macro name and pattern
 * ========================== */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ==========================
 * Period event callback (linked via interrupt config)
 * ========================== */
void IfxEgtm_periodEventFunction(void *data)
{
    /* User period event handling (optional). Keep minimal for real-time constraints. */
    (void)data;
}

/* ==========================
 * Initialization function
 * ========================== */
void initEgtmAtom3phInv(void)
{
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    /* Mandatory eGTM enable + CMU clock setup block (must precede driver init) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);                                        /* Enable module */
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);            /* Read module freq (reference pattern) */
        (void)frequency;                                                      /* Not used; explicit frequency set below */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, EGTM_CLK0_FREQUENCY_HZ);  /* Set global clock to requirement */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, EGTM_CLK0_FREQUENCY_HZ); /* ATOM/DTM from CLK0 */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);      /* Enable CLK0 */
    }

    /* Initialize base configuration with module reference */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Interrupt configuration - single period event on CH0, CPU0, priority 20 */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;       /* Exact macro name required */
    interruptConfig.vmId        = IfxSrc_VmId_0;           /* TC4xx specific */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Output configuration per channel (complementary via CDTM) */
    /* CH0 - Phase U */
    output[0].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                  = Ifx_ActiveState_high;               /* HS active high */
    output[0].complementaryPolarity     = Ifx_ActiveState_low;                /* LS active low */
    output[0].outputMode                = IfxPort_OutputMode_pushPull;
    output[0].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* CH1 - Phase V */
    output[1].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                  = Ifx_ActiveState_high;
    output[1].complementaryPolarity     = Ifx_ActiveState_low;
    output[1].outputMode                = IfxPort_OutputMode_pushPull;
    output[1].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* CH2 - Phase W */
    output[2].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                  = Ifx_ActiveState_high;
    output[2].complementaryPolarity     = Ifx_ActiveState_low;
    output[2].outputMode                = IfxPort_OutputMode_pushPull;
    output[2].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration per channel (1 us rising/falling) */
    dtmConfig[0].deadTime.rising        = (DEADTIME_US * 1.0e-6f) / 1.0e-6f; /* 1.0 us */
    dtmConfig[0].deadTime.falling       = (DEADTIME_US * 1.0e-6f) / 1.0e-6f; /* 1.0 us */
    dtmConfig[1].deadTime.rising        = (DEADTIME_US * 1.0e-6f) / 1.0e-6f; /* 1.0 us */
    dtmConfig[1].deadTime.falling       = (DEADTIME_US * 1.0e-6f) / 1.0e-6f; /* 1.0 us */
    dtmConfig[2].deadTime.rising        = (DEADTIME_US * 1.0e-6f) / 1.0e-6f; /* 1.0 us */
    dtmConfig[2].deadTime.falling       = (DEADTIME_US * 1.0e-6f) / 1.0e-6f; /* 1.0 us */

    /* Channel configuration - CH0=U, CH1=V, CH2=W (per requirements) */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[0]);
    channelConfig[0].timerCh            = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase              = 0.0f;
    channelConfig[0].duty               = PHASE_U_DUTY;
    channelConfig[0].dtm                = &dtmConfig[0];
    channelConfig[0].output             = &output[0];
    channelConfig[0].mscOut             = NULL_PTR;
    channelConfig[0].interrupt          = &interruptConfig; /* Period ISR on base channel */

    IfxEgtm_Pwm_initChannelConfig(&channelConfig[1]);
    channelConfig[1].timerCh            = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase              = 0.0f;
    channelConfig[1].duty               = PHASE_V_DUTY;
    channelConfig[1].dtm                = &dtmConfig[1];
    channelConfig[1].output             = &output[1];
    channelConfig[1].mscOut             = NULL_PTR;
    channelConfig[1].interrupt          = NULL_PTR;        /* No ISR on CH1 */

    IfxEgtm_Pwm_initChannelConfig(&channelConfig[2]);
    channelConfig[2].timerCh            = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase              = 0.0f;
    channelConfig[2].duty               = PHASE_W_DUTY;
    channelConfig[2].dtm                = &dtmConfig[2];
    channelConfig[2].output             = &output[2];
    channelConfig[2].mscOut             = NULL_PTR;
    channelConfig[2].interrupt          = NULL_PTR;        /* No ISR on CH2 */

    /* Top-level PWM configuration */
    config.cluster                      = IfxEgtm_Cluster_0;                     /* eGTM0 */
    config.subModule                    = IfxEgtm_Pwm_SubModule_atom;            /* ATOM */
    config.alignment                    = IfxEgtm_Pwm_Alignment_center;          /* Center-aligned */
    config.syncStart                    = TRUE;                                   /* Auto start after init */
    config.syncUpdateEnabled            = TRUE;                                   /* Shadow updates */
    config.numChannels                  = NUM_OF_CHANNELS;
    config.channels                     = channelConfig;
    config.frequency                    = PWM_FREQUENCY;                          /* Hz */
    config.clockSource.atom             = IfxEgtm_Cmu_Clk_0;                      /* ATOM from CLK0 */
    config.dtmClockSource               = IfxEgtm_Dtm_ClockSource_cmuClock0;     /* DTM from CLK0 */

    /* Initialize PWM driver - unified driver performs full configuration from structs */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Cache initial runtime values for updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* Configure LED for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ==========================
 * Runtime duty update function
 * ========================== */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap if next step would reach or exceed 100%, then increment */
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

    /* Apply updated duties immediately and coherently across all channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
