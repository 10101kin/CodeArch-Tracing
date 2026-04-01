/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: 3-phase complementary PWM on GTM TOM using unified IfxGtm_Pwm
 * - TC3xx family, KIT_A2G_TC387_5V_TFT
 * - Center-aligned, sync start, sync updates
 * - Shared TOM timer for update gating
 *
 * Notes:
 * - Watchdog disable must NOT be placed in this file.
 * - Pins must be validated for the target; placeholders are provided below.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "Gtm/Pwm/IfxGtm_Pwm.h"
#include "Gtm/Tom/Timer/IfxGtm_Tom_Timer.h"
#include "Port/IfxPort.h"
#include "Port/PinMap/IfxPort_PinMap.h"

/* === Configuration Macros (from user-confirmed migration values) === */
#define NUM_PWM_CHANNELS           (3u)
#define PWM_SWITCH_FREQ_HZ         (20000.0f)
#define PHASE_U_INIT_DUTY          (25.0f)
#define PHASE_V_INIT_DUTY          (50.0f)
#define PHASE_W_INIT_DUTY          (75.0f)
#define PHASE_DUTY_STEP            (10.0f)
#define ISR_PRIORITY_ATOM          (20)

/* LED: compound macro to pass (port, pin) */
#define LED                        &MODULE_P13, 0

/*
 * Pin routing placeholders (Validated pin symbols not provided for TC3xx in this context).
 * Replace NULL_PTR with the correct TOUT mappings from the GTM TOM1, Cluster_1 PinMap:
 *   e.g., &IfxGtm_TOM1_0_TOUTxx_P02_0_OUT etc., once validated for KIT_A2G_TC387_5V_TFT.
 */
#define PHASE_U_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTxx_P02_0_OUT */
#define PHASE_U_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTxx_P02_7_OUT */
#define PHASE_V_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTxx_P02_1_OUT */
#define PHASE_V_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTxx_P02_4_OUT */
#define PHASE_W_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTxx_P02_2_OUT */
#define PHASE_W_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTxx_P02_5_OUT */

/* Dead-time in seconds (EXACT migration value overrides any other requirement) */
#define PWM_DEAD_TIME_S            (5e-07f)

/* === Private module state === */
typedef struct
{
    IfxGtm_Pwm              pwm;                               /* PWM driver handle */
    IfxGtm_Tom_Timer        timer;                             /* Shared TOM timer for update gating */
    IfxGtm_Pwm_Channel      channels[NUM_PWM_CHANNELS];        /* Channels storage (must persist) */
    float32                 dutyCycles[NUM_PWM_CHANNELS];       /* Duty in percent */
    float32                 phases[NUM_PWM_CHANNELS];           /* Phase in percent (0..100) */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_PWM_CHANNELS];       /* Dead-times for each channel */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInvState;

/* === Forward declarations required by interrupt setup === */
void IfxGtm_periodEventFunction(void *data);

/* ISR: name and priority fixed by design. Minimal body: toggle LED only. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* === Internal callback (assigned to InterruptConfig.periodEvent) === */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* intentionally empty */
}

/* === Public API implementations === */

/**
 * Initialize GTM TOM 3-phase complementary PWM using IfxGtm_Pwm.
 * - Center-aligned, sync start, sync updates enabled
 * - DTM dead-time insertion configured via channel DTM settings
 * - Shared TOM timer prepared for update gating (disable/apply update sequence)
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Tom_Timer_Config     timerConfig;

    /* 2) Load default PWM configuration */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for three complementary pairs (HS/LS) */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;         /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;          /* LS active low */
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

    /* 4) DTM configuration for required dead-time; select DTM clock source */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_S;
    config.dtmClockSource = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 5) Channel configuration (logical indices 0,1,2) */
    /* 6) Interrupt configuration for base channel (period event only) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel 0 → Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* Base channel owns the interrupt */

    /* Channel 1 → Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2 → Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main configuration */
    config.cluster              = IfxGtm_Cluster_1;                         /* TOM1 cluster 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;                 /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;              /* Center-aligned */
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_PWM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_SWITCH_FREQ_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;               /* CLOCK SOURCE UNION: set .tom only */

    /* 8) GTM enable guard with mandatory CMU clock configuration */
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

    /* 9) Initialize unified PWM driver; store persistent state */
    IfxGtm_Pwm_init(&g_tom3phInvState.pwm, &g_tom3phInvState.channels[0], &config);

    g_tom3phInvState.dutyCycles[0] = channelConfig[0].duty;
    g_tom3phInvState.dutyCycles[1] = channelConfig[1].duty;
    g_tom3phInvState.dutyCycles[2] = channelConfig[2].duty;
    g_tom3phInvState.phases[0]     = channelConfig[0].phase;
    g_tom3phInvState.phases[1]     = channelConfig[1].phase;
    g_tom3phInvState.phases[2]     = channelConfig[2].phase;
    g_tom3phInvState.deadTimes[0]  = dtmConfig[0].deadTime;
    g_tom3phInvState.deadTimes[1]  = dtmConfig[1].deadTime;
    g_tom3phInvState.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 10) Initialize shared TOM timer for update gating */
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    {
        /* Keep defaults; ensure timer uses CMU clocks set above and does not collide with PWM TOUTs. */
        boolean timerOk = IfxGtm_Tom_Timer_init(&g_tom3phInvState.timer, &timerConfig);
        (void)timerOk; /* In production, handle failure (e.g., assert/log) */
    }

    /* 11) Configure LED pin as push-pull output (initial level low by default) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update three phase duty cycles (percent) with wrap rule, then apply atomically via TOM timer gating.
 * - Wrap rule per phase: if (duty + step) >= 100, reset to 0, then add step.
 * - Apply using: disableUpdate -> updateChannelsDutyImmediate -> applyUpdate
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule (explicit per-channel, no loops) */
    if ((g_tom3phInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInvState.dutyCycles[2] = 0.0f; }
    g_tom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply atomically at next timer boundary */
    IfxGtm_Tom_Timer_disableUpdate(&g_tom3phInvState.timer);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInvState.pwm, (float32 *)g_tom3phInvState.dutyCycles);
    IfxGtm_Tom_Timer_applyUpdate(&g_tom3phInvState.timer);
}
