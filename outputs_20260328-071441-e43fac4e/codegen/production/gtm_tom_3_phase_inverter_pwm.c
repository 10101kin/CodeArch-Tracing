/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-phase complementary inverter PWM
 * using the unified IfxGtm_Pwm high-level driver (TC3xx).
 *
 * - Center-aligned complementary PWM @ 20 kHz
 * - Dead-time: 0.5 us, Min pulse: 1.0 us (enforced via configuration where applicable)
 * - Active-high polarity (high-side), complementary active-low (low-side)
 * - Output mode push-pull, cmosAutomotiveSpeed1 pad driver
 * - Clock source: Fxclk0
 * - Shadow-transfer gating for atomic updates
 * - Initial duties: U=25%, V=50%, W=75%
 *
 * Notes:
 * - Watchdog disable belongs in Cpu0_Main.c only (none here).
 * - No STM-based timing or delays in this driver.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ============================= Macros and Constants ============================= */

/* Channel count: 3 complementary pairs = 3 channels in unified driver */
#define NUM_OF_CHANNELS             (3u)

/* Timing requirements */
#define PWM_FREQUENCY_HZ            (20000.0f)            /* 20 kHz */
#define DEAD_TIME_SEC               (0.5e-6f)             /* 0.5 us in seconds */
#define MIN_PULSE_SEC               (1.0e-6f)             /* 1.0 us in seconds (kept for design traceability) */

/* Initial duty cycles in PERCENT (0..100) */
#define PHASE_U_DUTY_INIT           (25.0f)
#define PHASE_V_DUTY_INIT           (50.0f)
#define PHASE_W_DUTY_INIT           (75.0f)

/* Duty step for the ramp updater (percent points per call) */
#define PHASE_DUTY_STEP             (0.1f)

/* ISR priority macro for GTM PWM (unified driver) */
#define ISR_PRIORITY_ATOM           (3)

/* LED debug pin macro (port, pin) */
#define LED                         &MODULE_P13, 0

/* Pin routing (validated pin symbols) */
#define PHASE_U_HS                  &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS                  &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS                  &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS                  &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS                  &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS                  &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* ============================= Module State ============================= */

typedef struct
{
    IfxGtm_Pwm                pwm;                                /* unified PWM handle */
    IfxGtm_Pwm_Channel        channels[NUM_OF_CHANNELS];          /* persistent channel array */
    float32                   dutyCycles[3];                      /* U,V,W high-side duties in percent */
    float32                   phases[3];                          /* optional phase array (not used) */
    IfxGtm_Pwm_DeadTime       deadTimes[3];                       /* stored dead-time per phase pair */
} GtmTom3PhPwm_State;

IFX_STATIC GtmTom3PhPwm_State g_gtmTom3PhPwm = {0};

/* ============================= Internal Declarations ============================= */

/* Period-event callback (assigned via InterruptConfig); body must be empty. */
void IfxGtm_periodEventFunction(void *data) { (void)data; }

/* ISR: minimal body toggles LED only. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Private helper (declared by design, not used for pin config as per unified API pattern) */
static void configureGtmTomPwmPins(void)
{
    /* Intentionally empty: Pin routing is done via IfxGtm_Pwm_OutputConfig array. */
}

/* ============================= Public API ============================= */

/**
 * Initialize the GTM PWM for a 3-phase complementary inverter (unified IfxGtm_Pwm).
 * - Declares local configuration structures and sets center-aligned, complementary outputs.
 * - Routes outputs to TOM1 channels with specified pins and pad settings.
 * - Uses FXCLK0 time base, syncStart and syncUpdate enabled for atomic updates.
 * - Enables GTM and configures CMU clocks inside enable guard.
 * - Applies initial duties and stores persistent state for runtime updates.
 */
void initGtmTomPwm(void)
{
    /* 1) Declare all configuration structures locally */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Load defaults and specialize */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output configuration: complementary pairs U, V, W */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;      /* high-side active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;       /* low-side  active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM configuration: dead-time for each complementary pair */
    dtmConfig[0].deadTime.rising = DEAD_TIME_SEC;
    dtmConfig[0].deadTime.falling = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.rising = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.falling = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.rising = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.falling = DEAD_TIME_SEC;

    /* Interrupt (base channel) configuration */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 3) Channel configurations (logical indices 0..2 for U,V,W) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;                 /* base channel gets IRQ config */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 4) Populate main configuration */
    config.gtmSFR              = &MODULE_GTM;
    config.cluster             = IfxGtm_Cluster_1;                 /* TOM1 resources */
    config.subModule           = IfxGtm_Pwm_SubModule_tom;
    config.alignment           = IfxGtm_Pwm_Alignment_center;
    config.syncStart           = TRUE;
    config.syncUpdateEnabled   = TRUE;                             /* shadow-transfer gating */
    config.numChannels         = (uint8)NUM_OF_CHANNELS;
    config.channels            = &channelConfig[0];
    config.frequency           = PWM_FREQUENCY_HZ;
    config.clockSource         = IfxGtm_Pwm_ClockSource_fxclk0;    /* FXCLK0 time base for TOM */
    config.dtmClockSource      = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock source */

    /* 5) Enable guard for GTM + CMU clocks (mandatory pattern) */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gtmTom3PhPwm.pwm, &g_gtmTom3PhPwm.channels[0], &config);

    /* 7) Store initial setpoints into module state */
    g_gtmTom3PhPwm.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3PhPwm.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3PhPwm.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtmTom3PhPwm.phases[0] = 0.0f;
    g_gtmTom3PhPwm.phases[1] = 0.0f;
    g_gtmTom3PhPwm.phases[2] = 0.0f;

    g_gtmTom3PhPwm.deadTimes[0].rising  = DEAD_TIME_SEC;
    g_gtmTom3PhPwm.deadTimes[0].falling = DEAD_TIME_SEC;
    g_gtmTom3PhPwm.deadTimes[1].rising  = DEAD_TIME_SEC;
    g_gtmTom3PhPwm.deadTimes[1].falling = DEAD_TIME_SEC;
    g_gtmTom3PhPwm.deadTimes[2].rising  = DEAD_TIME_SEC;
    g_gtmTom3PhPwm.deadTimes[2].falling = DEAD_TIME_SEC;

    /* 8) Optionally apply configured frequency immediately; syncStart enables outputs in phase */
    IfxGtm_Pwm_updateFrequencyImmediate(&g_gtmTom3PhPwm.pwm, PWM_FREQUENCY_HZ);

    /* Configure LED pin as push-pull output (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Atomically update all six synchronized PWM outputs (3 complementary pairs) using a ramp.
 * - Maintains persistent dutyCycles[3] for U,V,W (high-side duty in percent).
 * - Applies DUTY wrap rule and performs a single immediate bulk update under shadow gating.
 */
void updateGtmTomPwmDutyCycles(void)
{
    /* DUTY WRAP RULE (no loop; three separate checks) */
    if ((g_gtmTom3PhPwm.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhPwm.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3PhPwm.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhPwm.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3PhPwm.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhPwm.dutyCycles[2] = 0.0f; }

    g_gtmTom3PhPwm.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3PhPwm.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3PhPwm.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Single gated shadow transfer for all complementary pairs */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3PhPwm.pwm, (float32*)g_gtmTom3PhPwm.dutyCycles);
}
