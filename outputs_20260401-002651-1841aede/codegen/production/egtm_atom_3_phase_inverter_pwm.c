/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: EGTM ATOM 3-Phase Inverter PWM for TC4xx
 * - Migrated from GTM TOM (IfxGtm_Pwm) to EGTM ATOM (IfxEgtm_Pwm)
 * - Six complementary outputs (3 channels) center-aligned @ 20 kHz
 * - 1 us rising/falling dead-time
 * - Pins: P20.8..P20.13 (TOUT64..TOUT69)
 * - Period ISR on CPU0 (prio=20) toggles LED P03.9
 *
 * Notes:
 * - Watchdogs must be handled only in CpuX_Main.c (not here)
 * - No STM timing logic in this driver
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "Egtm/Pwm/IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ===================== Configuration Macros ===================== */
#define NUM_OF_CHANNELS           (3u)
#define PWM_FREQUENCY             (20000.0f)
#define ISR_PRIORITY_ATOM         (20)

/* User-requested pin assignments (KIT_A3G_TC4D7_LITE, TC4D7) */
#define PHASE_U_HS                (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* Initial duty cycles (%) */
#define PHASE_U_DUTY              (25.0f)
#define PHASE_V_DUTY              (50.0f)
#define PHASE_W_DUTY              (75.0f)

/* Duty step increment (%) */
#define PHASE_DUTY_STEP           (10.0f)

/* LED macro: expands to (port, pin) */
#define LED                       &MODULE_P03, 9

/* ===================== Module State ===================== */
typedef struct
{
    IfxEgtm_Pwm             pwm;                               /* Unified PWM driver handle */
    IfxEgtm_Pwm_Channel     channels[NUM_OF_CHANNELS];         /* Persistent channel data (used by driver) */
    float32                 dutyCycles[NUM_OF_CHANNELS];       /* Duty percent per phase */
    float32                 phases[NUM_OF_CHANNELS];           /* Phase offsets (deg or relative 0..1) */
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];        /* Dead-time per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;              /* Persistent module state */

/* ===================== ISR and Callback (declared before init) ===================== */
/**
 * Empty period-event callback used by the unified PWM driver. Body intentionally empty.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Forward-declare ISR with vector attributes */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/**
 * Period ISR: minimal processing (toggle LED only).
 */
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ===================== Public API ===================== */
/**
 * Initialize a 3-channel center-aligned complementary PWM on the EGTM ATOM submodule with
 * synchronized start/update and period interrupt.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all config structs as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load defaults into main config */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Populate complementary PWM outputs (U, V, W) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;                 /* HS active-high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;                  /* LS active-low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Configure DTM (1 us rising/falling) */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration for PWM-period notification */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations: logical indices 0,1,2 (U,V,W) */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;      /* base channel gets interrupt */

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;              /* no separate interrupt */

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;              /* no separate interrupt */

    /* 7) Main config fields */
    config.cluster             = IfxEgtm_Cluster_0;
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;
    config.alignment           = IfxEgtm_Pwm_Alignment_center;
    config.numChannels         = (uint8)NUM_OF_CHANNELS;
    config.channels            = &channelConfig[0];
    config.frequency           = PWM_FREQUENCY;
    config.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;          /* ATOM uses CMU CLK0 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;  
    config.syncUpdateEnabled   = TRUE;                                /* shadow to active at period */
    config.syncStart           = TRUE;                                /* start all together */

    /* 8) EGTM enable guard + CMU clock configuration (keep inside guard) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initial duties, phases, and dead-times into module state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]     = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as output (push-pull); do not force a level here */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update all three PWM channels by stepping each duty cycle by a fixed increment and
 * wrapping at 100%. Apply immediately at the driver's shadow-to-active transfer point.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap then step (no loop, three explicit blocks as required) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate multi-channel duty update */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
