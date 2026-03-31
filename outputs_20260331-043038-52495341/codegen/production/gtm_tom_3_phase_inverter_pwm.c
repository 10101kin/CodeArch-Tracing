/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief GTM TOM 3-phase complementary PWM driver (unified IfxGtm_Pwm API)
 *
 * - Submodule: TOM
 * - 3 logical channels → 6 physical outputs (complementary pairs)
 * - Center-aligned, synchronous start/update, FXCLK0 clock source
 * - Period-event routed via InterruptConfig; ISR toggles LED (debug)
 *
 * Notes:
 * - Watchdog handling must be done only in CpuX_Main.c (not here).
 * - No STM timing logic here; scheduling belongs to the application loop.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"
#include "IfxCpu.h"

/* =========================================================
 * Configuration Macros (user-confirmed values)
 * ========================================================= */
#define GTM_TOM_3PH_NUM_CHANNELS       (3u)
#define PWM_SWITCHING_FREQUENCY_HZ     (20000.0f)     /* 20 kHz */
#define PHASE_U_DUTY_INIT              (25.0f)        /* % */
#define PHASE_V_DUTY_INIT              (50.0f)        /* % */
#define PHASE_W_DUTY_INIT              (75.0f)        /* % */
#define PHASE_DUTY_STEP                (10.0f)        /* % */
#define PHASE_DUTY_MIN                 (10.0f)        /* % */
#define PHASE_DUTY_MAX                 (90.0f)        /* % */
#define PWM_DEAD_TIME                  (1.0e-6f)      /* 1 us (user-confirmed) */
#define PWM_MIN_PULSE_TIME             (1.0e-6f)      /* 1 us */

/* ISR priority for GTM TOM/ATOM service request line used by unified driver */
#define ISR_PRIORITY_ATOM              (3u)

/* LED (debug) on Port13.0: use compound macro so it can be passed as 2 args */
#define LED                             &MODULE_P13, 0u

/* =========================================================
 * TOUT Pin Macros (TOM1 channel routing as requested)
 * ========================================================= */
/* Phase U: CH2 (HS) on P00.3, CH1 (LS) on P00.2 */
#define PHASE_U_HS    (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS    (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)

/* Phase V: CH4 (HS) on P00.5, CH3 (LS) on P00.4 */
#define PHASE_V_HS    (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS    (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)

/* Phase W: CH6 (HS) on P00.7, CH5 (LS) on P00.6 */
#define PHASE_W_HS    (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS    (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* =========================================================
 * Module State
 * ========================================================= */
typedef struct
{
    IfxGtm_Pwm              pwm;                                      /* persistent driver handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM_3PH_NUM_CHANNELS];       /* persistent channels */
    float32                 dutyCycles[GTM_TOM_3PH_NUM_CHANNELS];     /* duty in percent */
    float32                 phases[GTM_TOM_3PH_NUM_CHANNELS];         /* phase in degrees or percent (API expects float) */
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM_3PH_NUM_CHANNELS];      /* per-pair dead-times */
} GTM_TOM_3Ph_State;

IFX_STATIC GTM_TOM_3Ph_State g_gtmTom3PhState;

/* =========================================================
 * ISR and Callback Declarations (must appear before init)
 * ========================================================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

void interruptGtmAtom(void)
{
    /* Minimal ISR: toggle LED/debug pin only */
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    /* Period event callback used by the unified PWM driver; keep empty */
    (void)data;
}

/* =========================================================
 * Initialization
 * ========================================================= */
/**
 * @brief Initialize a persistent 3-channel complementary PWM on the TOM sub-module using IfxGtm_Pwm.
 *
 * Algorithm summary:
 *  - Configure output pins (HS + complementary LS) for three phases
 *  - Configure DTM dead-time per complementary pair
 *  - Set up channel configs with initial duty and phase
 *  - Set InterruptConfig on base channel (index 0) with periodEvent callback
 *  - Set main config: TOM, center-aligned, syncStart/update, 20kHz, FXCLK0, DTM clock
 *  - Enable-guard GTM + CMU clocks (GCLK, CLK0, FXCLK domain)
 *  - Initialize PWM with persistent handle and channels
 *  - Store initial duties/dead-times to module state
 *  - Configure LED/debug GPIO (push-pull)
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_ChannelConfig     channelConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig      output[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   interruptConfig;

    /* 2) Populate defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (complementary pairs) */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;    /* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;     /* LS active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration (dead-time per complementary pair) */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME;

    /* 5) Interrupt configuration (period event on base channel only) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2) */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig; /* base period event */

    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;         /* no ISR on this channel */

    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;         /* no ISR on this channel */

    /* 7) Main configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM_3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_SWITCHING_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;               /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock source */

    /* 8) Enable-guard: GTM + CMU clocks setup (inside guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent state (applies initial duties atomically) */
    IfxGtm_Pwm_init(&g_gtmTom3PhState.pwm, &g_gtmTom3PhState.channels[0], &config);

    /* 10) Store initial state for later updates */
    g_gtmTom3PhState.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3PhState.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3PhState.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3PhState.phases[0] = channelConfig[0].phase;
    g_gtmTom3PhState.phases[1] = channelConfig[1].phase;
    g_gtmTom3PhState.phases[2] = channelConfig[2].phase;

    g_gtmTom3PhState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3PhState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3PhState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED/debug GPIO */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =========================================================
 * Duty Update
 * ========================================================= */
/**
 * @brief Update the three phase duties in percent and apply them immediately.
 *
 * Implements the immediate multi-channel update using the unified driver.
 */
void GTM_TOM_3_Phase_Inverter_PWM_updateDuties(void)
{
    /* Duty wrap rule: check against 100, reset to 0, then add step (no loops) */
    if ((g_gtmTom3PhState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3PhState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3PhState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhState.dutyCycles[2] = 0.0f; }

    g_gtmTom3PhState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3PhState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3PhState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate, synchronous update of all three complementary pairs */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3PhState.pwm, (float32 *)g_gtmTom3PhState.dutyCycles);
}
