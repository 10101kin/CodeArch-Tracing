/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-Phase complementary PWM using IfxGtm_Pwm
 * Target: KIT_A2G_TC387_5V_TFT (TC3xx)
 * Notes:
 *  - Follows iLLD patterns for IfxGtm_Pwm initialization and GTM CMU clock enable guard
 *  - No watchdog handling here (must be done only in CpuX_Main.c)
 *  - No STM delays or scheduling in this module
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ===================== Macros (configuration constants) ===================== */
#define GTM_TOM3PH_NUM_CHANNELS          (3u)
#define GTM_TOM3PH_PWM_FREQUENCY_HZ      (20000.0f)
#define GTM_TOM3PH_ISR_PRIORITY          (20)

#define PHASE_U_DUTY_INIT                (25.0f)
#define PHASE_V_DUTY_INIT                (50.0f)
#define PHASE_W_DUTY_INIT                (75.0f)
#define PHASE_DUTY_STEP                  (10.0f)
#define PHASE_DUTY_MIN                   (10.0f)   /* reserved (not enforced in update per wrap rule) */
#define PHASE_DUTY_MAX                   (90.0f)   /* reserved (not enforced in update per wrap rule) */

#define PWM_DEAD_TIME_S                  (5.0e-07f)  /* 0.5 us (migration-confirmed) */
#define PWM_MIN_PULSE_TIME_S             (1.0e-06f)

/* LED macro (compound form: expands to two arguments) */
#define LED                              &MODULE_P13, 0

/* ===================== Pin routing placeholders ===================== */
/*
 * No validated TOUT pin symbols are available in this context. To integrate on
 * target hardware, replace NULL_PTR with the proper TOM1 Cluster_1 TOUT maps matching:
 *   Phase U: HS P02.0, LS P02.7
 *   Phase V: HS P02.1, LS P02.4
 *   Phase W: HS P02.2, LS P02.5
 * Example (to be validated in PinMap headers):
 *   #define PHASE_U_HS  (&IfxGtm_TOM1_0_TOUTxx_P02_0_OUT)
 */
#define PHASE_U_HS                        (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTxx_P02_0_OUT */
#define PHASE_U_LS                        (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTxx_P02_7_OUT */
#define PHASE_V_HS                        (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTxx_P02_1_OUT */
#define PHASE_V_LS                        (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTxx_P02_4_OUT */
#define PHASE_W_HS                        (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTxx_P02_2_OUT */
#define PHASE_W_LS                        (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTxx_P02_5_OUT */

/* ===================== Module state ===================== */
typedef struct
{
    IfxGtm_Pwm              pwm;                                   /* driver handle */
    IfxGtm_Tom_Timer        timer;                                 /* shared TOM timer for gating */
    IfxGtm_Pwm_Channel      channels[GTM_TOM3PH_NUM_CHANNELS];     /* persistent runtime channels */
    float32                 dutyCycles[GTM_TOM3PH_NUM_CHANNELS];   /* percent (0..100) */
    float32                 phases[GTM_TOM3PH_NUM_CHANNELS];       /* degrees/percent phase basis */
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM3PH_NUM_CHANNELS];    /* per-channel deadtime copy */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3phState; /* persistent module state */

/* ===================== ISR and callback (must appear before init) ===================== */
/**
 * GTM ISR: toggles LED only. No driver calls inside.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, GTM_TOM3PH_ISR_PRIORITY)
{
    IfxPort_togglePin(LED);
}

/**
 * Empty period-event callback for IfxGtm_Pwm interrupt routing (signature-only).
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ===================== Public API implementations ===================== */
/**
 * Initialize 3-phase complementary PWM on TOM1 Cluster_1 using IfxGtm_Pwm.
 * - Center-aligned, 20 kHz, sync start + sync updates
 * - DTM deadtime configured per channel
 * - GTM CMU enable guard applied
 * - Shared TOM timer initialized for update gating
 * - LED configured for ISR toggle indication
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_ChannelConfig     channelConfig[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig      output[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   interruptConfig;
    IfxGtm_Tom_Timer_Config      timerConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for three complementary pairs (HS/LS) */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;  /* HS */
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;  /* LS */
    output[0].polarity                = Ifx_ActiveState_high;              /* HS active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;               /* LS active low  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM dead-time configuration (migration-confirmed 0.5 us both edges) */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_S;

    /* 5) Channel configuration: logical indices 0..2 with initial phase/duty */
    /* Phase U on logical Ch_0 */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    /* Interrupt only on base channel (0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)GTM_TOM3PH_ISR_PRIORITY;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    /* Phase V on logical Ch_1 */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Phase W on logical Ch_2 */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                      /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;              /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;           /* center-aligned */
    config.syncStart            = TRUE;                                   /* synchronized start */
    config.syncUpdateEnabled    = TRUE;                                   /* synchronized update */
    config.numChannels          = (uint8)GTM_TOM3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM3PH_PWM_FREQUENCY_HZ;           /* 20 kHz */
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;            /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;      /* DTM from CMU CLK0 */

    /* 8) GTM enable guard + CMU clock setup (MANDATORY: inside guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM (stores runtime into persistent state channels) */
    IfxGtm_Pwm_init(&g_gtmTom3phState.pwm, &g_gtmTom3phState.channels[0], &config);

    /* Persist initial duties/phases/dead-times in module state */
    g_gtmTom3phState.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3phState.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3phState.dutyCycles[2] = PHASE_W_DUTY_INIT;
    g_gtmTom3phState.phases[0]     = 0.0f;
    g_gtmTom3phState.phases[1]     = 0.0f;
    g_gtmTom3phState.phases[2]     = 0.0f;
    g_gtmTom3phState.deadTimes[0].rising = PWM_DEAD_TIME_S; g_gtmTom3phState.deadTimes[0].falling = PWM_DEAD_TIME_S;
    g_gtmTom3phState.deadTimes[1].rising = PWM_DEAD_TIME_S; g_gtmTom3phState.deadTimes[1].falling = PWM_DEAD_TIME_S;
    g_gtmTom3phState.deadTimes[2].rising = PWM_DEAD_TIME_S; g_gtmTom3phState.deadTimes[2].falling = PWM_DEAD_TIME_S;

    /* 10) Shared TOM timer for update gating */
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    /* Note: Timer base/timebase/channel selection should be aligned to PWM period and non-colliding TOM ch.
       Use defaults provided by initConfig here; integration may refine timerConfig fields as needed. */
    (void)IfxGtm_Tom_Timer_init(&g_gtmTom3phState.timer, &timerConfig); /* check return if needed during integration */

    /* 11) LED GPIO configuration (push-pull, initial low) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update three phase duty cycles with wrap rule and apply atomically using TOM timer gating.
 * Algorithm per phase i:
 *   if ((duty[i] + step) >= 100) duty[i] = 0;  // wrap
 *   duty[i] += step;                            // unconditional add
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap then increment (no additional clamp) */
    if ((g_gtmTom3phState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[2] = 0.0f; }
    g_gtmTom3phState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Synchronized commit via TOM timer gating */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phState.timer);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phState.pwm, (float32 *)g_gtmTom3phState.dutyCycles);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phState.timer);
}
