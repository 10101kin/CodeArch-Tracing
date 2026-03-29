/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for TC3xx GTM TOM 3-phase inverter PWM using unified IfxGtm_Pwm API.
 *
 * Design intent:
 *  - 3 complementary PWM pairs on TOM1 with TOM1 Ch0 as timebase
 *  - Center-aligned, 20 kHz, FXCLK0 clock source
 *  - Dead-time: 0.5 us (both edges) for all pairs
 *  - Min pulse: 1.0 us (stored in module state)
 *  - Pins (user-specified):
 *      U_HS = TOM1 CH2 -> P00.3 (TOUT12), U_LS = TOM1 CH1 -> P00.2 (TOUT11)
 *      V_HS = TOM1 CH4 -> P00.5 (TOUT14), V_LS = TOM1 CH3 -> P00.4 (TOUT13)
 *      W_HS = TOM1 CH6 -> P00.7 (TOUT16), W_LS = TOM1 CH5 -> P00.6 (TOUT15)
 *
 * Mandatory patterns and constraints followed:
 *  - Uses IfxGtm_Pwm initConfig/init exactly as specified
 *  - GTM enable guard with CMU clock configuration inside the guard
 *  - OutputConfig and DtmConfig arrays per channel
 *  - InterruptConfig populated and attached to channels
 *  - Persistent state contains IfxGtm_Pwm handle and channels array
 *  - No watchdog calls in this driver
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (numeric constants from requirements) ========================= */
#define NUM_OF_CHANNELS                 (3U)
#define PWM_FREQUENCY_HZ                (20000.0f)       /* 20 kHz */
#define DEAD_TIME_US                    (0.5f)           /* 0.5 us */
#define MIN_PULSE_US                    (1.0f)           /* 1.0 us */
#define ISR_PRIORITY_ATOM               (3)

/* LED macro: compound form (port, pin) as required */
#define LED                             &MODULE_P13, 0

/* ========================= Pin routing macros (user-specified mapping) ========================= */
/* Note: These TOUT symbols are provided by the iLLD PinMap headers for GTM TOM.
 *       If integration environment does not provide them, replace with valid symbols
 *       from your device's PinMap or set to NULL_PTR and update during integration.
 */
#define PHASE_U_HS                (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS                (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS                (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS                (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS                (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS                (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ========================= Initial duty configuration (percent) ========================= */
#define PHASE_U_DUTY_INIT         (25.0f)
#define PHASE_V_DUTY_INIT         (50.0f)
#define PHASE_W_DUTY_INIT         (75.0f)

/* ========================= Module persistent state ========================= */
typedef struct
{
    IfxGtm_Pwm               pwm;                                    /* Driver handle */
    IfxGtm_Pwm_Channel       channels[NUM_OF_CHANNELS];              /* Persistent channel handles */
    float32                  dutyCycles[NUM_OF_CHANNELS];            /* Duty in percent [0..100] */
    float32                  phases[NUM_OF_CHANNELS];                /* Phase in percent [0..100] */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];             /* Per-channel dead-times */
    float32                  minPulse_us;                            /* Min on-time (requirement) */
} GtmTom3PhPwm_State;

IFX_STATIC GtmTom3PhPwm_State g_tom1_3phPwm;

/* ========================= ISR and period callback ========================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Initialization ========================= */
/**
 * Initialize the GTM unified PWM for three complementary pairs on TOM1.
 * - Center-aligned 20 kHz timebase on TOM1, base logical channel SubModule_Ch_0
 * - FXCLK0 as TOM clock source, sync start and sync update enabled
 * - 0.5 us dead-time, 1.0 us min pulse (stored in state)
 * - Pins mapped to P00.[2..7] as specified by the user
 */
void initGtmTomPwm(void)
{
    /* 1) Persistent state already declared at file scope (g_tom1_3phPwm) */

    /* 2) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Main configuration fields (filled below after channel setup) */

    /* 4) Output routing: six complementary outputs across three channels */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;          /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;           /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) Dead-time configuration (0.5 us on both edges), and min pulse (state only) */
    dtmConfig[0].deadTime.rising = 0.5e-6f;
    dtmConfig[0].deadTime.falling = 0.5e-6f;
    dtmConfig[1].deadTime.rising = 0.5e-6f;
    dtmConfig[1].deadTime.falling = 0.5e-6f;
    dtmConfig[2].deadTime.rising = 0.5e-6f;
    dtmConfig[2].deadTime.falling = 0.5e-6f;

    g_tom1_3phPwm.minPulse_us = MIN_PULSE_US;

    /* Interrupt configuration (base-channel driven). Attach to all channels per rules. */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Enable guard: enable GTM and configure CMU only if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Channel configurations (logical indices 0..2) */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;

    /* CH1 -> Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = &irqCfg;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = &irqCfg;

    /* Main config: TOM submodule, center alignment, FXCLK0, 20 kHz, sync start/update */
    config.cluster              = IfxGtm_Cluster_1;                  /* TOM1 is in Cluster 1 on TC3xx */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;/* DTM clock source */

    /* Initialize PWM driver with persistent state (channels array must be persistent) */
    IfxGtm_Pwm_init(&g_tom1_3phPwm.pwm, &g_tom1_3phPwm.channels[0], &config);

    /* Store initial duties, phases, and dead-times in module state (reflect effective setup) */
    g_tom1_3phPwm.dutyCycles[0] = channelConfig[0].duty;
    g_tom1_3phPwm.dutyCycles[1] = channelConfig[1].duty;
    g_tom1_3phPwm.dutyCycles[2] = channelConfig[2].duty;

    g_tom1_3phPwm.phases[0] = channelConfig[0].phase;
    g_tom1_3phPwm.phases[1] = channelConfig[1].phase;
    g_tom1_3phPwm.phases[2] = channelConfig[2].phase;

    g_tom1_3phPwm.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom1_3phPwm.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom1_3phPwm.deadTimes[2] = dtmConfig[2].deadTime;

    /* Ensure the base frequency is set to exactly 20 kHz, immediately if required */
    IfxGtm_Pwm_updateFrequencyImmediate(&g_tom1_3phPwm.pwm, PWM_FREQUENCY_HZ);

    /* LED output for ISR debug (minimal ISR: toggle LED) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Notes:
     * - Synchronized start is handled by config.syncStart = TRUE during init.
     * - Initial duties are applied via channelConfig[].duty in init; no extra update here.
     * - Dead-time is configured through dtmConfig per channel.
     */
}
