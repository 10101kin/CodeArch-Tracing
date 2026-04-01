/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for TC4xx EGTM ATOM0 (Cluster 0) 3-phase complementary, center-aligned PWM
 * using the unified IfxEgtm_Pwm high-level driver.
 *
 * Follows iLLD initialization patterns and migration guidelines.
 *
 * Requirements implemented:
 * - 3 channels (U,V,W), complementary outputs with 1 us dead-time (rising/falling)
 * - Center-aligned PWM at 20 kHz
 * - Synchronous start and synchronous update enabled
 * - Period-event ISR on base channel (channel 0) toggling LED P03.9
 * - eGTM enable and CMU clock setup inside enable-guard
 *
 * Watchdog note: No watchdog disable in this driver (must be in CpuX_Main.c only).
 */

/* Own public header */
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Mandatory iLLD types and drivers */
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================================================================================= */
/* Configuration macros (migration values and signal/timing requirements)                    */
/* ========================================================================================= */
#define NUM_OF_CHANNELS           (3u)
#define PWM_FREQUENCY             (20000.0f)
#define ISR_PRIORITY_ATOM         (20)
#define PHASE_U_DUTY              (25.0f)
#define PHASE_V_DUTY              (50.0f)
#define PHASE_W_DUTY              (75.0f)
#define PHASE_DUTY_STEP           (10.0f)

/* LED: compound macro used directly in IfxPort_* calls */
#define LED                       &MODULE_P03, 9

/* User-requested pin assignments (ATOM0, Cluster 0) */
#define PHASE_U_HS                (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ========================================================================================= */
/* Module state                                                                              */
/* ========================================================================================= */
typedef struct
{
    IfxEgtm_Pwm            pwm;                                   /* Driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];             /* Persistent channels array */
    float32                dutyCycles[NUM_OF_CHANNELS];            /* Duty in percent */
    float32                phases[NUM_OF_CHANNELS];                /* Phase in percent (if used) */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];             /* Dead-time values (s) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================================================================================= */
/* ISR and callback (must be defined before init)                                            */
/* ========================================================================================= */
/**
 * Period-event ISR for EGTM ATOM group.
 * Minimal ISR: toggles the LED and returns.
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/**
 * Period event callback used by the IfxEgtm_Pwm interrupt configuration.
 * Intentionally empty; ISR performs the LED toggle.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================================================================================= */
/* Public API                                                                                */
/* ========================================================================================= */
/**
 * Initialize a 3-channel complementary, center-aligned PWM on EGTM ATOM0 Cluster 0.
 *
 * Algorithm (strict order):
 *  1) Declare local config structures
 *  2) IfxEgtm_Pwm_initConfig defaults
 *  3) output[] pin/polarity/mode/padDriver
 *  4) dtmConfig[] dead-time 1e-6 s rising/falling
 *  5) interruptConfig for period event on base channel (ch 0)
 *  6) channelConfig[0..2] with logical indices, phases, duties, dtm/output, interrupt only on [0]
 *  7) main config: cluster/submodule, alignment=center, syncStart/Update, freq=20kHz,
 *     clockSource.atom=Clk_0, dtmClockSource=cmuClock0, numChannels=3, channels=channelConfig
 *  8) Enable-guard and CMU setup inside guard
 *  9) IfxEgtm_Pwm_init with persistent handle + channels array
 * 10) Copy initial duties and dead-times into persistent state arrays
 * 11) Configure LED GPIO as push-pull output (no level set)
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration for complementary pairs (U, V, W) */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;   /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;    /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us rising/falling for each channel */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff = NULL_PTR;

    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff = NULL_PTR;

    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration: pulse-notify on period event, CPU0, prio 20 */
    interruptConfig.mode         = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider  = IfxSrc_Tos_cpu0;
    interruptConfig.priority     = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId         = IfxSrc_VmId_0;
    interruptConfig.periodEvent  = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent    = NULL_PTR;

    /* 6) Per-channel configuration (logical channel indices 0..2) */
    channelConfig[0].timerCh     = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase       = 0.0f;
    channelConfig[0].duty        = PHASE_U_DUTY;
    channelConfig[0].dtm         = &dtmConfig[0];
    channelConfig[0].output      = &output[0];
    channelConfig[0].mscOut      = NULL_PTR;
    channelConfig[0].interrupt   = &interruptConfig;  /* Base channel: ISR attached here */

    channelConfig[1].timerCh     = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase       = 0.0f;
    channelConfig[1].duty        = PHASE_V_DUTY;
    channelConfig[1].dtm         = &dtmConfig[1];
    channelConfig[1].output      = &output[1];
    channelConfig[1].mscOut      = NULL_PTR;
    channelConfig[1].interrupt   = NULL_PTR;

    channelConfig[2].timerCh     = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase       = 0.0f;
    channelConfig[2].duty        = PHASE_W_DUTY;
    channelConfig[2].dtm         = &dtmConfig[2];
    channelConfig[2].output      = &output[2];
    channelConfig[2].mscOut      = NULL_PTR;
    channelConfig[2].interrupt   = NULL_PTR;

    /* 7) Complete main configuration */
    config.cluster               = IfxEgtm_Cluster_0;
    config.subModule             = IfxEgtm_Pwm_SubModule_atom;
    config.alignment             = IfxEgtm_Pwm_Alignment_center;
    config.syncStart             = TRUE;
    config.syncUpdateEnabled     = TRUE;
    config.frequency             = PWM_FREQUENCY;
    config.clockSource.atom      = (uint32)IfxEgtm_Cmu_Clk_0; /* Use CMU CLK0 for ATOM */
    config.dtmClockSource        = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.numChannels           = (uint8)NUM_OF_CHANNELS;
    config.channels              = &channelConfig[0];

    /* 8) eGTM enable guard + CMU setup (inside the guard, use exact pattern) */
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

    /* 10) Store initial duties, phases, and dead-times into persistent state */
    g_egtmAtom3phInv.dutyCycles[0]  = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1]  = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2]  = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0]      = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]      = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]      = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0]   = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]   = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]   = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (no level change here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update duty cycles of all three channels by a fixed step with wrap-around at 100% and
 * apply immediately and synchronously using the unified driver bulk API.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap rule: check+reset then unconditional add, per channel, no loop */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately to all channels synchronously */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
