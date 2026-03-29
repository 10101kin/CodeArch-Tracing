/* ============================================================================
 *  File: egtm_atom_3_phase_inverter_pwm.c
 *  Brief: eGTM ATOM 3-phase inverter PWM driver (TC4xx, migration from TC3xx)
 *
 *  - eGTM Cluster 1, ATOM1, channels CH0..CH2
 *  - Complementary, center-aligned, 20 kHz, 1.0 us dead-time
 *  - syncStart/syncUpdate enabled
 *  - Period ISR on CPU0 with priority 20 toggles a board LED
 *
 *  Notes:
 *   - Pin routing is performed via IfxEgtm_Pwm_OutputConfig entries.
 *   - Do NOT place watchdog disable code here (CpuX_Main.c only per AURIX pattern).
 * ============================================================================ */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and Configuration Constants ========================= */

/* Number of logical PWM channels (complementary pairs) */
#define NUM_OF_CHANNELS            (3u)

/* Switching frequency (Hz) */
#define PWM_FREQUENCY              (20000.0f)

/* ISR priority for eGTM ATOM period event */
#define ISR_PRIORITY_ATOM          (20)

/* Initial duties (percent) */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)

/* Duty update step (percent) for updateEgtmAtom3phInvDuty */
#define PHASE_DUTY_STEP            (5.0f)

/* User-requested TOUT pins (validate against PinMap for target) */
#define PHASE_U_HS                 (&IfxEgtm_ATOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                 (&IfxEgtm_ATOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                 (&IfxEgtm_ATOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                 (&IfxEgtm_ATOM1_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS                 (&IfxEgtm_ATOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS                 (&IfxEgtm_ATOM1_2N_TOUT5_P02_5_OUT)

/* LED pin used by ISR to toggle (Port13.0) */
#define LED                        &MODULE_P13, 0

/* ============================= Module State (persistent) ============================== */

typedef struct
{
    IfxEgtm_Pwm               pwm;                                 /* Driver handle */
    IfxEgtm_Pwm_Channel       channels[NUM_OF_CHANNELS];           /* Persistent channels (required by driver) */
    float32                   dutyCycles[NUM_OF_CHANNELS];         /* Duty cycles in percent */
    float32                   phases[NUM_OF_CHANNELS];             /* Phases in percent or degrees as needed */
    IfxEgtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];          /* Dead-time per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ============================ Private ISR and Callbacks ============================== */

/* ISR: toggles LED only (minimal work in ISR) */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Optional private helper (not used by driver, kept for completeness) */
void EgtmAtomPeriodIsr(void)
{
    /* Not used; ISR is interruptEgtmAtom via vector. Intentionally empty. */
}

/* Period event callback: must be empty body by design */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ================================ Public Functions =================================== */

/**
 * Initialize a 3-phase inverter PWM on eGTM Cluster 1 ATOM1 with complementary, center-aligned
 * outputs at 20 kHz, 1.0 us dead-time, synchronous start/update, and a period ISR on CPU0 with
 * priority 20.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as local variables */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load defaults into the main config */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Populate output for three logical channels (one per phase) */
    /* Phase U */
    output[0].pin                     = PHASE_U_HS;
    output[0].complementaryPin        = PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;  /* HS active-high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;   /* LS active-low  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = PHASE_V_HS;
    output[1].complementaryPin        = PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = PHASE_W_HS;
    output[2].complementaryPin        = PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Set dead-time for each channel: 1.0 us rising/falling */
    dtmConfig[0].deadTime.rising = 1e-6f; dtmConfig[0].deadTime.falling = 1e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1e-6f; dtmConfig[1].deadTime.falling = 1e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1e-6f; dtmConfig[2].deadTime.falling = 1e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration: period event on base channel */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration for three logical channels (SubModule_Ch_0..2) */
    /* Channel 0 → Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;          /* percent */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;      /* Base channel gets interrupt */

    /* Channel 1 → Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;          /* percent */
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;              /* No ISR on this channel */

    /* Channel 2 → Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;          /* percent */
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;              /* No ISR on this channel */

    /* 7) Main configuration */
    config.cluster              = IfxEgtm_Cluster_1;                 /* eGTM Cluster 1 */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;        /* ATOM submodule */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;      /* center-aligned */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                     /* 20 kHz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                 /* ATOM CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM CMU CLK0 */
    config.syncUpdateEnabled    = TRUE;
    config.syncStart            = TRUE;

    /* 8) eGTM enable guard and CMU clock setup (MANDATORY pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver with persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initial state into persistent module-state struct */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (no forced state) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duty cycles by a fixed percent increment and apply immediately.
 * Sequence per phase: if (duty + STEP) >= 100 → duty = 0; then add STEP; finally call the
 * immediate multi-channel update API with the state's duty array (percent values).
 */
void updateEgtmAtom3phInvDuty(void)
{
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
