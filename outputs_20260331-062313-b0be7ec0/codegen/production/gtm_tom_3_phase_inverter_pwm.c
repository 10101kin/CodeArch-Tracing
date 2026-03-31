/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-Phase Inverter using IfxGtm_Pwm unified driver on TC3xx.
 *
 * Notes:
 * - Follows iLLD IfxGtm_Pwm initialization pattern with local config structures
 * - GTM enable-guard configures CMU clocks (GCLK, CLK0, FXCLK) dynamically
 * - Single-ended outputs on TOM1 high-side pins (complementaryPin = NULL_PTR)
 * - Center-aligned PWM, 20 kHz, synchronous start and synchronous update enabled
 * - Persistent module state holds the driver handle, channels, and duty state
 * - No watchdog handling here (must be in CpuX_Main.c only)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ========================= Macros and configuration values ========================= */

/* Channel count */
#define GTM_TOM_3PH_NUM_CHANNELS        (3)

/* PWM switching frequency (Hz) */
#define GTM_TOM_PWM_FREQUENCY_HZ        (20000.0f)

/* Initial duty cycles (%) */
#define PHASE_U_INIT_DUTY               (25.0f)
#define PHASE_V_INIT_DUTY               (50.0f)
#define PHASE_W_INIT_DUTY               (75.0f)

/* Duty update step and range (%) */
#define PHASE_DUTY_STEP                 (10.0f)
#define PHASE_DUTY_MIN                  (10.0f)
#define PHASE_DUTY_MAX                  (90.0f)

/* ISR priority macro (for unified PWM driver ISR) */
#define ISR_PRIORITY_ATOM               (10)

/* Debug LED: port/pin compound macro */
#define LED                             &MODULE_P13, 0

/*
 * Pin routing placeholders (validated TOM pin symbols must be provided by integration).
 * Replace NULL_PTR with the concrete TOM1 TOUT mapping symbols for the target board:
 *   - U_HS: &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
 *   - V_HS: &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
 *   - W_HS: &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
 */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUT12_P00_3_OUT */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUT14_P00_5_OUT */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_6_TOUT16_P00_7_OUT */

/* ========================= Module state ========================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                         /* PWM driver handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM_3PH_NUM_CHANNELS];          /* Persistent channels storage */
    float32                 dutyCycles[GTM_TOM_3PH_NUM_CHANNELS];         /* Duty state in percent */
    float32                 phases[GTM_TOM_3PH_NUM_CHANNELS];             /* Phase state in degrees/percent (as configured) */
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM_3PH_NUM_CHANNELS];          /* Dead times (rising/falling) per channel */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3ph = {0};

/* ========================= ISR and callback (internal) ========================= */

/* ISR: minimal body, toggles LED for timing observation */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback (assigned via InterruptConfig); must be empty */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */

/*
 * Initialize 3-phase center-aligned PWM on TOM using IfxGtm_Pwm unified driver.
 * Configuration sequence strictly follows iLLD patterns:
 *   1) Declare local config structures
 *   2) Load defaults with IfxGtm_Pwm_initConfig
 *   3) Fill OutputConfig[] (HS pins only), DtmConfig[] (zero), and ChannelConfig[]
 *   4) Fill main Config (TOM submodule, center alignment, syncStart/syncUpdate, 20 kHz)
 *   5) GTM enable-guard: enable module and CMU clocks
 *   6) Initialize PWM with persistent handle and channels
 *   7) Store persistent duty state
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: single-ended HS pins, active-high, push-pull */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = NULL_PTR;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = NULL_PTR;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = NULL_PTR;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: zero dead-time (single-ended HS only) */
    dtmConfig[0].deadTime.rising = 0.0f;  dtmConfig[0].deadTime.falling = 0.0f;
    dtmConfig[1].deadTime.rising = 0.0f;  dtmConfig[1].deadTime.falling = 0.0f;
    dtmConfig[2].deadTime.rising = 0.0f;  dtmConfig[2].deadTime.falling = 0.0f;

    /* Interrupt configuration for base channel (minimal, period callback only) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* Channel 0: Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;  /* assign interrupt to base channel */

    /* Channel 1: Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR; /* no ISR on this channel */

    /* Channel 2: Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR; /* no ISR on this channel */

    /* 5) Main PWM configuration */
    config.cluster            = IfxGtm_Cluster_0;
    config.subModule          = IfxGtm_Pwm_SubModule_tom;
    config.alignment          = IfxGtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.numChannels        = (uint8)GTM_TOM_3PH_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = GTM_TOM_PWM_FREQUENCY_HZ;
    /* CLOCK SOURCE UNION: set TOM FXCLK only */
    config.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_systemClock;

    /* 6) GTM enable-guard: enable module and required clocks dynamically */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Initialize PWM with persistent handle and channels storage */
    IfxGtm_Pwm_init(&g_gtmTom3ph.pwm, &g_gtmTom3ph.channels[0], &config);

    /* Store persistent state from initial configuration */
    g_gtmTom3ph.dutyCycles[0] = PHASE_U_INIT_DUTY;
    g_gtmTom3ph.dutyCycles[1] = PHASE_V_INIT_DUTY;
    g_gtmTom3ph.dutyCycles[2] = PHASE_W_INIT_DUTY;

    g_gtmTom3ph.phases[0] = channelConfig[0].phase;
    g_gtmTom3ph.phases[1] = channelConfig[1].phase;
    g_gtmTom3ph.phases[2] = channelConfig[2].phase;

    g_gtmTom3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* Configure LED pin for ISR toggle (after PWM init) */
    IfxPort_setPinModeOutput(&MODULE_P13, 0, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update phase duties by +10% with wrap (>=90% -> 10%) and apply synchronously.
 * Pass the state's duty array directly to the driver's immediate multi-channel API.
 */
void updateGtmTom3phInvDuty(void)
{
    /* U */
    g_gtmTom3ph.dutyCycles[0] += PHASE_DUTY_STEP;
    if (g_gtmTom3ph.dutyCycles[0] >= PHASE_DUTY_MAX)
    {
        g_gtmTom3ph.dutyCycles[0] = PHASE_DUTY_MIN;
    }

    /* V */
    g_gtmTom3ph.dutyCycles[1] += PHASE_DUTY_STEP;
    if (g_gtmTom3ph.dutyCycles[1] >= PHASE_DUTY_MAX)
    {
        g_gtmTom3ph.dutyCycles[1] = PHASE_DUTY_MIN;
    }

    /* W */
    g_gtmTom3ph.dutyCycles[2] += PHASE_DUTY_STEP;
    if (g_gtmTom3ph.dutyCycles[2] >= PHASE_DUTY_MAX)
    {
        g_gtmTom3ph.dutyCycles[2] = PHASE_DUTY_MIN;
    }

    /* Apply all updates immediately and synchronously */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3ph.pwm, (float32 *)g_gtmTom3ph.dutyCycles);
}
