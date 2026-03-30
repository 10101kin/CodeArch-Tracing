/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for TC4xx eGTM ATOM-based 3-phase complementary PWM using IfxEgtm_Pwm.
 *
 * Implements:
 *  - initEgtmAtom3phInv
 *  - updateEgtmAtom3phInvDuty
 *  - interruptEgtmAtom (ISR)
 *  - IfxEgtm_periodEventFunction (empty callback)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros (constants) ========================= */
#define NUM_OF_CHANNELS          (3u)
#define PWM_FREQUENCY            (20000.0f)      /* 20 kHz */
#define ISR_PRIORITY_ATOM        (20)

#define PHASE_U_DUTY             (25.0f)
#define PHASE_V_DUTY             (50.0f)
#define PHASE_W_DUTY             (75.0f)
#define PHASE_DUTY_STEP          (10.0f)

/* LED macro: compound form (port, pin). User requirement: toggle P03.9 on period ISR */
#define LED                      &MODULE_P03, 9

/* ========================= Pin routing macros ========================= */
/* Use ONLY validated pin symbols. For user-requested pins not present in the
 * validated list, keep NULL_PTR placeholders to be replaced during integration.
 *
 * Requested mapping (KIT_A3G_TC4D7_LITE):
 *  U HS = P20.8 (TOUT64)  -> placeholder (not in validated list)
 *  U LS = P20.9 (TOUT65)  -> available below
 *  V HS = P21.4 (TOUT55)  -> placeholder (not in validated list)
 *  V LS = P20.11 (TOUT67) -> placeholder (not in validated list)
 *  W HS = P20.12 (TOUT68) -> placeholder (not in validated list)
 *  W LS = P20.13 (TOUT69) -> placeholder (not in validated list)
 */
#define PHASE_U_HS   (NULL_PTR)                              /* Replace with &IfxEgtm_ATOM0_x_TOUT64_P20_8_OUT */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)    /* Validated */
#define PHASE_V_HS   (NULL_PTR)                              /* Replace with &IfxEgtm_ATOM0_x_TOUT55_P21_4_OUT */
#define PHASE_V_LS   (NULL_PTR)                              /* Replace with &IfxEgtm_ATOM0_x_TOUT67_P20_11_OUT */
#define PHASE_W_HS   (NULL_PTR)                              /* Replace with &IfxEgtm_ATOM0_x_TOUT68_P20_12_OUT */
#define PHASE_W_LS   (NULL_PTR)                              /* Replace with &IfxEgtm_ATOM0_x_TOUT69_P20_13_OUT */

/* ========================= Module-scoped state ========================= */
typedef struct
{
    IfxEgtm_Pwm            pwm;                                 /* Driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];           /* Persistent channel handles */
    float32                dutyCycles[NUM_OF_CHANNELS];          /* Duty in percent */
    float32                phases[NUM_OF_CHANNELS];              /* Phase in fraction [0..1) */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];           /* Dead-times for each channel */
} EGTM_Atom3phInv_State;

IFX_STATIC EGTM_Atom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback ========================= */
/* ISR: minimal body, toggle LED only. Priority provided via InterruptConfig. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback required by high-level driver. Must be empty. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Private helpers (none) ========================= */

/* ========================= Public API ========================= */
/** \brief Initialize a 3-channel complementary, center-aligned PWM for a three-phase inverter
 *         on eGTM ATOM0, Cluster 0 using the unified high-level driver.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as local variables */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load driver defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure OutputConfig for complementary pairs (CH0->V, CH1->U, CH2->W) */
    /* Phase V (channel 0) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[0].polarity              = Ifx_ActiveState_high;  /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;   /* LS active low */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase U (channel 1) */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (channel 2) */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Configure DTM dead-time per channel: 1 us rising/falling */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: period event, CPU0, priority 20 */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = &IfxEgtm_periodEventFunction;  /* callback required by driver */
    interruptConfig.dutyEvent   = NULL_PTR;                      /* not used */

    /* 6) Fill ChannelConfig[0..2] with logical channels and mapping CH0->V, CH1->U, CH2->W */
    /* CH0 -> Phase V */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_V_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;              /* base channel owns interrupt */

    /* CH1 -> Phase U */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_U_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;                      /* only base channel has interrupt */

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main configuration fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;         /* ATOM submodule */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;       /* center-aligned */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;          /* ATOM uses CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;  /* DTM clock */
    config.syncUpdateEnabled  = TRUE;                                /* sync update */
    config.syncStart          = TRUE;                                /* sync start */

    /* 8) eGTM enable guard + CMU clocks (keep all inside this guard) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        /* ATOM submodule requires CLK0 enable */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver (applies channel config, duties, phases, dead-times, pin routing) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist runtime state for later updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;  /* V */
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;  /* U */
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;  /* W */

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (used by ISR). No state set required. */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/** \brief Periodic duty-cycle stepper with wrap-at-100 behavior. */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap checks (separate ifs as specified) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    /* Unconditional step add */
    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply synchronously to all complementary pairs (percent values) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
