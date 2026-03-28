/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief GTM TOM 3-Phase complementary PWM using unified IfxGtm_Pwm driver (TC3xx)
 *
 * Implements a 3-phase inverter PWM on TOM1 Cluster_1 using the unified IfxGtm_Pwm driver.
 * - 20 kHz, center-aligned, 3 complementary pairs (U,V,W)
 * - Synchronous start and synchronous duty update
 * - 1 us dead-time on rising and falling edges
 * - Output mode pushPull with cmosAutomotiveSpeed1
 * - GTM enable guard (skips CMU re-init if already enabled)
 * - Period interrupt routed by high-level driver; ISR only toggles LED
 *
 * Note on pins: User-specified pins are P02.0/P02.7 (U), P02.1/P02.4 (V), P02.2/P02.5 (W).
 * Pin map symbols must be integrated from iLLD PinMap headers; placeholders are used here.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Configuration Constants ========================= */

/* Number of complementary pairs (U, V, W) */
#define NUM_OF_CHANNELS                 (3u)

/* PWM frequency in Hz */
#define PWM_FREQUENCY_HZ                (20000.0f)

/* Interrupt priority for PWM period event (CPU0) */
#define ISR_PRIORITY_ATOM               (20)

/* Initial duty cycle percentages in percent */
#define PHASE_U_DUTY_PERCENT            (25.0f)
#define PHASE_V_DUTY_PERCENT            (50.0f)
#define PHASE_W_DUTY_PERCENT            (75.0f)

/* Duty step in percent */
#define PHASE_DUTY_STEP                 (10.0f)

/* LED/debug pin: compound macro (port, pin) */
#define LED                             &MODULE_P13, 0

/*
 * Cluster selection: TOM1, Cluster_1
 * The concrete IfxGtm_Cluster enumerator depends on device headers. Use cast placeholder here.
 */
#define GTM_PWM_CLUSTER                 ((IfxGtm_Cluster)1)

/*
 * TOUT pin map placeholders (must be replaced during platform integration with valid symbols).
 * Examples (to be replaced): &IfxGtm_TOM1_0_TOUTxx_P02_0_OUT, etc.
 */
#define PHASE_U_HS                      ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* TODO: replace with &IfxGtm_TOM1_x_TOUTy_P02_0_OUT */
#define PHASE_U_LS                      ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* TODO: replace with &IfxGtm_TOM1_x_TOUTy_P02_7_OUT */
#define PHASE_V_HS                      ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* TODO: replace with &IfxGtm_TOM1_x_TOUTy_P02_1_OUT */
#define PHASE_V_LS                      ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* TODO: replace with &IfxGtm_TOM1_x_TOUTy_P02_4_OUT */
#define PHASE_W_HS                      ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* TODO: replace with &IfxGtm_TOM1_x_TOUTy_P02_2_OUT */
#define PHASE_W_LS                      ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* TODO: replace with &IfxGtm_TOM1_x_TOUTy_P02_5_OUT */

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm               pwm;                               /* Driver handle */
    IfxGtm_Pwm_Channel       channels[NUM_OF_CHANNELS];         /* Persistent channel handles */
    float32                  dutyCycles[NUM_OF_CHANNELS];       /* Duty in percent */
    float32                  phases[NUM_OF_CHANNELS];           /* Phase in degrees */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];        /* Dead-time per phase */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;

/* ========================= Internal ISR and Callback ========================= */

/* Period-event callback assigned in InterruptConfig; body must be empty. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Period ISR: declared with IFX_INTERRUPT and toggles the LED only. */
IFX_INTERRUPT(interruptGtmTomPeriod, 0, ISR_PRIORITY_ATOM);
void interruptGtmTomPeriod(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API ========================= */

/**
 * @brief Initialize the GTM-based 3-phase complementary PWM using IfxGtm_Pwm on TOM cluster.
 *
 * Behavior:
 * - Builds all configuration structures in memory first (no HW touch yet)
 * - GTM enable guard (enable + CMU clock setup if not already enabled)
 * - Initializes PWM driver with center alignment, sync start/update, FXCLK0
 * - Assigns three logical channels (0..2) with complementary pins and 1 us dead time
 * - Sets initial duties U/V/W = 25/50/75 percent into module state
 * - Configures LED/debug pin as GPIO output for ISR toggle
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Apply defaults and update main configuration */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output configuration: complementary pairs U, V, W */
    /* U phase */
    output[0].pin                     = PHASE_U_HS;            /* High-side */
    output[0].complementaryPin        = PHASE_U_LS;            /* Low-side  */
    output[0].polarity                = Ifx_ActiveState_high;
    output[0].complementaryPolarity   = Ifx_ActiveState_low;   /* mandatory complementary polarity */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase */
    output[1].pin                     = PHASE_V_HS;
    output[1].complementaryPin        = PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase */
    output[2].pin                     = PHASE_W_HS;
    output[2].complementaryPin        = PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM configuration: 1us rising and falling dead time */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* Base channel interrupt configuration: period event only on channel index 0 */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* Channel configurations: logical channel indices 0..2 map to U,V,W */
    /* CH0 → U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;                  /* base channel owns the period event */

    /* CH1 → V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;                 /* only base channel raises period event */

    /* CH2 → W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Main configuration fields */
    config.cluster              = GTM_PWM_CLUSTER;                /* TOM1, Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;       /* Use TOM sub-module */
    config.alignment            = IfxGtm_Pwm_Alignment_center;    /* center-aligned */
    config.syncStart            = TRUE;                           /* synchronous start */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;               /* 20 kHz */
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;             /* first available FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;
    config.syncUpdateEnabled    = TRUE;                           /* synchronous duty updates */

    /* 4) GTM enable guard with CMU clock configuration (no hardcoded frequencies) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 5) Initialize PWM driver (persistent handle and channels) */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 6) Store initial state for future updates */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;  /* U */
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;  /* V */
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;  /* W */

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure LED/debug pin as GPIO output for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Update U/V/W duty cycles using stepped pattern and apply atomically.
 *
 * Algorithm (percent domain):
 *  if ((duty + step) >= 100) duty = 0; then duty += step; for each U,V,W.
 * Updates are applied via IfxGtm_Pwm_updateChannelsDutyImmediate in one transaction.
 */
void updateGtmTom3phInvDuty(void)
{
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
