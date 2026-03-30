/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm unified driver
 *
 * - Submodule: TOM
 * - Mode: Center-aligned, complementary
 * - Frequency: 20 kHz
 * - Dead-time: 1 us rising/falling via DTM
 * - Sync start + sync update enabled
 * - ISR: toggles LED P13.0 (provider CPU0, prio 20)
 *
 * Notes:
 * - Follow iLLD initialization patterns exactly (see embedded patterns in project requirements)
 * - No watchdog handling here (must be in CpuX main files only)
 * - No STM timing here (handled in Cpu0_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Configuration Macros ========================= */
#define NUM_OF_CHANNELS            (3u)            /* U, V, W complementary pairs */
#define PWM_FREQUENCY_HZ           (20000.0f)      /* 20 kHz */
#define ISR_PRIORITY_ATOM          (20u)           /* Priority 20 on CPU0 */
#define PHASE_DUTY_STEP            (10.0f)         /* percent step */
#define PHASE_U_DUTY               (25.0f)         /* percent */
#define PHASE_V_DUTY               (50.0f)         /* percent */
#define PHASE_W_DUTY               (75.0f)         /* percent */
#define DEADTIME_SEC               (1.0e-6f)       /* 1 us */

/* LED: compound macro (port, pin) */
#define LED                        &MODULE_P13, 0

/*
 * Pin routing placeholders:
 * No validated TOUT pin symbols were provided for the requested pads.
 * Replace NULL_PTR with valid &IfxGtm_TOM1_x_TOUTy_P02_z_OUT symbols during board integration.
 */
#define PHASE_U_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTx_P02_0_OUT */
#define PHASE_U_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTy_P02_7_OUT */
#define PHASE_V_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTx_P02_1_OUT */
#define PHASE_V_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTy_P02_4_OUT */
#define PHASE_W_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTx_P02_2_OUT */
#define PHASE_W_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTy_P02_5_OUT */

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm            pwm;                               /* Unified PWM handle */
    IfxGtm_Pwm_Channel    channels[NUM_OF_CHANNELS];         /* Persistent channel handles */
    float32               dutyCycles[NUM_OF_CHANNELS];        /* Percent duties (0..100) */
    float32               phases[NUM_OF_CHANNELS];            /* Phase offsets (deg or % domain as configured) */
    IfxGtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];         /* Per-channel dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInvState;

/* ========================= ISR and Callback (declared before init) ========================= */

/* ISR declaration with provider CPU0 and priority ISR_PRIORITY_ATOM */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/**
 * ISR body for GTM PWM-routed interrupt. Minimal body: toggle LED.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/**
 * Empty period-event callback required by the high-level PWM driver's interrupt configuration.
 * Do not perform any action here. Suppress unused parameter warning.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */

/**
 * Initialize the GTM for a 3-phase inverter PWM on TOM1 in center-aligned complementary mode
 * at 20 kHz with 1 us rising/falling dead-time. Synchronous start and synchronous update enabled.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* 2) Enable guard with CMU clocks per iLLD pattern */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 3) Load safe defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 4) Output configuration (complementary) */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;    /* HS active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;     /* LS active low  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM configuration (1 us rising/falling) */
    dtmConfig[0].deadTime.rising = DEADTIME_SEC;
    dtmConfig[0].deadTime.falling = DEADTIME_SEC;
    dtmConfig[1].deadTime.rising = DEADTIME_SEC;
    dtmConfig[1].deadTime.falling = DEADTIME_SEC;
    dtmConfig[2].deadTime.rising = DEADTIME_SEC;
    dtmConfig[2].deadTime.falling = DEADTIME_SEC;

    /* 5) Base-channel interrupt configuration (assigned to channel 0 only) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Per-channel configuration — logical indices 0..2 */
    /* Channel 0 → Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;     /* base channel */

    /* Channel 1 → Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;             /* only base channel has IRQ config */

    /* Channel 2 → Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 6) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                     /* Target Cluster 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;             /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;          /* center-aligned */
    config.syncStart            = TRUE;                                  /* sync start */
    config.syncUpdateEnabled    = TRUE;                                  /* sync update */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                    /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;    /* DTM clock source */

    /* 7) Initialize PWM with persistent state arrays */
    IfxGtm_Pwm_init(&g_gtmTom3phInvState.pwm, &g_gtmTom3phInvState.channels[0], &config);

    /* 8) Store initial state (percent duties, phases, and dead-times). Do not push an update here. */
    g_gtmTom3phInvState.dutyCycles[0] = PHASE_U_DUTY;
    g_gtmTom3phInvState.dutyCycles[1] = PHASE_V_DUTY;
    g_gtmTom3phInvState.dutyCycles[2] = PHASE_W_DUTY;

    g_gtmTom3phInvState.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInvState.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInvState.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 9) Configure LED GPIO as push-pull output (for ISR toggle) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update PWM duty cycles using percentage-based logic with synchronous shadow transfer.
 * Rule per channel i in {0,1,2}: if (duty+step) >= 100 then duty = 0; then duty += step.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap checks (separate if-blocks as required) */
    if ((g_gtmTom3phInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[2] = 0.0f; }

    /* Unconditional increment */
    g_gtmTom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply to hardware with immediate update API (driver manages sync/shadow) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInvState.pwm, (float32*)g_gtmTom3phInvState.dutyCycles);
}
