/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm (unified driver)
 *
 * Notes:
 * - Follows iLLD IfxGtm_Pwm init pattern with enable-guard and CMU clock setup
 * - TOM submodule, center-aligned, sync start/update enabled
 * - Single-ended outputs on TOM1 high-side pins (U/V/W = P00.3 / P00.5 / P00.7)
 * - Initial duties: 25%, 50%, 75% (U, V, W)
 * - Runtime update: +10% step with wrap into [10%, 90%) as specified
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD dependencies (source-only): */
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* =====================================================================================
 * Macros (use target naming; values from migration specification)
 * ===================================================================================== */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY              (20000.0f)
#define ISR_PRIORITY_ATOM          (3)

/* LED macro (compound form as requested; not used in this driver) */
#define LED                        &MODULE_P13, 0

/* Phase pin bindings (user-preserved TOM1 pins; HS used, LS defined for reference) */
#define PHASE_U_HS                 (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS                 (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS                 (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS                 (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS                 (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS                 (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* Initial duties (percent) */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)

/* Duty update step and bounds (percent) */
#define PHASE_DUTY_STEP            (10.0f)
#define PHASE_DUTY_MIN             (10.0f)
#define PHASE_DUTY_MAX             (90.0f)

/* =====================================================================================
 * Module state
 * ===================================================================================== */

typedef struct
{
    IfxGtm_Pwm             pwm;                                      /* driver handle */
    IfxGtm_Pwm_Channel     channels[NUM_OF_CHANNELS];                /* persistent channel state for driver */
    float32                dutyCycles[NUM_OF_CHANNELS];               /* percent */
    float32                phases[NUM_OF_CHANNELS];                   /* phase offset, deg or % (here 0.0) */
    IfxGtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];                /* stored dt per channel (seconds) */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInv = {0};

/* =====================================================================================
 * Public API implementation
 * ===================================================================================== */

/**
 * Initialize 3-phase center-aligned PWM on TOM1 using IfxGtm_Pwm unified driver.
 * Configuration is created locally, hardware is touched only inside GTM enable-guard.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Per-channel pin/output + initial duty/phase setup (single-ended HS only) */
    /* Channel 0: Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = NULL_PTR; /* single-ended */
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 1: Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = NULL_PTR; /* single-ended */
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 2: Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = NULL_PTR; /* single-ended */
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time: zero (no complementary outputs used) */
    dtmConfig[0].deadTime.rising = 0.0f; dtmConfig[0].deadTime.falling = 0.0f;
    dtmConfig[1].deadTime.rising = 0.0f; dtmConfig[1].deadTime.falling = 0.0f;
    dtmConfig[2].deadTime.rising = 0.0f; dtmConfig[2].deadTime.falling = 0.0f;

    /* Channel 0: U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;  /* logical ordinal */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = NULL_PTR;                  /* no ISR */

    /* Channel 1: V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;  /* logical ordinal */
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;                  /* no ISR */

    /* Channel 2: W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;  /* logical ordinal */
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;                  /* no ISR */

    /* 4) Main configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;        /* TOM uses FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;/* per enum spec */

    /* 5) GTM enable-guard and CMU clocks setup (MANDATORY pattern) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_tom3phInv.pwm, &g_tom3phInv.channels[0], &config);

    /* 7) Store persistent state (duties, phases, deadtimes) */
    g_tom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_tom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_tom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_tom3phInv.phases[0] = channelConfig[0].phase;
    g_tom3phInv.phases[1] = channelConfig[1].phase;
    g_tom3phInv.phases[2] = channelConfig[2].phase;

    g_tom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInv.deadTimes[2] = dtmConfig[2].deadTime;
}

/**
 * Update the three phase duty cycles: +10% each step; if new value >= 90%, wrap to 10%.
 * Apply synchronously via unified driver's immediate multi-channel update API.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Add first, then wrap to maintain exact behavior_description */
    g_tom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    if (g_tom3phInv.dutyCycles[0] >= PHASE_DUTY_MAX) { g_tom3phInv.dutyCycles[0] = PHASE_DUTY_MIN; }
    if (g_tom3phInv.dutyCycles[1] >= PHASE_DUTY_MAX) { g_tom3phInv.dutyCycles[1] = PHASE_DUTY_MIN; }
    if (g_tom3phInv.dutyCycles[2] >= PHASE_DUTY_MAX) { g_tom3phInv.dutyCycles[2] = PHASE_DUTY_MIN; }

    /* Synchronous multi-channel shadow transfer */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInv.pwm, (float32 *)g_tom3phInv.dutyCycles);
}
