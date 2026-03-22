/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production 3-phase inverter PWM driver using eGTM ATOM unified iLLD (TC4xx)
 */
#include "egtm_atom_3_phase_inverter_pwm.h"

#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm.h"
#include "IfxEgtm_PinMap.h"   /* Generic PinMap header - family-agnostic */
#include "IfxPort.h"
#include "IfxCpu.h"           /* For IFX_INTERRUPT macro */
#include "IfxSrc.h"           /* For IfxSrc_Tos_cpu0 */

/* Proposed pin mapping from requirements (verify availability for TC4D7 in IfxEgtm_PinMap.h) */
#define PHASE_U_HS   &IfxEgtm_ATOM1_0_TOUT0_P02_0_OUT
#define PHASE_U_LS   &IfxEgtm_ATOM1_0N_TOUT7_P02_7_OUT
#define PHASE_V_HS   &IfxEgtm_ATOM1_1_TOUT1_P02_1_OUT
#define PHASE_V_LS   &IfxEgtm_ATOM1_1N_TOUT4_P02_4_OUT
#define PHASE_W_HS   &IfxEgtm_ATOM1_2_TOUT2_P02_2_OUT
#define PHASE_W_LS   &IfxEgtm_ATOM1_2N_TOUT5_P02_5_OUT

/* Driver instance structure */
typedef struct {
    IfxEgtm_Pwm           pwm;                            /* PWM Driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];      /* Channel data after init */
    float32               dutyCycles[NUM_OF_CHANNELS];    /* Duty cycle values (percent) */
    float32               phases[NUM_OF_CHANNELS];        /* Phase shift values (if used) */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];     /* Dead-time values */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;

/* ISR: use exact macro name/pattern */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback (linked via interrupt config if needed) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Application-specific period event handling (optional) */
    (void)data;
}

void initEgtmAtom3phInv(void)
{
    /* Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* Mandatory eGTM enable + CMU clock setup BEFORE PWM init */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        /* Enable CLK0 for ATOM clocking; ATOM clock source selected via config.clockSource.atom */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Interrupt configuration (period event) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Output configuration per phase: complementary high/low with dead-time */
    output[0].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                  = Ifx_ActiveState_high;   /* HS active-high */
    output[0].complementaryPolarity     = Ifx_ActiveState_low;    /* LS active-low */
    output[0].outputMode                = IfxPort_OutputMode_pushPull;
    output[0].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                  = Ifx_ActiveState_high;
    output[1].complementaryPolarity     = Ifx_ActiveState_low;
    output[1].outputMode                = IfxPort_OutputMode_pushPull;
    output[1].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                  = Ifx_ActiveState_high;
    output[2].complementaryPolarity     = Ifx_ActiveState_low;
    output[2].outputMode                = IfxPort_OutputMode_pushPull;
    output[2].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: 1us rising/falling for all channels */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* Channel configurations: base + synchronous channels on ATOM1 Ch0/1/2 */
    channelConfig[0].timerCh      = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase        = 0.0f;
    channelConfig[0].duty         = PHASE_U_DUTY;
    channelConfig[0].dtm          = &dtmConfig[0];
    channelConfig[0].output       = &output[0];
    channelConfig[0].mscOut       = NULL_PTR;
    channelConfig[0].interrupt    = &interruptConfig;   /* Period ISR on base channel */

    channelConfig[1].timerCh      = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase        = 0.0f;
    channelConfig[1].duty         = PHASE_V_DUTY;
    channelConfig[1].dtm          = &dtmConfig[1];
    channelConfig[1].output       = &output[1];
    channelConfig[1].mscOut       = NULL_PTR;
    channelConfig[1].interrupt    = NULL_PTR;

    channelConfig[2].timerCh      = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase        = 0.0f;
    channelConfig[2].duty         = PHASE_W_DUTY;
    channelConfig[2].dtm          = &dtmConfig[2];
    channelConfig[2].output       = &output[2];
    channelConfig[2].mscOut       = NULL_PTR;
    channelConfig[2].interrupt    = NULL_PTR;

    /* Main PWM configuration */
    config.cluster           = IfxEgtm_Cluster_1;                 /* EGTM_CLUSTER_INDEX = 1 */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;       /* Use ATOM */
    config.alignment         = IfxEgtm_Pwm_Alignment_center;     /* Center aligned */
    config.syncStart         = TRUE;                              /* TIMING_SYNCSTART */
    config.syncUpdateEnabled = TRUE;                              /* TIMING_SYNCUPDATE */
    config.numChannels       = NUM_OF_CHANNELS;
    config.channels          = channelConfig;
    config.frequency         = PWM_FREQUENCY;
    config.clockSource.atom  = IfxEgtm_Cmu_Clk_0;                 /* ATOM clock: CMU CLK0 */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock source */

    /* Initialize unified PWM driver - after this, channels start and outputs enable as per config */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* Store initial runtime values for updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* Configure diagnostic LED pin */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(void)
{
    /* Ramp duties with wrap-around at 100% (percent domain) */
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

    /* Immediate multi-channel duty update; sync behavior is handled by shadow registers */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
