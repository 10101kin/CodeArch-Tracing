/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: 3-phase center-aligned complementary PWM on TOM (unified IfxGtm_Pwm)
 * - TC3xx family
 * - TOM1 (Cluster_1), FXCLK0, 20 kHz, dead-time 1 us (rising/falling)
 * - Complementary HS/LS pairs with sync start and sync update
 *
 * Notes:
 * - Follows iLLD initialization patterns from authoritative documentation
 * - GTM enable guard with CMU GCLK/CLK0 configuration and FXCLK domain enable
 * - Interrupt callback (period) is a no-op; ISR toggles an LED (P13.0)
 * - No watchdog handling here (must be in CpuX main per project rules)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Constants ========================= */

/* Channel count */
#define NUM_OF_CHANNELS              (3u)

/* PWM frequency (Hz) */
#define PWM_FREQUENCY_HZ             (20000.0f)

/* Interrupt priority for PWM period event (ATOM/TOM routed by driver) */
#define ISR_PRIORITY_ATOM            (20)

/* Initial duties in percent */
#define PHASE_U_DUTY                 (25.0f)
#define PHASE_V_DUTY                 (50.0f)
#define PHASE_W_DUTY                 (75.0f)

/* Duty step in percent */
#define PHASE_DUTY_STEP              (10.0f)

/* LED: P13.0 (compound macro providing port+pin) */
#define LED                          &MODULE_P13, 0

/*
 * Pin routing macros for TOM1 phase outputs.
 * IMPORTANT: No validated TOUT pin symbols were provided in the template list.
 * Per rule: do not invent pin symbols. Therefore, routing pointers are set to NULL.
 * Replace these NULL pointers with validated IfxGtm_TOMx_y_TOUTz_P.._OUT symbols
 * for your target board/package when available.
 */
#define PHASE_U_HS                   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* User requested: P02.0 */
#define PHASE_U_LS                   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* User requested: P02.7 */
#define PHASE_V_HS                   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* User requested: P02.1 */
#define PHASE_V_LS                   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* User requested: P02.4 */
#define PHASE_W_HS                   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* User requested: P02.2 */
#define PHASE_W_LS                   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* User requested: P02.5 */

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                   /* unified PWM driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];             /* persistent channel handles (driver stores pointer) */
    float32                 dutyCycles[NUM_OF_CHANNELS];            /* percent [0..100] */
    float32                 phases[NUM_OF_CHANNELS];                /* degrees or percent phase (design: initialized to 0) */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];            /* copy of configured dead-times */
} GtmTom3Ph_State;

IFX_STATIC GtmTom3Ph_State g_state;

/* ========================= ISR and Callback Declarations ========================= */

/*
 * ISR declaration (priority binding occurs via IFX_INTERRUPT; provider is set in InterruptConfig).
 * Body must be minimal as per best practices.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/* Period event callback (no-op) used by the unified PWM driver's InterruptConfig */
void IfxGtm_periodEventFunction(void *data);

/* ========================= ISR and Callback Definitions ========================= */

/**
 * interruptGtmAtom
 * Minimal ISR: toggle LED pin on PWM period event.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/**
 * IfxGtm_periodEventFunction
 * No-op period callback; invoked by driver on period events.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API Implementations ========================= */

/**
 * initGtmTomPwm
 * Initialize 3-phase center-aligned complementary PWM (TOM) using unified IfxGtm_Pwm.
 * - Config structs declared locally; persistent driver state in g_state
 * - Complementary HS/LS pairs with 1.0 us rising/falling dead-time
 * - FXCLK0 clock source, syncStart and syncUpdate enabled
 * - Period interrupt configured on base channel, ISR toggles LED P13.0
 */
void initGtmTomPwm(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load default configuration */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for 3 complementary pairs (HS/LS) */
    /* Phase U */
    output[0].pin                    = PHASE_U_HS;                   /* High-side output */
    output[0].complementaryPin       = PHASE_U_LS;                   /* Low-side output  */
    output[0].polarity               = Ifx_ActiveState_high;         /* HS active high   */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;          /* LS active low    */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 3b) Dead-time configuration: 1.0 us rising/falling for all channels */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 4) Interrupt configuration: period notify on base channel only */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify; /* period notify */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = (IfxGtm_Pwm_callBack)IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = (IfxGtm_Pwm_callBack)NULL_PTR;

    /* Channel configuration (logical indices Ch_0..Ch_2) */
    /* U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;   /* base channel with period ISR */

    /* V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;           /* no ISR on secondary channels */

    /* W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 3c) Main PWM configuration fields */
    /* config.cluster left at default from initConfig (target: TOM1/Cluster_1) */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.atom     = (uint32)IfxGtm_Cmu_Fxclk_0;    /* FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 5) GTM enable guard and CMU configuration (inside guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize the PWM driver with persistent state (g_state.channels) */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

    /* 7) Initialize module-state arrays (duties, phases, dead-times) */
    g_state.dutyCycles[0] = PHASE_U_DUTY;
    g_state.dutyCycles[1] = PHASE_V_DUTY;
    g_state.dutyCycles[2] = PHASE_W_DUTY;

    g_state.phases[0] = 0.0f;
    g_state.phases[1] = 0.0f;
    g_state.phases[2] = 0.0f;

    g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Synchronous start: handled by config.syncStart = TRUE (no explicit API call) */

    /* 9) Configure LED GPIO (P13.0) after PWM init */
    IfxPort_setPinModeOutput(&MODULE_P13, 0, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * updateGtmTomPwmDutyCycles
 * Update three phase duties (percent) by fixed step with wrap rule and apply
 * immediately using the unified driver.
 */
void updateGtmTomPwmDutyCycles(void)
{
    /* Wrap rule per design: if (duty + step) >= 100 then duty = 0; then add step */
    if ((g_state.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[0] = 0.0f; }
    if ((g_state.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[1] = 0.0f; }
    if ((g_state.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[2] = 0.0f; }

    g_state.dutyCycles[0] += PHASE_DUTY_STEP;
    g_state.dutyCycles[1] += PHASE_DUTY_STEP;
    g_state.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately to the hardware (coherent across channels) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32*)g_state.dutyCycles);
}
