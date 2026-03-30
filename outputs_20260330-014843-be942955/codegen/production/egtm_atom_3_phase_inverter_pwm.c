/******************************************************************************
 * @file    egtm_atom_3_phase_inverter_pwm.c
 * @brief   TC4xx eGTM/ATOM 3-phase complementary center-aligned PWM driver
 * @version 1.0
 * @note    Migration: TC3xx -> TC4xx using IfxEgtm_Pwm (unified high-level driver)
 *          - Submodule: ATOM (Cluster 0)
 *          - Frequency: 20 kHz, Center-aligned
 *          - Complementary outputs with dead-time = 1 us (rising/falling)
 *          - Initial duties U/V/W = 25/50/75 %
 *          - Atomic duty update step = +10 %
 *          - Period-event ISR priority = 20, provider = CPU0, VMID 0
 *          - LED toggle pin: P03.9
 *
 * IMPORTANT ARCHITECTURE NOTES:
 * - Do NOT disable watchdogs in this driver (Cpu0_Main.c only as per project standard)
 * - Clocks: Enable eGTM and configure GCLK and CMU clocks inside the enable guard
 * - Pins: OutputConfig array used for pin routing; no separate pin-config function
 * - Interrupts: Configured via InterruptConfig; ISR only toggles LED
 ******************************************************************************/

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/*============================================================================*/
/* Configuration macros (constants from requirements)                         */
/*============================================================================*/
#define NUM_OF_CHANNELS               (3u)
#define PWM_FREQUENCY_HZ              (20000.0f)
#define ISR_PRIORITY_ATOM             (20)
#define PHASE_U_DUTY                  (25.0f)
#define PHASE_V_DUTY                  (50.0f)
#define PHASE_W_DUTY                  (75.0f)
#define PHASE_DUTY_STEP               (10.0f)

/* LED (P03.9) compound macro: usable as (port, pin) */
#define LED                           &MODULE_P03, 9u

/*============================================================================*/
/* Pin mapping (use validated symbols when available; otherwise NULL_PTR)      */
/*============================================================================*/
/*
 * Validated ATOM pins list is limited in this context. Where a validated
 * IfxEgtm_*_TOUT*_Pxx_y_OUT symbol is not provided below, the macro is set
 * to NULL_PTR. Replace NULL_PTR with the correct symbol from IfxEgtm_PinMap.h
 * during integration for the chosen device/package.
 */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT64_P20_8_OUT */
#define PHASE_U_LS   (NULL_PTR) /* Prefer &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT if available */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT55_P21_4_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT67_P20_11_OUT */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT68_P20_12_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT69_P20_13_OUT */

/*============================================================================*/
/* Module state                                                               */
/*============================================================================*/
typedef struct
{
    IfxEgtm_Pwm             pwm;                                 /* Driver handle */
    IfxEgtm_Pwm_Channel     channels[NUM_OF_CHANNELS];           /* Persistent channels array */
    float32                 dutyCycles[NUM_OF_CHANNELS];          /* Duty in percent */
    float32                 phases[NUM_OF_CHANNELS];              /* Phase in [0..1) */
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];           /* Dead-time per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/*============================================================================*/
/* Forward declarations (private)                                              */
/*============================================================================*/
/**
 * Period-event ISR for eGTM/ATOM
 * Body must only toggle LED (minimal ISR latency as per guidelines)
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/**
 * Period event callback (assigned via InterruptConfig)
 * Must be empty per design — no actions here.
 */
void IfxEgtm_periodEventFunction(void *data);

/*============================================================================*/
/* Private functions                                                          */
/*============================================================================*/
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty */
}

/*============================================================================*/
/* Public API                                                                  */
/*============================================================================*/
/**
 * Initialize a 3-channel complementary, center-aligned inverter PWM using
 * the unified eGTM high-level PWM driver (ATOM0, Cluster 0).
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as locals */
    IfxEgtm_Pwm_Config           config;                                 /* Main PWM config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];         /* Per-channel cfg */
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];             /* Per-channel DTM */
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;                        /* Period ISR cfg */
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];                /* Per-channel pins */

    /* 2) Initialize the main config with driver defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Fill OutputConfig for 3 logical channels (HS active-high, LS active-low) */
    /* U phase */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* High-side active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* Low-side  active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Set DTM dead-time per channel: rising=1 us, falling=1 us */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Configure interrupt parameters for period event */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Configure ChannelConfig for three logical PWM channels 0..2 */
    /* CH0 → U phase (base channel gets interrupt) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;

    /* CH1 → V phase */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* CH2 → W phase */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Complete the main config fields */
    config.cluster            = IfxEgtm_Cluster_0;                    /* ATOM0 Cluster 0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;           /* ATOM */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;         /* Center-aligned */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;               /* 3 channels */
    config.channels           = channelConfig;                        /* Attach channel cfg */
    config.frequency          = PWM_FREQUENCY_HZ;                     /* 20 kHz */
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                    /* ATOM source: CMU.CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;    /* DTM from CMU.CLK0 */
    config.syncUpdateEnabled  = TRUE;                                  /* Synchronized updates */
    config.syncStart          = TRUE;                                  /* Synchronized start */

    /* 8) eGTM enable guard and CMU clock configuration */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        (void)IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM); /* Runtime read as per pattern */
        /* Set GCLK divider to unity (no division) */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
        /* Set ECLK0 divider to unity for DTM timing (no division) */
        IfxEgtm_Cmu_setEclkDivider(&MODULE_EGTM, IfxEgtm_Cmu_Eclk_0, 1u, 1u);
        /* Enable FXCLK and CMU CLK0 clocks */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent channels array from module state */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store persistent state copies of initial duties, phases, and dead-times */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure the LED GPIO as output (no explicit set required) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Atomic multi-channel duty update: step +10% and wrap at 100%.
 * Applies new duties with IfxEgtm_Pwm_updateChannelsDutyImmediate.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap rule: check -> maybe reset -> unconditional add (for each channel) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply atomically to all channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
