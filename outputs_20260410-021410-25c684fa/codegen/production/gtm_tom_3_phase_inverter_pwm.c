/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production-ready GTM TOM 3-Phase Inverter PWM driver (TC3xx)
 *
 * Implements a 3-channel complementary, center-aligned PWM on TOM with
 * synchronous start and synchronous shadow updates using IfxGtm_Pwm.
 *
 * NOTE:
 * - Watchdog handling must remain in CpuX_Main.c (not here).
 * - Do not install interrupts explicitly; the PWM driver routes via InterruptConfig.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ================================
 * Configuration macros (required)
 * ================================ */
#define NUM_PWM_CHANNELS        (3)
#define PWM_FREQUENCY_HZ        (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM       (20)

/* Initial duty cycles in percent */
#define PHASE_U_DUTY_INIT       (25.0f)
#define PHASE_V_DUTY_INIT       (50.0f)
#define PHASE_W_DUTY_INIT       (75.0f)
#define PHASE_DUTY_STEP         (10.0f)

/* Dead-time between complementary outputs (seconds) */
#define PWM_DEAD_TIME           (5e-07f)       /* 0.5 us per reconciled migration values */

/* Debug LED pin: P13.0 */
#define LED                     &MODULE_P13, 0

/* ================================
 * Validated pin symbols (user-specified)
 * Use ONLY these symbols from the device pinmap headers.
 * ================================ */
#define PHASE_U_HS              (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)  /* P02.0 */
#define PHASE_U_LS              (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT) /* P02.7 */
#define PHASE_V_HS              (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)  /* P02.1 */
#define PHASE_V_LS              (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT) /* P02.4 */
#define PHASE_W_HS              (&IfxGtm_ATOM0_7N_TOUT2_P02_2_OUT) /* P02.2 */
#define PHASE_W_LS              (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT) /* P02.5 */

/* ================================
 * Module state
 * ================================ */
typedef struct
{
    IfxGtm_Pwm               pwm;                                  /* PWM driver handle */
    IfxGtm_Pwm_Channel       channels[NUM_PWM_CHANNELS];           /* Persistent channels array (owned by driver) */
    float32                  dutyCycles[NUM_PWM_CHANNELS];          /* Duty cycles in percent */
    float32                  phases[NUM_PWM_CHANNELS];              /* Phase offsets in degrees (or percent) */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_PWM_CHANNELS];          /* Per-channel dead-time */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInvState;                 /* Persistent module instance */

/* ================================
 * ISR and period callback
 * ================================ */

/* ISR: routed by PWM driver's InterruptConfig. Minimal body: toggle LED. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Empty periodic callback used by InterruptConfig */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ================================
 * Public API implementations
 * ================================ */

/*
 * Initialize a 3-channel complementary, center-aligned PWM on the TOM cluster
 * with synchronous start and updates. Persistent state holds handle, channels,
 * dutyCycles, phases, and deadTimes.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structs as locals */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary pairs per logical channel */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;          /* High-side active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;           /* Low-side active LOW  */
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

    /* 4) Interrupt configuration for base channel only */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;           /* periodic notification */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;                       /* CPU0 */
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;       /* Priority 20 */
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;            /* Period callback */
    interruptConfig.dutyEvent   = NULL_PTR;                              /* Not used */

    /* 5) Channel configurations */
    /* Dead-time for all channels */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME;

    /* Logical channels 0..2 */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;                      /* Base channel IRQ */

    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;                               /* No IRQ */

    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;                               /* No IRQ */

    /* 6) Main configuration */
    config.cluster            = IfxGtm_Cluster_1;                         /* TOM1 Cluster_1 (TGC1) */
    config.subModule          = IfxGtm_Pwm_SubModule_tom;                 /* TOM */
    config.alignment          = IfxGtm_Pwm_Alignment_center;              /* Center-aligned */
    config.syncStart          = TRUE;                                     /* Synchronous start */
    config.syncUpdateEnabled  = TRUE;                                     /* Synchronous shadow transfer */
    config.numChannels        = (uint8)NUM_PWM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource.tom    = (uint32)IfxGtm_Cmu_Fxclk_0;               /* TOM uses FXCLK0 */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;         /* DTM from CMU Clock 0 */

    /* 7) Enable guard: GTM/CMU clocks setup (all inside guard) */
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

    /* 8) Initialize PWM with persistent channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInvState.pwm, g_gtmTom3phInvState.channels, &config);

    /* 9) Store initial state for later updates */
    g_gtmTom3phInvState.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3phInvState.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3phInvState.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtmTom3phInvState.phases[0] = 0.0f;
    g_gtmTom3phInvState.phases[1] = 0.0f;
    g_gtmTom3phInvState.phases[2] = 0.0f;

    g_gtmTom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure LED/debug GPIO as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update duty cycles in 10% steps with wrap rule and apply immediate
 * synchronous multi-channel update.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 -> duty = 0; then add step (always) */
    if ((g_gtmTom3phInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate synchronous update (percent units) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInvState.pwm, (float32 *)g_gtmTom3phInvState.dutyCycles);
}

/*
 * Unit-test helper. No-op implementation as behavior is test-defined.
 */
void values(void)
{
    /* Intentionally empty (used by unit tests) */
}
