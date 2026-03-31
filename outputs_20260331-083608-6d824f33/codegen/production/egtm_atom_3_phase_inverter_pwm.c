/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver: EGTM ATOM 3-Phase Inverter PWM (TC4xx, migration from TC3xx)
 * - Complementary, center-aligned PWM using IfxEgtm_Pwm (unified driver)
 * - ATOM1 CH0/1/2 primaries, DTM for complementary outputs
 * - 20 kHz switching, 1 us dead-time
 * - Single period interrupt on CPU0 toggling LED P03.9 (low-active)
 *
 * Notes:
 * - Watchdog disable belongs ONLY in CpuX main files; none here.
 * - No STM timing in this module. Scheduling (e.g., 500 ms) is done in Cpu0_Main.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define EGTM_PWM_NUM_CHANNELS           (3u)
#define PWM_FREQUENCY                   (20000.0f)   /* Hz */
#define ISR_PRIORITY_ATOM               (20)

#define PHASE_U_DUTY                    (25.0f)      /* % */
#define PHASE_V_DUTY                    (50.0f)      /* % */
#define PHASE_W_DUTY                    (75.0f)      /* % */
#define PHASE_DUTY_STEP                 (10.0f)      /* % */

/* User-requested pin assignments (ATOM1 CH0/1/2 on Port 20) */
#define PHASE_U_HS                      (&IfxEgtm_ATOM1_0N_TOUT64_P20_8_OUT)
#define PHASE_U_LS                      (&IfxEgtm_ATOM1_0_TOUT65_P20_9_OUT)
#define PHASE_V_HS                      (&IfxEgtm_ATOM1_1N_TOUT66_P20_10_OUT)
#define PHASE_V_LS                      (&IfxEgtm_ATOM1_1_TOUT67_P20_11_OUT)
#define PHASE_W_HS                      (&IfxEgtm_ATOM1_0_TOUT68_P20_12_OUT)
#define PHASE_W_LS                      (&IfxEgtm_ATOM1_1_TOUT69_P20_13_OUT)

/* LED P03.9 (low-active) */
#define LED                             (&MODULE_P03), 9

/* =============================
 * Module State
 * ============================= */
typedef struct
{
    IfxEgtm_Pwm             pwm;                                      /* Driver handle */
    IfxEgtm_Pwm_Channel     channels[EGTM_PWM_NUM_CHANNELS];          /* Persistent channel data (filled by init) */
    float32                 dutyCycles[EGTM_PWM_NUM_CHANNELS];         /* Duty in percent [0..100] */
    float32                 phases[EGTM_PWM_NUM_CHANNELS];             /* Phase in percent/ratio (design uses 0.0f) */
    IfxEgtm_Pwm_DeadTime    deadTimes[EGTM_PWM_NUM_CHANNELS];          /* Dead-time per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =============================
 * ISR and Callback (declared before init)
 * ============================= */

/* Period-event callback: required signature; intentionally empty */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Hardware ISR: toggles LED only */
IFX_INTERRUPT(EgtmAtomPeriodIsr, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* =============================
 * Public Functions
 * ============================= */

/**
 * Initialize a 3-channel complementary, center-aligned PWM using the unified eGTM PWM driver.
 * - ATOM submodule, cluster 1
 * - Sync start/update enabled, 20 kHz
 * - Clock: ATOM CLK0 (FXCLK0) and DTM CMU clock 0
 * - Single period interrupt on channel 0
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare local configuration objects */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;
    IfxEgtm_Pwm_OutputConfig      output[EGTM_PWM_NUM_CHANNELS];

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Populate output pin configuration (primary = HS, complementary = LS) */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;           /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;            /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1e-6 s for both edges */
    dtmConfig[0].deadTime.rising = 1e-6f; dtmConfig[0].deadTime.falling = 1e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1e-6f; dtmConfig[1].deadTime.falling = 1e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1e-6f; dtmConfig[2].deadTime.falling = 1e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration: pulse-notify, CPU0, priority 20; periodEvent only */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = &IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;  /* Phase U */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;            /* Interrupt on first channel only */

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;  /* Phase V */
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;  /* Phase W */
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main configuration */
    config.cluster              = IfxEgtm_Cluster_1;
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;
    config.alignment            = IfxEgtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.frequency            = PWM_FREQUENCY;
    config.clockSource.atom     = (uint32)IfxEgtm_Cmu_Clk_0;     /* ATOM clock: CLK0 (FXCLK0 domain) */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.numChannels          = (uint8)EGTM_PWM_NUM_CHANNELS;
    config.channels             = channelConfig;

    /* 8) Enable-guard: enable eGTM and configure CMU clocks */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent handle and channels storage */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store initial duty cycles, phases, and dead-times into persistent state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as output (do not drive it) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duty cycles synchronously by a fixed step and apply immediately.
 * - Wrap to 0 if (current + step) >= 100, then always add the step.
 * - Apply new duties atomically at next update point via Immediate API.
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
