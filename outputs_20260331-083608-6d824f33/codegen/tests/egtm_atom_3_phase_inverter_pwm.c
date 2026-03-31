/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM ATOM 3-Phase complementary PWM on TC4xx.
 * - 3 channels (U, V, W), complementary outputs via DTM
 * - Center-aligned, syncStart + syncUpdate enabled
 * - 20 kHz switching, 1 us dead-time
 * - Period interrupt on CPU0 (prio 20) toggles LED P03.9 (low-active)
 *
 * Notes:
 * - No watchdog handling here (must be done only in CpuX_Main.c)
 * - No STM timing here; higher-level scheduling occurs in Cpu0_Main.c
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================================
 * Numeric configuration macros (user-confirmed)
 * ============================================= */
#define EGTM_PWM_NUM_CHANNELS   (3u)
#define PWM_FREQUENCY           (20000.0f)   /* Hz */
#define ISR_PRIORITY_ATOM       (20)
#define PHASE_U_DUTY            (25.0f)      /* percent */
#define PHASE_V_DUTY            (50.0f)      /* percent */
#define PHASE_W_DUTY            (75.0f)      /* percent */
#define PHASE_DUTY_STEP         (10.0f)      /* percent */

/* =====================
 * LED macro (P03.9 low)
 * ===================== */
#define LED &MODULE_P03, 9

/* =============================================
 * Pin routing macros (use validated symbols only)
 * If validated symbols for the requested pins are not present in this
 * template context, provide NULL_PTR placeholders and document the
 * intended symbols for integration.
 * ============================================= */
#define PHASE_U_HS  (NULL_PTR) /* Expected: &IfxEgtm_ATOM1_0N_TOUT64_P20_8_OUT */
#define PHASE_U_LS  (NULL_PTR) /* Expected: &IfxEgtm_ATOM1_0_TOUT65_P20_9_OUT  */
#define PHASE_V_HS  (NULL_PTR) /* Expected: &IfxEgtm_ATOM1_1N_TOUT66_P20_10_OUT */
#define PHASE_V_LS  (NULL_PTR) /* Expected: &IfxEgtm_ATOM1_1_TOUT67_P20_11_OUT  */
#define PHASE_W_HS  (NULL_PTR) /* Expected: &IfxEgtm_ATOM1_2N_TOUT68_P20_12_OUT */
#define PHASE_W_LS  (NULL_PTR) /* Expected: &IfxEgtm_ATOM1_2_TOUT69_P20_13_OUT  */

/* =============================
 * Module persistent state (RAM)
 * ============================= */
typedef struct
{
    IfxEgtm_Pwm              pwm;                                       /* Driver handle */
    IfxEgtm_Pwm_Channel      channels[EGTM_PWM_NUM_CHANNELS];           /* Channel runtime data (filled by init) */
    float32                  dutyCycles[EGTM_PWM_NUM_CHANNELS];         /* Duty in percent [0..100] */
    float32                  phases[EGTM_PWM_NUM_CHANNELS];             /* Phase in period fraction [0..1], here all 0 */
    IfxEgtm_Pwm_DeadTime     deadTimes[EGTM_PWM_NUM_CHANNELS];          /* Dead-time settings (seconds) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =====================================================
 * ISR and period-callback (declared BEFORE init function)
 * ===================================================== */

/* Route by driver via InterruptConfig: CPU0, priority ISR_PRIORITY_ATOM. */
IFX_INTERRUPT(EgtmAtomPeriodIsr, 0, ISR_PRIORITY_ATOM);

/**
 * Hardware ISR for the eGTM PWM period event: toggles LED and returns.
 */
void EgtmAtomPeriodIsr(void)
{
    IfxPort_togglePin(LED);
}

/**
 * Empty period-event callback; assigned to InterruptConfig.periodEvent.
 * Required exact signature by unified PWM driver.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ===========================
 * Driver public API functions
 * =========================== */

/**
 * Initialize a 3-channel complementary, center-aligned PWM using eGTM ATOM.
 *
 * Algorithm (per SW Detailed Design):
 * 1) Declare local configuration objects
 * 2) Load defaults with IfxEgtm_Pwm_initConfig
 * 3) Populate OutputConfig for U/V/W with requested pins, polarity, pad driver
 * 4) Configure 1 us dead-time for rising/falling edges on all channels
 * 5) Configure InterruptConfig (pulse-notify, CPU0, priority 20, periodEvent set)
 * 6) Fill ChannelConfig[0..2] for logical channels Ch_0..Ch_2 with duties, outputs, DTM
 * 7) Complete main config: Cluster1, ATOM, center, syncStart+syncUpdate, 20 kHz,
 *    ATOM clock FXCLK0, DTM clock CMU0, attach channels and numChannels=3
 * 8) Enable guard for eGTM and CMU clock setup
 * 9) Call IfxEgtm_Pwm_init with persistent handle and channels array
 * 10) Store initial duties and dead-times into module state
 * 11) Configure LED as output (do not drive it here)
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxEgtm_Pwm_Config            config;                                /* Main PWM config */
    IfxEgtm_Pwm_ChannelConfig     channelConfig[EGTM_PWM_NUM_CHANNELS];  /* Per-channel configuration (input to init) */
    IfxEgtm_Pwm_DtmConfig         dtmConfig[EGTM_PWM_NUM_CHANNELS];      /* Per-channel DTM config */
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;                       /* Single interrupt config (period) */
    IfxEgtm_Pwm_OutputConfig      output[EGTM_PWM_NUM_CHANNELS];         /* Per-channel output pins */

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration: primary=complementary pair, polarity high/low, push-pull, auto speed pad */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;              /* HS active-high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;               /* LS active-low  */
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

    /* 4) Dead-time: 1e-6 seconds on rising and falling for all channels */
    dtmConfig[0].deadTime.rising = 1e-6f;  dtmConfig[0].deadTime.falling = 1e-6f;  dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1e-6f;  dtmConfig[1].deadTime.falling = 1e-6f;  dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1e-6f;  dtmConfig[2].deadTime.falling = 1e-6f;  dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration: pulse notify, CPU0, priority, periodEvent callback assigned */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = &IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices Ch_0..Ch_2 with initial duties and DTM/output links */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;   /* Only CH0 has interrupt */

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main configuration */
    config.cluster             = IfxEgtm_Cluster_1;
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;
    config.alignment           = IfxEgtm_Pwm_Alignment_center;
    config.syncStart           = TRUE;
    config.syncUpdateEnabled   = TRUE;
    config.frequency           = PWM_FREQUENCY;
    config.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;              /* ATOM uses CMU CLK0 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;      /* DTM from CMU clock 0 */
    config.channels            = &channelConfig[0];
    config.numChannels         = (uint8)EGTM_PWM_NUM_CHANNELS;

    /* 8) Enable-guard: enable eGTM and configure CMU clocks */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver with persistent handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Persist initial duties, phases, and dead-times */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    g_egtmAtom3phInv.deadTimes[0].rising  = dtmConfig[0].deadTime.rising;
    g_egtmAtom3phInv.deadTimes[0].falling = dtmConfig[0].deadTime.falling;
    g_egtmAtom3phInv.deadTimes[1].rising  = dtmConfig[1].deadTime.rising;
    g_egtmAtom3phInv.deadTimes[1].falling = dtmConfig[1].deadTime.falling;
    g_egtmAtom3phInv.deadTimes[2].rising  = dtmConfig[2].deadTime.rising;
    g_egtmAtom3phInv.deadTimes[2].falling = dtmConfig[2].deadTime.falling;

    /* 11) Configure LED GPIO as output (do not drive it here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duty cycles synchronously by a fixed step and apply immediately.
 * No timing here; call from application loop with desired period.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-then-increment rule (per channel, no loop) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply atomically at next update point */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
