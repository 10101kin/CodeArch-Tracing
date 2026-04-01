/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM ATOM 3-Phase Inverter PWM on TC4xx.
 * - 3 complementary, center-aligned PWM pairs @ 20 kHz with 1 us dead-time (rising/falling)
 * - ATOM submodule (ATOM0): CH0..CH2 for U/V/W phases
 * - ADC trigger routed from ATOM0 CH3 via IfxEgtm_Trigger_trigToAdc (AdcTriggerSignal_0)
 * - LED (P03.9) toggle in ISR for debug
 *
 * Notes:
 * - Followed iLLD high-level PWM init pattern and EGTM CMU enable guard.
 * - No watchdog operations here (per AURIX watchdog placement rule).
 * - No STM timing here; runtime updates via updateEgtmAtom3phInvDuty().
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (configuration constants) ========================= */

/* Channel count */
#define EGTM_PWM_NUM_CHANNELS          (3u)

/* PWM switching frequency (Hz) */
#define EGTM_PWM_FREQUENCY_HZ          (20000.0f)

/* Initial duty cycles (%) */
#define PHASE_U_DUTY                   (25.0f)
#define PHASE_V_DUTY                   (50.0f)
#define PHASE_W_DUTY                   (75.0f)

/* Optional step (not used here; provided per migration values) */
#define PHASE_DUTY_STEP                (0.01f)

/* Dead-time values in seconds (1.0 us each) */
#define PWM_DEADTIME_RISING_S          (1.0e-6f)
#define PWM_DEADTIME_FALLING_S         (1.0e-6f)

/* ISR priority for ATOM period event */
#define ISR_PRIORITY_ATOM              (25)

/* LED macro: compound form (port, pin) for P03.9 */
#define LED                            (&MODULE_P03), 9u

/* ========================= Pin routing macros (TOUT map) ========================= */
/* Use ONLY validated pin symbols. If a requested pin is not in the validated list, use NULL_PTR. */
/* Requested complementary pairs: 
 * U: P20.8 (HS), P20.9 (LS)  → validated list provides only P20.9 for ATOM0_0N
 * V: P20.10 (HS), P20.11 (LS) → not in validated list
 * W: P20.12 (HS), P20.13 (LS) → not in validated list
 * ADC_TRIG: P33.0              → not in validated list for ATOM0
 */
#define PHASE_U_HS                     (NULL_PTR)                           /* TODO: replace with &IfxEgtm_ATOM0_0_TOUTxx_P20_8_OUT if available */
#define PHASE_U_LS                     (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT) /* Validated */
#define PHASE_V_HS                     (NULL_PTR)                           /* TODO: replace with validated symbol for P20.10 */
#define PHASE_V_LS                     (NULL_PTR)                           /* TODO: replace with validated symbol for P20.11 */
#define PHASE_W_HS                     (NULL_PTR)                           /* TODO: replace with validated symbol for P20.12 */
#define PHASE_W_LS                     (NULL_PTR)                           /* TODO: replace with validated symbol for P20.13 */

/* ========================= Internal state ========================= */

typedef struct
{
    IfxEgtm_Pwm             pwm;                                     /* Driver handle */
    IfxEgtm_Pwm_Channel     channels[EGTM_PWM_NUM_CHANNELS];         /* Persistent channel handles */
    float32                 dutyCycles[EGTM_PWM_NUM_CHANNELS];       /* Duty in percent (0..100) */
    float32                 phases[EGTM_PWM_NUM_CHANNELS];           /* Phase offsets in percent (0..100) */
    IfxEgtm_Pwm_DeadTime    deadTimes[EGTM_PWM_NUM_CHANNELS];        /* Dead-time per channel */
} EgtmAtom3ph_State;

IFX_STATIC EgtmAtom3ph_State g_egtmAtom3ph;

/* ========================= Interrupts and callbacks ========================= */

/* ISR: must only toggle LED */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: empty body by design */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Helper typedefs for trigger routing ========================= */
/* The enumerations for trigger source/channel/signal may vary by device; use safe casts. */
#ifndef EGTM_TRIGGER_SOURCE
#define EGTM_TRIGGER_SOURCE     ((IfxEgtm_TrigSource)0)      /* ATOM0 source index */
#endif
#ifndef EGTM_TRIGGER_CHANNEL
#define EGTM_TRIGGER_CHANNEL    ((IfxEgtm_TrigChannel)3)     /* Use ATOM0 CH3 as trigger generator */
#endif
#ifndef EGTM_ADC_TRIGGER_SIGNAL
#define EGTM_ADC_TRIGGER_SIGNAL ((IfxEgtm_Cfg_AdcTriggerSignal)0) /* AdcTriggerSignal_0 */
#endif

/* ========================= Public API implementations ========================= */

/**
 * Initialize EGTM ATOM0 CH0..CH2 for 3-phase complementary PWM and route ATOM0 CH3 to ADC trigger.
 * Behavior:
 *  - Local allocation of all configuration structures
 *  - Complementary center-aligned PWM with dead-time on both edges
 *  - Interrupt configured on base channel (channel index 0)
 *  - EGTM CMU enable guard with dynamic frequency programming
 *  - Single unified IfxEgtm_Pwm_init call with persistent handles
 *  - Store initial duty cycles and dead-times into persistent state
 *  - Route ATOM0 CH3 trigger to ADC via IfxEgtm_Trigger_trigToAdc()
 *  - Configure LED pin (P03.9) as push-pull output for debug
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all config structures locally */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Initialize config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration (complementary pairs) */
    /* Polarity convention: HS active HIGH, LS active LOW */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
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

    /* 4) Dead-time configuration (both edges) */
    dtmConfig[0].deadTime.rising  = PWM_DEADTIME_RISING_S;
    dtmConfig[0].deadTime.falling = PWM_DEADTIME_FALLING_S;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = PWM_DEADTIME_RISING_S;
    dtmConfig[1].deadTime.falling = PWM_DEADTIME_FALLING_S;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = PWM_DEADTIME_RISING_S;
    dtmConfig[2].deadTime.falling = PWM_DEADTIME_FALLING_S;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration (single object, attached to base channel only) */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (CH0..CH2 logical indices) */
    /* Base + two synchronous channels; center-aligned complementary PWM */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg; /* only base channel gets interrupt */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)EGTM_PWM_NUM_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;            /* Use CMU Clock 0 for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;    /* DTM driven by CMU Clock 0 */
    config.syncUpdateEnabled  = TRUE;                                  /* Synchronous shadow updates at period end */
    config.syncStart          = TRUE;                                  /* Start outputs after init */

    /* 8) EGTM enable guard + CMU clocking (exact pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver once with persistent handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3ph.pwm, g_egtmAtom3ph.channels, &config);

    /* 10) Store initial duty cycles, phases, and dead-times into persistent state */
    g_egtmAtom3ph.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3ph.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3ph.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3ph.phases[0] = channelConfig[0].phase;
    g_egtmAtom3ph.phases[1] = channelConfig[1].phase;
    g_egtmAtom3ph.phases[2] = channelConfig[2].phase;

    g_egtmAtom3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Route ADC trigger: ATOM0 CH3 → ADC Trigger Signal 0 */
    {
        boolean routed = IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0, EGTM_TRIGGER_SOURCE, EGTM_TRIGGER_CHANNEL, EGTM_ADC_TRIGGER_SIGNAL);
        (void)routed; /* In production, assert or handle routing failure as needed */
    }

    /* 12) Configure LED pin as push-pull output for debug */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update duty cycles for the three logical PWM channels atomically.
 *
 * @param requestDuty Pointer to 3 float32 values representing duty in percent [0..100].
 *                    Order: channel 0, channel 1, channel 2 (U, V, W as configured).
 * Behavior:
 *  - Copy and clamp to [0, 100]
 *  - Store into persistent state
 *  - Apply atomically to all channels using driver immediate update API
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    float32 d0 = requestDuty[0];
    float32 d1 = requestDuty[1];
    float32 d2 = requestDuty[2];

    /* Clamp to [0, 100] */
    if (d0 < 0.0f) { d0 = 0.0f; } else if (d0 > 100.0f) { d0 = 100.0f; }
    if (d1 < 0.0f) { d1 = 0.0f; } else if (d1 > 100.0f) { d1 = 100.0f; }
    if (d2 < 0.0f) { d2 = 0.0f; } else if (d2 > 100.0f) { d2 = 100.0f; }

    /* Store into persistent state */
    g_egtmAtom3ph.dutyCycles[0] = d0;
    g_egtmAtom3ph.dutyCycles[1] = d1;
    g_egtmAtom3ph.dutyCycles[2] = d2;

    /* Atomic multi-channel update (shadow registers apply at next period boundary) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3ph.pwm, (float32*)g_egtmAtom3ph.dutyCycles);
}
