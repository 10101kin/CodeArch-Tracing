/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: EGTM ATOM 3-Phase Complementary, Center-Aligned PWM (TC4xx)
 * - Unified high-level driver: IfxEgtm_Pwm
 * - Sub-module: ATOM (Cluster 0)
 * - Complementary outputs with dead-time insertion
 * - Period ISR toggles LED, callback empty
 *
 * Notes:
 * - Followed iLLD initialization patterns and migration rules (TC3xx -> TC4xx, GTM -> eGTM)
 * - No watchdog operations here (must be in CpuX_Main.c only)
 * - No STM/timing logic here (scheduling belongs to CpuX_Main.c)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD headers (selected) */
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =============================
 * Macros (configuration values)
 * ============================= */
#define EGTM_PWM_NUM_CHANNELS   (3u)
#define PWM_FREQUENCY           (20000.0f)      /* Hz */
#define ISR_PRIORITY_ATOM       (20)

/* Duty cycles (percent) */
#define PHASE_U_DUTY            (25.0f)
#define PHASE_V_DUTY            (50.0f)
#define PHASE_W_DUTY            (75.0f)
#define PHASE_DUTY_STEP         (10.0f)

/* LED (compound macro expands to (port, pin)) */
#define LED                     &MODULE_P03, 9

/* =============================
 * Pin routing macros (validated list only)
 * =============================
 * Use the validated pin symbols list for TC4xx eGTM. If a requested pin is not
 * present in the validated list, leave it as NULL_PTR and replace during
 * integration with the correct PinMap symbol from your BSP/device pinmap.
 *
 * User-requested pins:
 *  U: HS=P20.8 (TOUT64),  LS=P20.9  (TOUT65)
 *  V: HS=P21.4 (TOUT55),  LS=P20.11 (TOUT67)
 *  W: HS=P20.12(TOUT68),  LS=P20.13 (TOUT69)
 */
#define PHASE_U_HS   (NULL_PTR)                               /* Replace with &IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT when available */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)     /* Validated list */
#define PHASE_V_HS   (NULL_PTR)                               /* Replace with &IfxEgtm_ATOM0_x_TOUT55_P21_4_OUT when available */
#define PHASE_V_LS   (NULL_PTR)                               /* Replace with &IfxEgtm_ATOM0_xN_TOUT67_P20_11_OUT when available */
#define PHASE_W_HS   (NULL_PTR)                               /* Replace with &IfxEgtm_ATOM0_x_TOUT68_P20_12_OUT when available */
#define PHASE_W_LS   (NULL_PTR)                               /* Replace with &IfxEgtm_ATOM0_xN_TOUT69_P20_13_OUT when available */

/* =============================
 * Module state
 * ============================= */

typedef struct
{
    IfxEgtm_Pwm               pwm;                                   /* Driver handle (persistent) */
    IfxEgtm_Pwm_Channel       channels[EGTM_PWM_NUM_CHANNELS];       /* Persistent channels array */
    float32                   dutyCycles[EGTM_PWM_NUM_CHANNELS];     /* Percent 0..100 */
    float32                   phases[EGTM_PWM_NUM_CHANNELS];         /* Phase offsets (s) or 0 */
    IfxEgtm_Pwm_DeadTime      deadTimes[EGTM_PWM_NUM_CHANNELS];      /* Per-channel dead-time */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =============================
 * ISR and callback (declared before init)
 * ============================= */

/**
 * Period ISR (hardware) for eGTM ATOM PWM.
 * Keep minimal: toggle LED and return.
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/**
 * Period event callback used by the unified driver.
 * Must exist and be empty.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Public API
 * ============================= */

/**
 * Initialize 3-phase complementary, center-aligned PWM on eGTM ATOM (Cluster 0).
 * - Uses IfxEgtm_Pwm unified API
 * - 3 logical channels (each drives HS/LS with dead-time)
 * - Center-aligned, 20 kHz, syncStart + syncUpdate enabled
 * - FXCLK0/CLK0 clock path enabled in CMU
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration structures (locals) */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[EGTM_PWM_NUM_CHANNELS];

    /* 2) Populate main config defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration: complementary pairs, polarity HS=high, LS=low */
    /* Phase U */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;      /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;       /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time: 1 us rising, 1 us falling for each channel */
    dtmConfig[0].deadTime.rising = 1e-6f; dtmConfig[0].deadTime.falling = 1e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1e-6f; dtmConfig[1].deadTime.falling = 1e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1e-6f; dtmConfig[2].deadTime.falling = 1e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;   /* pulse notify */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;               /* CPU0 */
    interruptConfig.priority    = ISR_PRIORITY_ATOM;             /* priority 20 */
    interruptConfig.vmId        = IfxSrc_VmId_0;                 /* VM 0 */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;   /* callback */
    interruptConfig.dutyEvent   = NULL_PTR;                      /* none */

    /* 6) Logical channel configuration: Ch0..Ch2, phase=0, duties 25/50/75 */
    /* Channel 0 -> Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* only CH0 */

    /* Channel 1 -> Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2 -> Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main config: ATOM, cluster 0, center-aligned, syncStart/Update, 20 kHz, FXCLK0/CLK0 */
    config.cluster             = IfxEgtm_Cluster_0;
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;
    config.alignment           = IfxEgtm_Pwm_Alignment_center;
    config.numChannels         = (uint8)EGTM_PWM_NUM_CHANNELS;
    config.channels            = channelConfig;
    config.frequency           = PWM_FREQUENCY;
    config.clockSource.atom    = IfxEgtm_Cmu_Clk_0;                /* ATOM uses CLK0 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_systemClock; /* DTM clock source */
    config.syncUpdateEnabled   = TRUE;
    config.syncStart           = TRUE;

    /* 8) Enable-guard and CMU setup (inside guard only) */
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

    /* 9) Initialize PWM with persistent handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist initial state (duties, phases, dead-times) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (no state change here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Advance each channel duty by a fixed step and apply immediately.
 * Sequence per channel: if (duty+step)>=100 -> duty=0; then duty += step.
 * Applies all three duties atomically via immediate multi-channel update.
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
