/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM ATOM 3-Phase Inverter PWM (TC4xx)
 *
 * - 3 complementary, center-aligned PWM pairs on ATOM0 CH0..CH2 @ 20 kHz
 * - 1 us rising/falling dead-time via DTM
 * - ATOM0 CH3 routed as ADC trigger via IfxEgtm_Trigger_trigToAdc (AdcTriggerSignal_0)
 * - Single PWM init (unified driver), persistent state for runtime updates
 * - LED (P03.9) configured as push-pull output; ISR toggles LED
 *
 * Notes:
 * - Watchdog disable is NOT performed here (must be in CpuX_Main.c only per AURIX standard)
 * - No STM/timing ops in this module; scheduling done by application code
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =====================
 * Configuration macros
 * ===================== */
#define NUM_OF_CHANNELS        (3)
#define PWM_FREQUENCY          (20000.0f)     /* Hz */
#define PHASE_U_DUTY           (25.0f)        /* % */
#define PHASE_V_DUTY           (50.0f)        /* % */
#define PHASE_W_DUTY           (75.0f)        /* % */
#define PHASE_DUTY_STEP        (0.01f)        /* % step (not used here; kept for app reference) */
#define ISR_PRIORITY_ATOM      (100)          /* PWM period ISR priority */

/* LED macro: compound form (port, pin) */
#define LED                    &MODULE_P03, 9

/*
 * Complementary output pin mappings.
 * Use only validated symbols; if unavailable in this template, leave as NULL_PTR for integration.
 * The application integrator must replace NULL_PTR with the appropriate &IfxEgtm_ATOMx_y[_N]_TOUTz_Pxx_y_OUT symbols.
 */
#define PHASE_U_HS             (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_0_TOUT<P20_8> symbol in target PinMap */
#define PHASE_U_LS             (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_0N_TOUT<P20_9> symbol in target PinMap */
#define PHASE_V_HS             (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT<P20_10> symbol in target PinMap */
#define PHASE_V_LS             (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT<P20_11> symbol in target PinMap */
#define PHASE_W_HS             (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT<P20_12> symbol in target PinMap */
#define PHASE_W_LS             (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT<P20_13> symbol in target PinMap */

/* =====================
 * Module state
 * ===================== */

typedef struct
{
    IfxEgtm_Pwm             pwm;                              /* Driver handle */
    IfxEgtm_Pwm_Channel     channels[NUM_OF_CHANNELS];        /* Persistent channel data */
    float32                 dutyCycles[NUM_OF_CHANNELS];       /* Duty in percent */
    float32                 phases[NUM_OF_CHANNELS];           /* Phase offsets in seconds (if used) */
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];        /* Dead-time per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =====================
 * ISR and Callbacks
 * ===================== */

/* ISR: PWM period event — minimal body per best practices */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period callback required by IfxEgtm_Pwm_InterruptConfig — EMPTY body */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =====================
 * Public API
 * ===================== */

/**
 * Initialize the EGTM module clocks (guarded), configure 3-phase complementary PWM on ATOM0 CH0..CH2
 * with center alignment and 1 us dead-time, set up interrupt on base channel, and route ATOM0 CH3 to ADC trigger.
 * Pins are attached via OutputConfig; when NULL_PTR, routing is skipped and must be completed during integration.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration — complementary pairs with required polarities */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* HS active-high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* LS active-low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration — both edges = 1 us (in seconds) */
    dtmConfig[0].deadTime.rising  = 1e-6f;  dtmConfig[0].deadTime.falling  = 1e-6f;  dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising  = 1e-6f;  dtmConfig[1].deadTime.falling  = 1e-6f;  dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising  = 1e-6f;  dtmConfig[2].deadTime.falling  = 1e-6f;  dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration — single period-event callback on base channel */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration — three logical channels (U, V, W) */
    /* Base channel: U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;    /* only first channel has interrupt */

    /* Synchronous channel: V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Synchronous channel: W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;              /* ATOM submodule */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;            /* center-aligned */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;               /* CMU Clock 0 for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM clock from CMU Clock 0 */
    config.syncUpdateEnabled  = TRUE;                                    /* shadow-to-active at period end */
    config.syncStart          = TRUE;                                    /* outputs start after init */

    /* 8) EGTM enable guard + CMU configuration (MANDATORY) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);                                  /* GCLK 1:1 */
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);                /* CMU CLK0 1:1 */
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Unified PWM init — pass persistent handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store initial state (duties, phases, dead-times) for later updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]     = channelConfig[2].phase;
    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 11) Route ATOM0 CH3 to ADC trigger bus (AdcTriggerSignal_0). Edge-aligned generation handled by application. */
    {
        /* Cast literals to required enum types without assuming specific names */
        (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                        (IfxEgtm_TrigSource)0,           /* ATOM0 source selection (implementation-defined) */
                                        (IfxEgtm_TrigChannel)3,          /* ATOM0 CH3 */
                                        (IfxEgtm_Cfg_AdcTriggerSignal)0  /* AdcTriggerSignal_0 */);
    }

    /* 12) Configure LED GPIO as push-pull output for debug indication */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* No timing operations or explicit start — handled by config.syncStart */
}

/**
 * Update the three PWM duties atomically using shadow registers.
 * - Clamps each requested duty to [0, 100]
 * - Applies via IfxEgtm_Pwm_updateChannelsDutyImmediate so updates take effect at the next period boundary
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    if (requestDuty == NULL_PTR)
    {
        return; /* Defensive: ignore null input */
    }

    /* Copy and clamp to [0, 100] percent */
    float32 d0 = requestDuty[0]; if (d0 < 0.0f) d0 = 0.0f; if (d0 > 100.0f) d0 = 100.0f; g_egtmAtom3phInv.dutyCycles[0] = d0;
    float32 d1 = requestDuty[1]; if (d1 < 0.0f) d1 = 0.0f; if (d1 > 100.0f) d1 = 100.0f; g_egtmAtom3phInv.dutyCycles[1] = d1;
    float32 d2 = requestDuty[2]; if (d2 < 0.0f) d2 = 0.0f; if (d2 > 100.0f) d2 = 100.0f; g_egtmAtom3phInv.dutyCycles[2] = d2;

    /* Apply atomically to all three channels (shadow → active at period end) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
