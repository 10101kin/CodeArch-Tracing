/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for TC3xx GTM TOM 3-Phase complementary inverter PWM using IfxGtm_Pwm
 *
 * Notes:
 * - Follows iLLD initialization pattern for IfxGtm_Pwm (see authoritative examples)
 * - Uses TOM submodule, Cluster_1, center-aligned @ 20 kHz
 * - Complementary HS/LS outputs per phase with 1 us dead-time, minPulse 1 us
 * - FXCLK0 as TOM clock source; GTM/CMU is enabled inside guard if not already enabled
 * - No watchdog handling in this module (Cpu0_Main.c handles watchdog per AURIX pattern)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ============================== Macros and constants ============================== */

/* Channel count */
#define TOM3PH_NUM_CHANNELS            (3u)

/* PWM frequency (Hz) */
#define TOM3PH_PWM_FREQUENCY_HZ        (20000.0f)

/* Initial duty cycles in percent (0..100) */
#define PHASE_U_INIT_DUTY              (25.0f)
#define PHASE_V_INIT_DUTY              (50.0f)
#define PHASE_W_INIT_DUTY              (75.0f)

/* Duty step in percent */
#define PHASE_DUTY_STEP                (10.0f)

/* Dead-time configuration (seconds) */
#define PWM_DEAD_TIME_SEC_RISE         (1.0e-6f)   /* rising  edge dead-time: 1 us */
#define PWM_DEAD_TIME_SEC_FALL         (1.0e-6f)   /* falling edge dead-time: 1 us */

/* ISR priority (provider cpu0) */
#define ISR_PRIORITY_ATOM              (20)

/* Diagnostic LED on P13.0: compound macro used directly in IfxPort APIs */
#define LED                            &MODULE_P13, 0

/* User-requested validated TOM1 TOUT mappings (pin symbols) */
#define PHASE_U_HS_PIN_SYMBOL          (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS_PIN_SYMBOL          (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS_PIN_SYMBOL          (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS_PIN_SYMBOL          (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS_PIN_SYMBOL          (&IfxGtm_TOM1_5_TOUT2_P02_2_OUT)
#define PHASE_W_LS_PIN_SYMBOL          (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* ============================== Module state ============================== */

typedef struct
{
    IfxGtm_Pwm               pwm;                                  /* driver handle */
    IfxGtm_Pwm_Channel       channels[TOM3PH_NUM_CHANNELS];        /* persistent channel runtime */
    float32                  dutyCycles[TOM3PH_NUM_CHANNELS];      /* current duty (percent) */
    float32                  phases[TOM3PH_NUM_CHANNELS];          /* current phase (rad or percent scale as configured) */
    IfxGtm_Pwm_DeadTime      deadTimes[TOM3PH_NUM_CHANNELS];       /* per-channel dead-times */
} Tom3phInv_State;

IFX_STATIC Tom3phInv_State g_tom3phInv;

/* ============================== ISR and callbacks ============================== */

/*
 * ISR: toggles diagnostic LED only. Priority and provider per requirements.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/*
 * Period-event callback (assigned to InterruptConfig.periodEvent). Empty body by design.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================== Public API ============================== */

/*
 * Initialize GTM TOM 3-phase complementary PWM using unified IfxGtm_Pwm driver.
 * Follows mandatory configuration/enable pattern and stores persistent state.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for three channels: active-high HS, active-low LS, push-pull, pad driver set */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS_PIN_SYMBOL;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS_PIN_SYMBOL;
    output[0].polarity              = Ifx_ActiveState_high;          /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;           /* LS active low for complementary */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS_PIN_SYMBOL;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS_PIN_SYMBOL;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS_PIN_SYMBOL;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS_PIN_SYMBOL;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (1 us rising/falling). Min pulse is 1 us (enforced by driver if supported). */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_SEC_RISE;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_SEC_FALL;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_SEC_RISE;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_SEC_FALL;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_SEC_RISE;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_SEC_FALL;

    /* 5) Base channel interrupt configuration (period event only on channel 0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = (IfxGtm_Pwm_callBack)IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Per-channel configuration: logical channels 0..2, zero phase, initial duties */
    /* Channel 0 → Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;  /* logical index */
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;           /* only base channel has interrupt */

    /* Channel 1 → Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;  /* logical index */
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Channel 2 → Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;  /* logical index */
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main configuration */
    config.cluster              = IfxGtm_Cluster_1;                          /* Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;                  /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;               /* center-aligned */
    config.syncStart            = TRUE;                                       /* synchronized start */
    config.syncUpdateEnabled    = TRUE;                                       /* synchronized updates */
    config.numChannels          = (uint8)TOM3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = TOM3PH_PWM_FREQUENCY_HZ;                    /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                         /* FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;           /* DTM clock source */

    /* 8) Enable GTM/CMU clocks if not already enabled (guard) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        {
            float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize the PWM using persistent state (channels array and handle) */
    IfxGtm_Pwm_init(&g_tom3phInv.pwm, &g_tom3phInv.channels[0], &config);

    /* 10) Store persistent state for runtime updates */
    g_tom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_tom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_tom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_tom3phInv.phases[0] = channelConfig[0].phase;
    g_tom3phInv.phases[1] = channelConfig[1].phase;
    g_tom3phInv.phases[2] = channelConfig[2].phase;

    g_tom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure diagnostic LED GPIO as push-pull output (safe state) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update three phase duties in percent and apply atomically via immediate update API.
 * Wrap rule: if (duty+step) >= 100 then duty = 0; then duty += step.
 */
void updateGtmTom3phInvDuty(void)
{
    if ((g_tom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[2] = 0.0f; }

    g_tom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInv.pwm, (float32 *)g_tom3phInv.dutyCycles);
}
