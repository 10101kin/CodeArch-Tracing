/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver for TC4xx EGTM ATOM 3-phase complementary PWM using IfxEgtm_Pwm
 * - 3 channels (complementary pairs), center-aligned, sync start/update
 * - Period-event ISR on base channel (ch0) toggles LED
 * - Dead-time: 1 us rising/falling
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ==========================
 * Configuration Macros
 * ========================== */
#define NUM_OF_CHANNELS           (3U)
#define PWM_FREQUENCY             (20000.0f)
#define ISR_PRIORITY_ATOM         (20)

/* Phase pin mapping (validated/user-specified) */
#define PHASE_U_HS                (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* Initial duty cycles (percent) and step */
#define PHASE_U_DUTY              (25.0f)
#define PHASE_V_DUTY              (50.0f)
#define PHASE_W_DUTY              (75.0f)
#define PHASE_DUTY_STEP           (10.0f)

/* LED pin: P03.9 (compound macro expands to two args: port, pin) */
#define LED                       &MODULE_P03, 9

/* ==========================
 * Module State
 * ========================== */
typedef struct
{
    IfxEgtm_Pwm                 pwm;                                 /* Driver handle */
    IfxEgtm_Pwm_Channel         channels[NUM_OF_CHANNELS];           /* Persistent channel data */
    float32                     dutyCycles[NUM_OF_CHANNELS];         /* Duty in percent */
    float32                     phases[NUM_OF_CHANNELS];             /* Phase in percent (not used: center aligned) */
    IfxEgtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];          /* Stored dead-times */
} EGTM_Atom3PhInv_State;

IFX_STATIC EGTM_Atom3PhInv_State g_egtmAtom3phInv;

/* ==========================
 * ISR and Callback
 * ========================== */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty as per design */
}

IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* ==========================
 * Public API Implementations
 * ========================== */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all config structures as locals */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];

    /* 2) Initialize the main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Build output[] for 3 complementary channels */
    /* Phase U */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;      /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;       /* LS active low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us rising/falling for each channel */
    dtmConfig[0].deadTime.rising     = 1e-6f;
    dtmConfig[0].deadTime.falling    = 1e-6f;
    dtmConfig[0].fastShutOff         = NULL_PTR;

    dtmConfig[1].deadTime.rising     = 1e-6f;
    dtmConfig[1].deadTime.falling    = 1e-6f;
    dtmConfig[1].fastShutOff         = NULL_PTR;

    dtmConfig[2].deadTime.rising     = 1e-6f;
    dtmConfig[2].deadTime.falling    = 1e-6f;
    dtmConfig[2].fastShutOff         = NULL_PTR;

    /* 5) Interrupt configuration for base channel */
    interruptConfig.mode         = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider  = IfxSrc_Tos_cpu0;
    interruptConfig.priority     = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId         = IfxSrc_VmId_0;
    interruptConfig.periodEvent  = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent    = NULL_PTR;

    /* 6) Channel configuration: logical channel indices Ch_0..Ch_2 */
    /* CH0 -> Phase U, with interrupt */
    channelConfig[0].timerCh     = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase       = 0.0f;
    channelConfig[0].duty        = PHASE_U_DUTY;
    channelConfig[0].dtm         = &dtmConfig[0];
    channelConfig[0].output      = &output[0];
    channelConfig[0].mscOut      = NULL_PTR;
    channelConfig[0].interrupt   = &interruptConfig;

    /* CH1 -> Phase V, no interrupt */
    channelConfig[1].timerCh     = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase       = 0.0f;
    channelConfig[1].duty        = PHASE_V_DUTY;
    channelConfig[1].dtm         = &dtmConfig[1];
    channelConfig[1].output      = &output[1];
    channelConfig[1].mscOut      = NULL_PTR;
    channelConfig[1].interrupt   = NULL_PTR;

    /* CH2 -> Phase W, no interrupt */
    channelConfig[2].timerCh     = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase       = 0.0f;
    channelConfig[2].duty        = PHASE_W_DUTY;
    channelConfig[2].dtm         = &dtmConfig[2];
    channelConfig[2].output      = &output[2];
    channelConfig[2].mscOut      = NULL_PTR;
    channelConfig[2].interrupt   = NULL_PTR;

    /* 7) Complete main config */
    config.cluster               = IfxEgtm_Cluster_0;
    config.subModule             = IfxEgtm_Pwm_SubModule_atom;
    config.alignment             = IfxEgtm_Pwm_Alignment_center;
    config.syncStart             = TRUE;
    config.syncUpdateEnabled     = TRUE;
    config.frequency             = PWM_FREQUENCY;
    config.clockSource.atom      = (uint32)IfxEgtm_Cmu_Clk_0;      /* ATOM uses CMU Clk_0 */
    config.dtmClockSource        = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.numChannels           = (uint8)NUM_OF_CHANNELS;
    config.channels              = &channelConfig[0];

    /* 8) Enable-guard and CMU setup INSIDE the guard */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize the PWM driver with persistent state */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initial duties, phases, and dead-times into the persistent state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (no initial level set here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(void)
{
    /* Duty wrap/update per design: check then unconditional add */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately and synchronously across channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
