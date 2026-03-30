/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for 3-phase complementary PWM using GTM TOM and IfxGtm_Pwm
 * - Center-aligned, 20 kHz, 1 us dead-time on both edges
 * - 3 complementary pairs (U, V, W)
 * - Unified IfxGtm_Pwm API with synchronized start/update
 *
 * Notes:
 * - Follows authoritative iLLD initialization pattern and union clock-source rule
 * - No watchdog handling here (must be in CpuX_Main.c)
 * - No STM timing here (scheduling in CpuX_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ======================== Configuration Macros ======================== */
#define GTM_TOM_PWM_NUM_CHANNELS       (3u)
#define GTM_TOM_PWM_FREQUENCY_HZ       (20000.0f)   /* 20 kHz */
#define GTM_TOM_PWM_DUTY_INIT_U        (25.0f)
#define GTM_TOM_PWM_DUTY_INIT_V        (50.0f)
#define GTM_TOM_PWM_DUTY_INIT_W        (75.0f)
#define GTM_TOM_PWM_DUTY_STEP_PERCENT  (10.0f)
#define ISR_PRIORITY_ATOM              (20)

/* LED/debug pin: compound macro for IfxPort API (port, pin) */
#define LED                            &MODULE_P13, 0  /* P13.0 */

/* ======================== Pin Routing Placeholders ======================== */
/*
 * No validated TOUT pin symbols were provided for TC3xx in the context.
 * Use NULL_PTR placeholders; replace with valid &IfxGtm_TOM1_y_TOUTz_P02_q_OUT symbols during board integration:
 *  - Phase U: P02.0 (HS), P02.7 (LS)
 *  - Phase V: P02.1 (HS), P02.4 (LS)
 *  - Phase W: P02.2 (HS), P02.5 (LS)
 */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTx_P02_0_OUT */
#define PHASE_U_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTx_P02_7_OUT */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTx_P02_1_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTx_P02_4_OUT */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTx_P02_2_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTx_P02_5_OUT */

/* ======================== Module State ======================== */
typedef struct
{
    IfxGtm_Pwm              pwm;                                   /* PWM handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM_PWM_NUM_CHANNELS];    /* persistent channels storage */
    float32                 dutyCycles[GTM_TOM_PWM_NUM_CHANNELS];  /* duty in percent */
    float32                 phases[GTM_TOM_PWM_NUM_CHANNELS];      /* phase in percent (0 for all) */
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM_PWM_NUM_CHANNELS];   /* stored dead-times */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3ph;

/* ======================== ISR and Callback (defined before init) ======================== */
/*
 * ISR: Minimal body per requirements — toggle LED/debug pin.
 * Note: The PWM driver handles interrupt routing via InterruptConfig; no SRC setup here.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Period-event callback hook: empty body per requirements.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ======================== Public API ======================== */
/**
 * Initialize a 3-phase, center-aligned PWM on the GTM TOM submodule with
 * three complementary output pairs at 20 kHz and 1 us deadtime.
 *
 * Sequence strictly follows the unified IfxGtm_Pwm initialization pattern:
 *  1) Declare local config structures and init defaults
 *  2) Fill OutputConfig[] and DtmConfig[]
 *  3) Prepare InterruptConfig with period callback
 *  4) Fill ChannelConfig[] (U, V, W) with complementary outputs and dead-time
 *  5) Complete main config (TOM submodule, cluster 1, center, syncStart/Update, FXCLK0)
 *  6) Enable-guard the GTM + CMU clocks if not already enabled
 *  7) Initialize PWM driver with persistent channels array
 *  8) Store initial state and configure LED/debug GPIO as output
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_PWM_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_PWM_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_PWM_NUM_CHANNELS];

    /* Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 2) Output configuration: 3 complementary pairs (U, V, W) */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high; /* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;  /* LS active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 3) DTM configuration: 1 us dead-time on both edges for all channels */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 4) Interrupt configuration: period callback; base channel only */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 5) Channel configuration: logical channel indices 0..2 map to U, V, W */
    /* Base channel (U) with interrupt attached */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = GTM_TOM_PWM_DUTY_INIT_U;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;

    /* Channel 1 (V) */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = GTM_TOM_PWM_DUTY_INIT_V;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Channel 2 (W) */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = GTM_TOM_PWM_DUTY_INIT_W;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 6) Main configuration: TOM submodule, cluster 1, center-aligned, sync */
    config.cluster              = IfxGtm_Cluster_1;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM_PWM_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;           /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;

    /* 7) Enable guard: enable GTM and CMU clocks only if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize PWM with persistent channels array */
    IfxGtm_Pwm_init(&g_gtmTom3ph.pwm, &g_gtmTom3ph.channels[0], &config);

    /* 9) Store initial state for duty/phase/dead-time */
    g_gtmTom3ph.dutyCycles[0] = GTM_TOM_PWM_DUTY_INIT_U;
    g_gtmTom3ph.dutyCycles[1] = GTM_TOM_PWM_DUTY_INIT_V;
    g_gtmTom3ph.dutyCycles[2] = GTM_TOM_PWM_DUTY_INIT_W;

    g_gtmTom3ph.phases[0] = 0.0f;
    g_gtmTom3ph.phases[1] = 0.0f;
    g_gtmTom3ph.phases[2] = 0.0f;

    g_gtmTom3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure LED/debug GPIO as output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three logical channel duties (U, V, W) in percent using a
 * step-and-wrap pattern and apply them immediately.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap-to-0 then add step — apply exactly this sequence for each phase */
    if ((g_gtmTom3ph.dutyCycles[0] + GTM_TOM_PWM_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3ph.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3ph.dutyCycles[1] + GTM_TOM_PWM_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3ph.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3ph.dutyCycles[2] + GTM_TOM_PWM_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3ph.dutyCycles[2] = 0.0f; }

    g_gtmTom3ph.dutyCycles[0] += GTM_TOM_PWM_DUTY_STEP_PERCENT;
    g_gtmTom3ph.dutyCycles[1] += GTM_TOM_PWM_DUTY_STEP_PERCENT;
    g_gtmTom3ph.dutyCycles[2] += GTM_TOM_PWM_DUTY_STEP_PERCENT;

    /* Immediate duty update with the state's duty array */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3ph.pwm, (float32 *)g_gtmTom3ph.dutyCycles);
}
