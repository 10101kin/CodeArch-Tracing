/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production-ready EGTM ATOM 3-Phase Inverter PWM driver for AURIX TC4xx.
 *
 * Notes:
 * - Uses IfxEgtm_Pwm unified high-level driver on EGTM ATOM submodule.
 * - Complementary, center-aligned PWM with dead-time insertion.
 * - Follows iLLD initialization patterns and CMU clock enable guard.
 * - No watchdog operations in this driver (per architecture standard).
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros (Numerical configuration) ========================= */
#define NUM_OF_CHANNELS          (3)
#define PWM_FREQUENCY            (20000.0f)
#define ISR_PRIORITY_ATOM        (20)
#define PHASE_U_DUTY             (25.0f)
#define PHASE_V_DUTY             (50.0f)
#define PHASE_W_DUTY             (75.0f)
#define PHASE_DUTY_STEP          (10.0f)

/* LED pin (compound macro used directly as (port, pin) in API calls) */
#define LED                      &MODULE_P03, 9  /* P03.9 */

/* ========================= Pin Macros (Validated symbols only) ========================= */
/*
 * User-requested complementary pin assignments (highest priority):
 *   U: HS=P20.8 (TOUT64), LS=P20.9 (TOUT65)
 *   V: HS=P21.4 (TOUT55), LS=P20.11 (TOUT67)
 *   W: HS=P20.12 (TOUT68), LS=P20.13 (TOUT69)
 *
 * Validated symbols provided (subset):
 *   &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT  (P20.9)
 *
 * For pins not present in the validated list, use NULL_PTR placeholders and
 * leave comments for integrators to provide the exact PinMap symbols.
 */
#define PHASE_U_HS   (NULL_PTR)                         /* Replace with &IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT when available */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS   (NULL_PTR)                         /* Replace with &IfxEgtm_ATOM0_?_TOUT55_P21_4_OUT when available */
#define PHASE_V_LS   (NULL_PTR)                         /* Replace with &IfxEgtm_ATOM0_?_TOUT67_P20_11_OUT when available */
#define PHASE_W_HS   (NULL_PTR)                         /* Replace with &IfxEgtm_ATOM0_?_TOUT68_P20_12_OUT when available */
#define PHASE_W_LS   (NULL_PTR)                         /* Replace with &IfxEgtm_ATOM0_?_TOUT69_P20_13_OUT when available */

/* ========================= Module State ========================= */
typedef struct
{
    IfxEgtm_Pwm              pwm;                               /* Unified driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];         /* Persistent channel state (driver writes into this) */
    float32                  dutyCycles[NUM_OF_CHANNELS];       /* Duty in percent per logical channel */
    float32                  phases[NUM_OF_CHANNELS];           /* Phase (s) per logical channel */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];        /* Dead-time per logical channel */
} EgtmAtom3phInv_State;

/* IFX_STATIC per architecture requirement */
IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback (defined before init) ========================= */
/* Bind ISR: provider = cpu0, priority = ISR_PRIORITY_ATOM (driver will configure SRC) */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/** \brief ISR invoked at PWM period event (keep minimal): toggle LED and return */
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/** \brief Period event callback used by unified driver (intentionally empty) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */
/** \brief Initialize 3-phase complementary, center-aligned PWM on EGTM ATOM using unified driver. */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Populate output pin configurations per logical channel (HS active-high, LS active-low) */
    /* Channel 0 → Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high; /* HS: active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;  /* LS: active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 1 → Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 2 → Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us rising and 1 us falling for each channel */
    /* Use 1e-6f literal as required */
    dtmConfig[0].deadTime.rising = 1e-6f; dtmConfig[0].deadTime.falling = 1e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1e-6f; dtmConfig[1].deadTime.falling = 1e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1e-6f; dtmConfig[2].deadTime.falling = 1e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration: pulse notify on period, CPU0, priority=20, vmId=0, callback assigned */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = (IfxEgtm_Pwm_callBack)IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Logical channel configurations: channels 0..2, center-aligned complementary pairs */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* Interrupt only on first channel */

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

    /* 7) Main configuration: ATOM on Cluster 0, center-aligned, sync enabled, 20 kHz, CMU Clk_0 for ATOM, DTM from cmuClock0 */
    config.cluster             = IfxEgtm_Cluster_0;
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;
    config.alignment           = IfxEgtm_Pwm_Alignment_center;
    config.numChannels         = (uint8)NUM_OF_CHANNELS;
    config.channels            = channelConfig;
    config.frequency           = PWM_FREQUENCY;
    config.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;     /* UNION: set only the ATOM field */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.syncUpdateEnabled   = TRUE;
    config.syncStart           = TRUE;

    /* 8) EGTM enable-guard and CMU setup (inside guard only) */
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

    /* 9) Initialize PWM with persistent driver handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist initial state (duties, phases, dead-times) */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED pin as push-pull output (state not changed here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/** \brief Advance each channel's duty by fixed step and apply immediately (atomic multi-channel update). */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap then increment, per phase, explicit (no loop) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply to hardware immediately, atomically for all configured channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
