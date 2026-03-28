/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM (TC3xx)
 *
 * Implementation strictly follows iLLD high-level IfxGtm_Pwm initialization
 * pattern and the module structural rules provided.
 *
 * Watchdog disable is NOT performed in this module (must be in CpuX_Main.c only).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Constants ========================= */

/* Channel count and PWM frequency */
#define NUM_OF_CHANNELS             (3u)
#define PWM_FREQUENCY               (20000.0f)          /* 20 kHz */

/* ISR configuration */
#define ISR_PRIORITY_ATOM           (20)

/* Phase initial duties in percent */
#define PHASE_U_INIT_DUTY_PERCENT   (25.0f)
#define PHASE_V_INIT_DUTY_PERCENT   (50.0f)
#define PHASE_W_INIT_DUTY_PERCENT   (75.0f)

/* Duty update step in percent */
#define PHASE_DUTY_STEP_PERCENT     (10.0f)

/* Dead-time and minimum pulse times (seconds) */
#define PWM_DEADTIME_S              (1.0e-6f)           /* 1.0 us */
#define PWM_MIN_PULSE_S             (1.0e-6f)           /* 1.0 us (not explicitly set in IfxGtm_Pwm config) */

/* LED pin macro (compound form: port, pin) */
#define LED                         &MODULE_P13, 0

/*
 * Pin routing placeholders: No validated TOM1/TOUT pin symbols were provided.
 * Replace NULL_PTR with valid &IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols during integration.
 */
#define PHASE_U_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTx_P02_0_OUT */
#define PHASE_U_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTx_P02_7_OUT */
#define PHASE_V_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTx_P02_1_OUT */
#define PHASE_V_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTx_P02_4_OUT */
#define PHASE_W_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTx_P02_2_OUT */
#define PHASE_W_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTx_P02_5_OUT */

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                   /* PWM driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];             /* Persistent channel array */
    float32                 dutyCycles[NUM_OF_CHANNELS];            /* Duty in percent */
    float32                 phases[NUM_OF_CHANNELS];                /* Phase in degrees */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];            /* Dead-time per channel */
} GtmTom3PhPwm_State;

/* Use IFX_STATIC for module-level state (from Compilers.h) */
IFX_STATIC GtmTom3PhPwm_State g_gtmTom3PhPwmState;

/* ========================= ISR and Callback (decl/def before init) ========================= */

/* ISR declaration with priority macro (handled by driver InterruptConfig routing) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/*
 * ISR: Toggles the LED each time it's invoked.
 * Do NOT call any driver-level interrupt handler here.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Period-event callback: empty body per requirements.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API Implementations ========================= */

/*
 * Initialize 3-phase complementary PWM using the high-level IfxGtm_Pwm driver.
 */
void initGtmTomPwm(void)
{
    /* 1) Declare local configuration structures */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   interruptConfig;
    IfxGtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (pin routing and polarity) */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;   /* high-side active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;    /* low-side  active low  */
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

    /* DTM: 1.0us rising/falling dead-time for each channel */
    dtmConfig[0].deadTime.rising = PWM_DEADTIME_S;
    dtmConfig[0].deadTime.falling = PWM_DEADTIME_S;
    dtmConfig[1].deadTime.rising = PWM_DEADTIME_S;
    dtmConfig[1].deadTime.falling = PWM_DEADTIME_S;
    dtmConfig[2].deadTime.rising = PWM_DEADTIME_S;
    dtmConfig[2].deadTime.falling = PWM_DEADTIME_S;

    /* Period-event interrupt configuration: TOS CPU0, priority 20, callback pointer set */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR; /* no duty event callback */

    /* Channel configuration: logical channels 0..2 map to phases U,V,W */
    /* Channel 0 - Phase U (base channel gets the interrupt) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INIT_DUTY_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    /* Channel 1 - Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INIT_DUTY_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;  /* interrupt only on base channel */

    /* Channel 2 - Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INIT_DUTY_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 4) Main PWM config: TOM1 Cluster_1 (TGC1), center-aligned, sync start/update */
    /* Note: Cluster selection uses IfxGtm_Cluster symbol; ensure target SoC cluster matches TOM1/TGC1. */
    config.cluster              = IfxGtm_Cluster_1;                 /* TOM1 Cluster_1 (TGC1) */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;         /* Use TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;      /* Center-aligned PWM */
    config.syncStart            = TRUE;                              /* Start channels in sync */
    config.syncUpdateEnabled    = TRUE;                              /* Coherent shadow updates */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                    /* 20 kHz */
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;               /* Use FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock; /* DTM clock source */

    /* 5) GTM enable guard + CMU clock configuration (inside the guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize the PWM with persistent channels array */
    IfxGtm_Pwm_init(&g_gtmTom3PhPwmState.pwm, &g_gtmTom3PhPwmState.channels[0], &config);

    /* 7) Initialize and store initial runtime state */
    g_gtmTom3PhPwmState.dutyCycles[0] = PHASE_U_INIT_DUTY_PERCENT;
    g_gtmTom3PhPwmState.dutyCycles[1] = PHASE_V_INIT_DUTY_PERCENT;
    g_gtmTom3PhPwmState.dutyCycles[2] = PHASE_W_INIT_DUTY_PERCENT;

    g_gtmTom3PhPwmState.phases[0] = 0.0f;
    g_gtmTom3PhPwmState.phases[1] = 0.0f;
    g_gtmTom3PhPwmState.phases[2] = 0.0f;

    g_gtmTom3PhPwmState.deadTimes[0].rising  = dtmConfig[0].deadTime.rising;
    g_gtmTom3PhPwmState.deadTimes[0].falling = dtmConfig[0].deadTime.falling;
    g_gtmTom3PhPwmState.deadTimes[1].rising  = dtmConfig[1].deadTime.rising;
    g_gtmTom3PhPwmState.deadTimes[1].falling = dtmConfig[1].deadTime.falling;
    g_gtmTom3PhPwmState.deadTimes[2].rising  = dtmConfig[2].deadTime.rising;
    g_gtmTom3PhPwmState.deadTimes[2].falling = dtmConfig[2].deadTime.falling;

    /* 9) Configure LED GPIO after PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update three phase duties in percent and apply immediately.
 */
void updateGtmTomPwmDutyCycles(void)
{
    /* Wrap rule: if (duty + step) >= 100, set to 0 then add step (separate ifs, no loop) */
    if ((g_gtmTom3PhPwmState.dutyCycles[0] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3PhPwmState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3PhPwmState.dutyCycles[1] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3PhPwmState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3PhPwmState.dutyCycles[2] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3PhPwmState.dutyCycles[2] = 0.0f; }

    g_gtmTom3PhPwmState.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    g_gtmTom3PhPwmState.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    g_gtmTom3PhPwmState.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;

    /* Immediate update request (driver schedules coherently for next PWM period) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3PhPwmState.pwm, (float32 *)g_gtmTom3PhPwmState.dutyCycles);
}
