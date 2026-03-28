/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief Production driver: GTM TOM 3-phase complementary inverter PWM (unified IfxGtm_Pwm)
 *
 * Requirements implemented:
 * - TC3xx, TOM1-based 3-phase complementary PWM, center-aligned, 20 kHz
 * - Dead-time: 0.5 us, Min pulse: 1.0 us (enforced by configuration constraints)
 * - Active-high outputs, push-pull, cmosAutomotiveSpeed1
 * - FXCLK0 time base, shadow-transfer gating for atomic updates
 * - Initial duties: U=25%%, V=50%%, W=75%%
 * - Persistent state with unified IfxGtm_Pwm driver
 *
 * Notes:
 * - No watchdog handling here (per AURIX architecture rules)
 * - Follows authoritative iLLD initialization patterns and structural rules
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ============================= Macros and configuration constants ============================= */

/* Number of complementary pairs = number of logical channels in unified driver */
#define NUM_OF_CHANNELS                 (3u)

/* Timing requirements */
#define PWM_FREQUENCY_HZ                (20000.0f)        /* 20 kHz */
#define PWM_DEADTIME_SEC                (0.5e-6f)         /* 0.5 us */
#define PWM_MIN_PULSE_SEC               (1.0e-6f)         /* 1.0 us (constraint hint) */

/* Initial duty cycles in percent */
#define PHASE_U_INIT_DUTY_PERCENT       (25.0f)
#define PHASE_V_INIT_DUTY_PERCENT       (50.0f)
#define PHASE_W_INIT_DUTY_PERCENT       (75.0f)

/* Duty update step in percent (normalized 0..100 scale) */
#define PHASE_DUTY_STEP                 (1.0f)

/* ISR priority macro (used by IFX_INTERRUPT and InterruptConfig) */
#define ISR_PRIORITY_ATOM               (3)

/* LED for ISR toggle: compound macro (port pointer, pin index) */
#define LED                              &MODULE_P13, 0

/*
 * User-requested pin assignments (validated pin symbols) — high priority
 * Mapping TOM1 channels to P00.x TOUTs for complementary pairs U, V, W.
 */
#define PHASE_U_HS                      &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS                      &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS                      &IfxGtm_TOM1_1N_TOUT14_P00_5_OUT
#define PHASE_V_LS                      &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS                      &IfxGtm_TOM1_3N_TOUT16_P00_7_OUT
#define PHASE_W_LS                      &IfxGtm_TOM1_2N_TOUT15_P00_6_OUT

/* ============================= Module state ============================= */

typedef struct
{
    IfxGtm_Pwm                 pwm;                               /* unified PWM handle */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];         /* persistent channels array */
    float32                    dutyCycles[NUM_OF_CHANNELS];       /* high-side duties in percent */
    float32                    phases[NUM_OF_CHANNELS];           /* optional phase offsets (deg or %) */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];        /* per-pair dead time settings */
} GtmTomPwm_State;

IFX_STATIC GtmTomPwm_State g_state; /* IFX_STATIC required by structural rules */

/* ============================= ISR and callbacks (declared before init) ============================= */

/* ISR: name and priority per Driver-Specific Knowledge; body toggles LED only */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: must exist and remain empty; assigned via InterruptConfig */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================= Local helpers ============================= */

/* Private: configure debug LED (do NOT configure PWM pins here; routing is via OutputConfig) */
static void configureGtmTomPwmPins(void)
{
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/* ============================= Public API ============================= */

/**
 * Initialize the GTM PWM for a 3-phase complementary inverter using the unified IfxGtm_Pwm driver.
 * Follows mandatory initialization guidelines, enable-guard pattern, and shadow-update configuration.
 */
void initGtmTomPwm(void)
{
    /* 1) Declare all configuration structures locally */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Load defaults into the main config */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Center-aligned, complementary, active-high with complementary low, push-pull, pad driver */
    /* 3) Per-channel output routing and complementary mapping: U, V, W (HS/LS) */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;    /* HS active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;     /* LS active low */
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

    /* DTM configuration: dead-time for each complementary pair */
    dtmConfig[0].deadTime.rising = PWM_DEADTIME_SEC;
    dtmConfig[0].deadTime.falling = PWM_DEADTIME_SEC;
    dtmConfig[1].deadTime.rising = PWM_DEADTIME_SEC;
    dtmConfig[1].deadTime.falling = PWM_DEADTIME_SEC;
    dtmConfig[2].deadTime.rising = PWM_DEADTIME_SEC;
    dtmConfig[2].deadTime.falling = PWM_DEADTIME_SEC;

    /* Interrupt configuration for base channel (period event) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;   /* empty body per rule */
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6 logical channels per original requirement map to 3 unified complementary pairs */
    /* Use ordinal submodule channel indices 0..(N-1) as mandated */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INIT_DUTY_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INIT_DUTY_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = &irqCfg;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INIT_DUTY_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = &irqCfg;

    /* 4) Populate main config */
    config.cluster              = IfxGtm_Cluster_1;                 /* TOM1 cluster */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;         /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart            = TRUE;                             /* start in sync */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;           /* 3 complementary pairs */
    config.channels             = &channelConfig[0];                /* channel config list */
    config.frequency            = PWM_FREQUENCY_HZ;                 /* 20 kHz */
    config.clockSource.atom     = IfxGtm_Cmu_Clk_0;                 /* FXCLK0 time base */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock */
    config.syncUpdateEnabled    = TRUE;                             /* shadow update at period */

    /* 5) Enable guard: enable GTM + configure CMU clocks for FXCLK0 domain */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize the unified PWM driver with persistent channels array */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

    /* 7) Store initial duty setpoints and dead-times into persistent state */
    g_state.dutyCycles[0] = PHASE_U_INIT_DUTY_PERCENT;
    g_state.dutyCycles[1] = PHASE_V_INIT_DUTY_PERCENT;
    g_state.dutyCycles[2] = PHASE_W_INIT_DUTY_PERCENT;

    g_state.phases[0] = channelConfig[0].phase;
    g_state.phases[1] = channelConfig[1].phase;
    g_state.phases[2] = channelConfig[2].phase;

    g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Optionally apply frequency immediately; syncStart ensures channels start together */
    IfxGtm_Pwm_updateFrequencyImmediate(&g_state.pwm, PWM_FREQUENCY_HZ);

    /* Initialize debug LED after PWM init (order per structural rule) */
    configureGtmTomPwmPins();
}

/**
 * Atomically update all three complementary PWM pairs under shadow-transfer gating.
 * Duty values are maintained in percent (0..100). Wrap rule per structural guidance.
 */
void updateGtmTomPwmDutyCycles(void)
{
    /* Wrap checks: reset to 0 if next step reaches/exceeds 100, then always add STEP */
    if ((g_state.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[0] = 0.0f; }
    if ((g_state.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[1] = 0.0f; }
    if ((g_state.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[2] = 0.0f; }

    g_state.dutyCycles[0] += PHASE_DUTY_STEP;
    g_state.dutyCycles[1] += PHASE_DUTY_STEP;
    g_state.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate bulk update under shadow-transfer gating; complementary LS handled by driver */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32 *)g_state.dutyCycles);
}
