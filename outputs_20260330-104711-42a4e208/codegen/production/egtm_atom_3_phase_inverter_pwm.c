/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: EGTM ATOM 3-Phase Inverter PWM using IfxEgtm_Pwm (TC4xx).
 *
 * Design goals:
 *  - 3 complementary center-aligned PWM pairs (U, V, W) on ATOM0, Cluster 0
 *  - Frequency: 20 kHz, Dead-time: 1 us rising / 1 us falling
 *  - Initial duties: U=25%, V=50%, W=75%
 *  - Duty update step: +10% with wrap at 100%
 *  - Period-event ISR toggles LED P03.9
 *
 * Mandatory constraints:
 *  - Use IfxEgtm_Pwm high-level driver
 *  - Follow config/init patterns and CMU enable guard
 *  - No watchdog disable calls in this module
 *  - Use validated pin symbols only; if not available, leave as NULL_PTR with a comment
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration macros
 * ============================= */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY              (20000.0f)
#define ISR_PRIORITY_ATOM          (20)
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* LED macro in compound (port, pin) form for direct use in IfxPort_* APIs */
#define LED                        &MODULE_P03, 9

/* =============================
 * Pin routing macros (validated symbols only)
 * User requested pins on KIT_A3G_TC4D7_LITE:
 *  - U HS=P20.8 (TOUT64), U LS=P20.9 (TOUT65)
 *  - V HS=P21.4 (TOUT55), V LS=P20.11 (TOUT67)
 *  - W HS=P20.12 (TOUT68), W LS=P20.13 (TOUT69)
 * From the validated list, only TOUT65_P20_9 is available. Others are placeholders.
 * Replace NULL_PTR with the proper IfxEgtm_ATOM* pin symbols during integration
 * if your ADS template provides them.
 * ============================= */
#define PHASE_U_HS   (NULL_PTR)                          /* Replace with &IfxEgtm_ATOMx_y_TOUT64_P20_8_OUT */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS   (NULL_PTR)                          /* Replace with &IfxEgtm_ATOMx_y_TOUT55_P21_4_OUT */
#define PHASE_V_LS   (NULL_PTR)                          /* Replace with &IfxEgtm_ATOMx_y_TOUT67_P20_11_OUT */
#define PHASE_W_HS   (NULL_PTR)                          /* Replace with &IfxEgtm_ATOMx_y_TOUT68_P20_12_OUT */
#define PHASE_W_LS   (NULL_PTR)                          /* Replace with &IfxEgtm_ATOMx_y_TOUT69_P20_13_OUT */

/* =============================
 * Module state
 * ============================= */
typedef struct
{
    IfxEgtm_Pwm            pwm;                                   /* Driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];             /* Persistent channel state array */
    float32                dutyCycles[NUM_OF_CHANNELS];            /* Duty in percent [0..100] */
    float32                phases[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];            /* Per-channel dead-time copy */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =============================
 * ISR and callback declarations
 * ============================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/*
 * Empty period-event callback required by the high-level PWM driver.
 * All observable ISR activity happens in interruptEgtmAtom().
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * ISR implementation
 * ============================= */
void interruptEgtmAtom(void)
{
    /* Minimal ISR: toggle LED to indicate PWM period */
    IfxPort_togglePin(LED);
}

/* =============================
 * Initialization
 * ============================= */
/**
 * Initialize a 3-channel complementary, center-aligned PWM for a three-phase inverter on eGTM ATOM0, Cluster 0.
 * Algorithm per SW Detailed Design (steps 1..11).
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load driver defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration for three complementary pairs (CH0->V, CH1->U, CH2->W) */
    /* Phase V (CH0) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[0].polarity              = Ifx_ActiveState_high;      /* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;       /* LS active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase U (CH1) */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (CH2) */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM dead-time configuration: 1us rising / 1us falling */
    dtmConfig[0].deadTime.rising = 1e-6f;  dtmConfig[0].deadTime.falling = 1e-6f;  dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1e-6f;  dtmConfig[1].deadTime.falling = 1e-6f;  dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1e-6f;  dtmConfig[2].deadTime.falling = 1e-6f;  dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration: period event, pulse notify, CPU0, prio 20 */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations (logical indices: 0..2) */
    /* CH0 -> Phase V, duty 50% */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_V_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* interrupt only on base channel */

    /* CH1 -> Phase U, duty 25% */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_U_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* CH2 -> Phase W, duty 75% */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main configuration fields */
    config.cluster              = IfxEgtm_Cluster_0;
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;          /* ATOM submodule */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;        /* center-aligned */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = channelConfig;
    config.frequency            = PWM_FREQUENCY;
    config.clockSource.atom     = (uint32)IfxEgtm_Cmu_Clk_0;           /* ATOM clock source: CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;   /* DTM clock */
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;

    /* 8) eGTM enable guard and CMU clock setup (ALL CMU calls inside this guard) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            /* ATOM submodule uses CLK-based clocks */
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize PWM hardware and apply configuration */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist runtime state for later updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as output (used by ISR) */
    IfxPort_setPinModeOutput(&MODULE_P03, 9, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =============================
 * Runtime duty update
 * ============================= */
/**
 * Periodic duty-cycle stepper for U, V, W by +10% with wrap at 100%.
 * Applies update immediately and synchronously to all complementary pairs.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-then-add behavior per channel (no loop, explicit per-channel statements) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately using the driver's multi-channel API */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
