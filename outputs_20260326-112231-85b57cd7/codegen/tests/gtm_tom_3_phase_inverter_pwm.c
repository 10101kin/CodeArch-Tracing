/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm
 *
 * Notes:
 * - Follows mandatory iLLD initialization patterns for IfxGtm_Pwm.
 * - Uses TOM1 cluster with complementary HS/LS pairs and hardware deadtime.
 * - Synchronous start and synchronous update enabled.
 * - No watchdog handling in this driver (Cpu0_Main.c only).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and Configuration Constants ========================= */

/* Channel count */
#define NUM_PWM_CHANNELS                 (3)

/* PWM configuration (Hz) */
#define PWM_FREQUENCY_HZ                 (20000.0f)

/* Initial duties in percent */
#define PHASE_U_DUTY_PERCENT             (25.0f)
#define PHASE_V_DUTY_PERCENT             (50.0f)
#define PHASE_W_DUTY_PERCENT             (75.0f)

/* Duty step (percent) */
#define PHASE_DUTY_STEP_PERCENT          (10.0f)

/* ISR priority for GTM-based toggle (shared with PWM period callback priority) */
#define ISR_PRIORITY_ATOM                (20)

/* LED pin macro (compound form: port, pin) */
#define LED                              &MODULE_P13, 0

/*
 * Pin routing for TOM1 Cluster_1 complementary pairs on P02.x (U, V, W):
 * Use the _N_ convention for complementary outputs.
 */
#define PHASE_U_HS   (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_2N_TOUT5_P02_5_OUT)

/* ========================= Forward Declarations (ISR and Callback) ========================= */

/* ISR: toggles LED (hardware interrupt handler) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/* Period event callback (empty body) */
void IfxGtm_periodEventFunction(void *data);

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm           pwm;                               /* unified PWM driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_PWM_CHANNELS];        /* persistent channel storage */
    float32              dutyCycles[NUM_PWM_CHANNELS];       /* duty values in percent */
    float32              phases[NUM_PWM_CHANNELS];           /* reserved for future phase control */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_PWM_CHANNELS];       /* stored dead-time settings */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_state;

/* ========================= ISR and Callback Implementations ========================= */

/*
 * ISR: interruptGtmAtom
 * Minimal ISR that toggles a debug LED pin.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Callback: IfxGtm_periodEventFunction
 * Empty PWM period callback registered via IfxGtm_Pwm_InterruptConfig.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API Implementations ========================= */

/*
 * initGtmTom3phInv
 * Initialize a 3-phase complementary PWM on TOM1 cluster using the unified IfxGtm_Pwm driver.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all local configuration structures */
    IfxGtm_Pwm_Config          config;
    IfxGtm_Pwm_ChannelConfig   channelConfig[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_OutputConfig    output[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmConfig[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig irqCfg;

    /* 2) Load driver defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (HS/LS pairs), polarity, drive */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;             /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;              /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us on rising and falling edges */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Interrupt configuration: base-channel-only period event */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2 */
    /* Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;                     /* base channel interrupt */

    /* Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;                    /* base-channel-only pattern */

    /* Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                   /* TOM1 cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;           /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;        /* center-aligned */
    config.syncStart            = TRUE;                                /* synchronous start */
    config.numChannels          = NUM_PWM_CHANNELS;                   
    config.channels             = &channelConfig[0];                  
    config.frequency            = PWM_FREQUENCY_HZ;                   /* 20 kHz */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;   /* DTM clock */
    config.syncUpdateEnabled    = TRUE;                                /* synchronized updates */
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;                 /* FXCLK0 for TOM */

    /* 8) GTM enable and CMU setup (guarded) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM driver with persistent state */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

    /* 10) Store initial duties and dead-times in state (applied during init) */
    g_state.dutyCycles[0] = channelConfig[0].duty;
    g_state.dutyCycles[1] = channelConfig[1].duty;
    g_state.dutyCycles[2] = channelConfig[2].duty;

    g_state.phases[0] = channelConfig[0].phase;
    g_state.phases[1] = channelConfig[1].phase;
    g_state.phases[2] = channelConfig[2].phase;

    g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (initial state low is target convention; do not force here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Note: Periodic ATOM0 CH0 2 Hz interrupt setup is not explicitly configured here per unified driver rules.
       The PWM period event callback is registered via irqCfg for base channel. */
}

/*
 * updateGtmTom3phInvDuty
 * Step the percent-based duty for U, V, W by +10% with wrap rule and apply immediately.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap logic per channel: if (duty + step) >= 100 -> duty = 0, then add step */
    if ((g_state.dutyCycles[0] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_state.dutyCycles[0] = 0.0f; }
    if ((g_state.dutyCycles[1] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_state.dutyCycles[1] = 0.0f; }
    if ((g_state.dutyCycles[2] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_state.dutyCycles[2] = 0.0f; }

    g_state.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    g_state.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    g_state.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;

    /* Apply synchronized immediate update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32 *)g_state.dutyCycles);
}
