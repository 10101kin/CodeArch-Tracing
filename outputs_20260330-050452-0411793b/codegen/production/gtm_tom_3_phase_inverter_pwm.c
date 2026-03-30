/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: 3-phase center-aligned PWM using GTM TOM via IfxGtm_Pwm
 * - 3 complementary pairs (U,V,W)
 * - 20 kHz switching, 1 us dead-time (rising/falling)
 * - Unified driver (IfxGtm_Pwm) with InterruptConfig period callback hook
 * - GTM enable guard with CMU clock configuration (GCLK, FXCLK0, CLK0)
 * - LED/debug pin toggled in ISR (priority per macro)
 *
 * Notes:
 * - Watchdog disable MUST NOT be placed in this file (handled in CpuX_Main.c).
 * - No STM delays here; main loop schedules updates.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ============================ Configuration macros ============================ */
#define NUM_OF_CHANNELS           (3u)
#define PWM_FREQUENCY             (20000.0f)        /* 20 kHz */
#define ISR_PRIORITY_ATOM         (20u)

/* LED/debug pin: compound macro (port,pin) */
#define LED                       &MODULE_P13, 0    /* P13.0 */

/* Initial duties in percent and step */
#define PHASE_U_DUTY              (25.0f)
#define PHASE_V_DUTY              (50.0f)
#define PHASE_W_DUTY              (75.0f)
#define PHASE_DUTY_STEP           (10.0f)

/*
 * Pin routing: use validated pin symbols only. None are available in this context,
 * so provide NULL_PTR placeholders to be replaced during board integration.
 * Required mapping (KIT_A2G_TC387_5V_TFT, TC38x LFBGA516, TOM1, cluster 1):
 *   U: P02.0 (HS) / P02.7 (LS)
 *   V: P02.1 (HS) / P02.4 (LS)
 *   W: P02.2 (HS) / P02.5 (LS)
 */
#define PHASE_U_HS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTxx_P02_0_OUT */
#define PHASE_U_LS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTxx_P02_7_OUT */
#define PHASE_V_HS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTxx_P02_1_OUT */
#define PHASE_V_LS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTxx_P02_4_OUT */
#define PHASE_W_HS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTxx_P02_2_OUT */
#define PHASE_W_LS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTxx_P02_5_OUT */

/* ============================== Module state ================================= */
typedef struct
{
    IfxGtm_Pwm               pwm;                               /* PWM driver handle */
    IfxGtm_Pwm_Channel       channels[NUM_OF_CHANNELS];         /* Persistent channel state (driver-owned) */
    float32                  dutyCycles[NUM_OF_CHANNELS];       /* Duty in percent [U,V,W] */
    float32                  phases[NUM_OF_CHANNELS];           /* Phase shift in percent (0..100) */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];        /* Dead-time per channel */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInv;

/* ========================== ISR and callback hooks =========================== */
/* ISR forward declaration with Ifx macro; priority via ISR_PRIORITY_ATOM */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/*
 * Empty period-event callback assigned via InterruptConfig.
 * The PWM driver invokes this at period events; no user processing here.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty */
}

/*
 * Minimal ISR: toggle LED/debug pin. Do not call driver APIs here.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* =============================== Public API ================================== */
/*
 * Initialize a 3-phase, center-aligned PWM on TOM with complementary pairs.
 * - 20 kHz, 1 us dead-time on both edges
 * - TOM1, cluster 1, FXCLK0 as TOM clock source
 * - Synchronized start and synchronized updates enabled
 * - Period-event callback registered; ISR toggles LED
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* 2) Load defaults from unified PWM driver */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for 3 complementary pairs (U,V,W) */
    /* Complementary polarity convention: HS active HIGH, LS active LOW */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM (dead-time) configuration: 1 us on both edges */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Base channel interrupt configuration (attached to channel index 0 only) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations (logical indices 0..2 for TOM submodule) */
    /* U phase on channel 0 */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* base channel gets the interrupt */

    /* V phase on channel 1 */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;         /* only base channel has interrupt */

    /* W phase on channel 2 */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;         /* only base channel has interrupt */

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                 /* cluster index 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;         /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                    /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;               /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock; /* DTM clock source */

    /* 8) GTM enable guard: enable and (re)configure CMU only if disabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver (channels array must be persistent) */
    IfxGtm_Pwm_init(&g_tom3phInv.pwm, &g_tom3phInv.channels[0], &config);

    /* 10) Persist initial state (duties, phases, dead-times) */
    g_tom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_tom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_tom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_tom3phInv.phases[0] = channelConfig[0].phase;
    g_tom3phInv.phases[1] = channelConfig[1].phase;
    g_tom3phInv.phases[2] = channelConfig[2].phase;

    g_tom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED/debug GPIO as output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update U/V/W duty cycles by STEP with wrap-to-0-then-add-step behavior, then
 * apply immediately via the unified PWM driver's immediate update API.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap checks (separate per phase, per specification) */
    if ((g_tom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[2] = 0.0f; }

    /* Unconditional add of STEP */
    g_tom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply update immediately */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInv.pwm, (float32 *)g_tom3phInv.dutyCycles);
}
