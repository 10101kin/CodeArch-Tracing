/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: 3-phase complementary PWM on GTM TOM1 Cluster_1 using IfxGtm_Pwm.
 * - Center-aligned, 20 kHz, 1 us HW dead time (DTM), sync start + sync update.
 * - Complementary outputs mapped to user-requested pins on KIT_A2G_TC387_5V_TFT.
 * - Immediate multi-channel duty updates in percent (0..100), step=10 with wrap rule.
 * - GTM enable/clock guard per iLLD pattern; no watchdog calls here.
 * - ISR toggles P13.0. Period-event callback stub is empty.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ===========================
 * Configuration Macros
 * =========================== */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY              (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM          (20)

/* Initial duty percentages (0..100) */
#define PHASE_U_DUTY_INIT          (25.0f)
#define PHASE_V_DUTY_INIT          (50.0f)
#define PHASE_W_DUTY_INIT          (75.0f)

/* Duty update step (percent) */
#define PHASE_DUTY_STEP            (10.0f)

/* LED: P13.0 (compound macro for port, pin) */
#define LED                        &MODULE_P13, 0

/* ===========================
 * Validated Pin Symbols (TOUT maps)
 * Use exactly the user-requested pins from validated list.
 * =========================== */
#define PHASE_U_HS                 (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                 (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                 (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                 (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS                 (&IfxGtm_TOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS                 (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* ===========================
 * Module State
 * =========================== */

typedef struct
{
    IfxGtm_Pwm           pwm;                                 /* PWM driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];           /* Persistent channel handles */
    float32              dutyCycles[NUM_OF_CHANNELS];          /* Percent duties (0..100) */
    float32              phases[NUM_OF_CHANNELS];              /* Phase offsets (rad or deg) if needed */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];           /* Applied HW dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInvState;

/* ===========================
 * Private ISR and Callback (placed before init as required)
 * =========================== */

/* ISR: declared with IFX_INTERRUPT and minimal body (toggle LED). */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: empty stub assigned via InterruptConfig. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ===========================
 * Public API
 * =========================== */

/**
 * Initializes 3-phase complementary PWM on GTM TOM1 Cluster_1 using IfxGtm_Pwm.
 * - Center-aligned, 20 kHz, 1 us rising/falling dead time.
 * - Sync start and sync update enabled.
 * - Complementary outputs:
 *     U: HS=P02.0, LS=P02.7
 *     V: HS=P02.1, LS=P02.4
 *     W: HS=P02.2, LS=P02.5
 * - Interrupt configured for period events (priority 20, CPU0) with empty callback.
 * - LED P13.0 configured as push-pull output for ISR toggling.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load default config */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: 3 logical channels with complementary pins */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;      /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;       /* LS active LOW for complementary PWM */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM: 1.0us rising/falling */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* InterruptConfig: CPU0, prio=20, period event */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;    /* period notify */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel configuration (logical indices 0..2) */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig; /* base channel period ISR */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 4) Complete main config */
    config.cluster              = IfxGtm_Cluster_1;                 /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;         /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;      /* Center aligned */
    config.syncStart            = TRUE;                              /* Sync start */
    config.syncUpdateEnabled    = TRUE;                              /* Sync update */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                     /* 20kHz */
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;                /* FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM from CMU CLK0 */

    /* 5) GTM enable guard (exact pattern) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInvState.pwm, &g_gtmTom3phInvState.channels[0], &config);

    /* 7) Store initial duties and dead-times into module state (no update call needed) */
    g_gtmTom3phInvState.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3phInvState.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3phInvState.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtmTom3phInvState.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInvState.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInvState.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure LED/debug GPIO P13.0 as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Updates U/V/W duty cycles with STEP=10 percent using wrap rule:
 *   if ((duty + STEP) >= 100) duty = 0; duty += STEP;  // for each phase
 * Applies the new values with immediate multi-channel update (percent units).
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule per phase (explicit, no loops) */
    if ((g_gtmTom3phInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Coherent immediate update (percent units) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInvState.pwm, (float32*)g_gtmTom3phInvState.dutyCycles);
}
