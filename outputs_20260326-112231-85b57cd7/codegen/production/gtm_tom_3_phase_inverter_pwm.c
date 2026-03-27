/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for 3-phase complementary PWM on GTM TOM1 Cluster_1
 * using unified IfxGtm_Pwm high-level driver with hardware dead-time.
 *
 * Design constraints followed:
 *  - Mandatory unified IfxGtm_Pwm init pattern (config -> clocks -> init)
 *  - Complementary outputs via OutputConfig array with 1 us deadtime
 *  - Center-aligned, syncStart & syncUpdate enabled, 20 kHz
 *  - Single base-channel period IRQ callback (empty body)
 *  - Persistent state (pwm handle + channels array) using IFX_STATIC
 *  - No watchdog handling; no STM timing in this module
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ========================= Macros and configuration constants ========================= */

/* Channel count */
#define NUM_OF_CHANNELS                (3)

/* PWM timing */
#define PWM_FREQUENCY_HZ               (20000.0f)  /* 20 kHz */

/* Duty cycle initialization (percent) */
#define DUTY_INIT_U                    (25.0f)
#define DUTY_INIT_V                    (50.0f)
#define DUTY_INIT_W                    (75.0f)

/* Duty step (percent) */
#define DUTY_STEP_PERCENT              (10.0f)

/* Interrupt priority for LED toggle ISR (CPU0) */
#define ISR_PRIORITY_ATOM              (20)

/* LED pin (compound macro as required: port, pin) */
#define LED                            &MODULE_P13, 0

/*
 * Pin routing macros for complementary PWM pairs (HS/LS).
 * Note: Use validated/reference iLLD PinMap symbols. The unified driver consumes
 *       pointers to IfxGtm_Pwm_ToutMap entries via OutputConfig.
 *       The following symbols follow the TOM1/_N_ convention.
 */
#define PHASE_U_HS                     ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                     ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                     ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                     ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS                     ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS                     ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_2N_TOUT5_P02_5_OUT)

/* ========================= Module state (persistent) ========================= */

typedef struct
{
    IfxGtm_Pwm          pwm;                                 /* Driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];           /* Persistent channels array */
    float32             dutyCycles[NUM_OF_CHANNELS];          /* Percent duties */
    float32             phases[NUM_OF_CHANNELS];              /* Phase offsets (deg or percent) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];          /* Per-channel dead time */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_state;                        /* Module-level state */

/* ========================= ISR and callbacks (file scope) ========================= */

/*
 * Minimal ISR toggling a diagnostic LED. The timer source is GTM ATOM0 CH0 (configured externally).
 * Priority is ISR_PRIORITY_ATOM on CPU0 as declared below.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Empty PWM period callback as required by the unified PWM driver.
 * Must be visible (non-static) and take a void * parameter.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty */
}

/* ========================= Public API implementations ========================= */

/*
 * Initialize a 3-phase complementary PWM on TOM1 (Cluster_1) with center-aligned 20 kHz,
 * syncStart and syncUpdate enabled, and 1 us hardware dead-time.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all local configuration structures */
    IfxGtm_Pwm_Config          config;
    IfxGtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig irqCfg;

    /* 2) Load driver defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Global PWM behavior: center-aligned, 20 kHz, sync start & update, TOM1 cluster */
    config.cluster            = IfxGtm_Cluster_1;                    /* TOM1 cluster index */
    config.subModule          = IfxGtm_Pwm_SubModule_tom;
    config.alignment          = IfxGtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource.atom   = IfxGtm_Cmu_Fxclk_0;                  /* FXCLK0 */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;    /* DTM from CMU CLK0 */

    /* 4) Output configuration and hardware dead-time (1 us on rising and falling) */
    /* U-phase */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;          /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;           /* LS active low  */
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

    /* DTM (hardware dead-time) */
    dtmConfig[0].deadTime.rising = 1e-6f;  /* 1 us */
    dtmConfig[0].deadTime.falling = 1e-6f; /* 1 us */
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Base-channel interrupt configuration (period event only) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;  /* empty body callback */
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Three channel configs: logical indices Ch_0, Ch_1, Ch_2 */
    /* U-phase */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = DUTY_INIT_U;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;                /* base channel only */

    /* V-phase */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = DUTY_INIT_V;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;              /* base-channel-only pattern */

    /* W-phase */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = DUTY_INIT_W;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) GTM enable + CMU clocks inside guard */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        float32 frequency;
        IfxGtm_enable(&MODULE_GTM);
        frequency = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, frequency);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, frequency);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize the unified PWM driver with persistent state */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

    /* 9) Store initial duties and dead-times into state (init already applied to HW) */
    g_state.dutyCycles[0] = channelConfig[0].duty;
    g_state.dutyCycles[1] = channelConfig[1].duty;
    g_state.dutyCycles[2] = channelConfig[2].duty;

    g_state.phases[0] = channelConfig[0].phase;
    g_state.phases[1] = channelConfig[1].phase;
    g_state.phases[2] = channelConfig[2].phase;

    g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure LED GPIO as push-pull output (initial state as default) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 11) ATOM0 CH0 periodic interrupt at 2 Hz (no pin output) —
     *     Timer/channel configuration and SRC enable are platform/boot-level concerns
     *     and are intentionally not performed here to keep this module focused on PWM.
     *     The ISR is declared above and will toggle the LED when triggered.
     */
}

/*
 * Step PWM duties: +10% with wrap rule: if (duty+step) >= 100 -> duty = 0, then +step.
 * Apply all updates as a synchronized group using the immediate update API.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap checks per channel (explicitly, no loop) */
    if ((g_state.dutyCycles[0] + DUTY_STEP_PERCENT) >= 100.0f)
    {
        g_state.dutyCycles[0] = 0.0f;
    }
    if ((g_state.dutyCycles[1] + DUTY_STEP_PERCENT) >= 100.0f)
    {
        g_state.dutyCycles[1] = 0.0f;
    }
    if ((g_state.dutyCycles[2] + DUTY_STEP_PERCENT) >= 100.0f)
    {
        g_state.dutyCycles[2] = 0.0f;
    }

    /* Unconditional add per requirement */
    g_state.dutyCycles[0] += DUTY_STEP_PERCENT;
    g_state.dutyCycles[1] += DUTY_STEP_PERCENT;
    g_state.dutyCycles[2] += DUTY_STEP_PERCENT;

    /* Apply as a synchronized group immediately */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32*)g_state.dutyCycles);
}
