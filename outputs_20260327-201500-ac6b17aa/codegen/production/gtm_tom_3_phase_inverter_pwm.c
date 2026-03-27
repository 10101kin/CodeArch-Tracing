/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: 3-phase inverter PWM on GTM TOM using IfxGtm_Pwm high-level driver.
 * Migration target: IfxGtm_Pwm (unified) from legacy IfxGtm_Tom_PwmHl.
 *
 * Notes:
 * - Follows authoritative iLLD initialization patterns and mandatory guidelines.
 * - No watchdog handling here (Cpu0_Main.c only).
 * - No STM timing logic here.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Configuration Constants ========================= */

/* Channel count */
#define NUM_OF_CHANNELS                  (3u)

/* PWM frequency: 20 kHz */
#define PWM_FREQUENCY_HZ                 (20000.0f)

/* Interrupt priority for PWM period event (ATOM/TOM group IRQ routing) */
#define ISR_PRIORITY_ATOM                (20u)

/* Initial duty cycles in percent */
#define PHASE_U_DUTY_INIT                (25.0f)
#define PHASE_V_DUTY_INIT                (50.0f)
#define PHASE_W_DUTY_INIT                (75.0f)

/* Duty step for updates (percent) */
#define PHASE_DUTY_STEP                  (10.0f)

/* Debug LED pin (port, pin) compound macro */
#define LED                              &MODULE_P13, 0

/*
 * Pin routing: Use validated/reference pin symbols (TOUT on P00.x) for TOM1 cluster pairs.
 * Complementary pairs: HS (pin) / LS (complementaryPin)
 * Note: Symbols taken from reference mapping examples.
 */
#define PHASE_U_HS   ((IfxGtm_Pwm_ToutMap *)&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   ((IfxGtm_Pwm_ToutMap *)&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   ((IfxGtm_Pwm_ToutMap *)&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   ((IfxGtm_Pwm_ToutMap *)&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   ((IfxGtm_Pwm_ToutMap *)&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   ((IfxGtm_Pwm_ToutMap *)&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                              /* High-level PWM driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];        /* Persistent channels array */
    float32                 dutyCycles[NUM_OF_CHANNELS];       /* Duty cycles in percent */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];        /* Dead-time settings per channel */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtiState;

/* ========================= ISR and Callback Declarations ========================= */

/* ISR: Period event interrupt wrapper for GTM PWM (minimal body). */
IFX_INTERRUPT(interruptGtmTomPwm, 0, ISR_PRIORITY_ATOM);

/* Period-event callback for IfxGtm_Pwm (empty by design). */
void IfxGtm_periodEventFunction(void *data);

/* ========================= ISR and Callback Definitions ========================= */

void interruptGtmTomPwm(void)
{
    /* Service the PWM interrupt using the high-level driver's handler.
       Base channel is channels[0] (single-base-channel routing). */
    IfxGtm_Pwm_interruptHandler(&g_gtiState.channels[0], NULL_PTR);
}

void IfxGtm_periodEventFunction(void *data)
{
    /* Empty callback by design. */
    (void)data;
}

/* ========================= Public API Implementations ========================= */

/*
 * Initialize 3-phase inverter PWM on TOM1 (Cluster 1): center-aligned, 3 complementary pairs with 1us DT.
 *
 * Algorithm (per SW detailed design):
 *  1) Declare local config structures.
 *  2) initConfig defaults.
 *  3) Populate OutputConfig for U/V/W HS/LS pairs.
 *  4) Configure per-channel (Ch_0..Ch_2), center aligned, 20kHz, DT=1us, sync start/update, initial duties.
 *  5) Period-event interrupt: provider CPU0, priority 20, callback IfxGtm_periodEventFunction; only channel 0 has interrupt.
 *  6) Enable guard for GTM/CMU clocks.
 *  7) Call high-level IfxGtm_Pwm_init with persistent handle and channels array.
 *  8) Store initial duties and dead-times into module state.
 *  9) Configure debug GPIO for ISR (P13.0) as push-pull output.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Populate OutputConfig for three complementary pairs (U, V, W) */
    /* U-phase */
    output[0].pin                    = PHASE_U_HS;  /* High-side */
    output[0].complementaryPin       = PHASE_U_LS;  /* Low-side  */
    output[0].polarity               = Ifx_ActiveState_high; /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;  /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V-phase */
    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W-phase */
    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1us on rising and falling edges */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Interrupt configuration: period-event routed to CPU0 with priority 20 */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel configurations: logical indices 0..2 (SubModule_Ch_0..2) */
    /* U-phase on Ch_0 */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* base channel interrupt only */

    /* V-phase on Ch_1 */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;          /* single-base-channel routing */

    /* W-phase on Ch_2 */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                 /* TOM1 cluster (Cluster 1) */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;         /* Use TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;      /* Center-aligned PWM */
    config.syncStart            = TRUE;                              /* Synchronous start */
    config.syncUpdateEnabled    = TRUE;                              /* Shadow transfer at period */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                  /* 20 kHz */
    /* Clock sources: use CMU CLK0 for submodule and DTM CMU clock0 for dead-time */
    /* Some iLLD variants use a clockSource field; defaults are used if not explicitly set. */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 6) Enable guard: enable GTM and configure CMU clocks only if disabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Initialize the PWM driver with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gtiState.pwm, &g_gtiState.channels[0], &config);

    /* 8) Store initial state for later updates */
    g_gtiState.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtiState.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtiState.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtiState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtiState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtiState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 9) Configure ISR/debug GPIO (P13.0) as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update the three PWM duties by +10% with wrap rule, apply immediately.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap then step: three explicit checks as required */
    if ((g_gtiState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtiState.dutyCycles[0] = 0.0f; }
    if ((g_gtiState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtiState.dutyCycles[1] = 0.0f; }
    if ((g_gtiState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtiState.dutyCycles[2] = 0.0f; }

    g_gtiState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtiState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtiState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply updated duties coherently to the synchronized group */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtiState.pwm, (float32 *)g_gtiState.dutyCycles);
}
