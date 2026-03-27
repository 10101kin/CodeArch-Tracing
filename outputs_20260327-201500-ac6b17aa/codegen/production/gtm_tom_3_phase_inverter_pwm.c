/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for a 3-phase inverter PWM using IfxGtm_Pwm on TOM1 (cluster 1),
 * center-aligned, complementary HS/LS pairs with dead-time insertion.
 *
 * Initialization follows the authoritative iLLD high-level PWM pattern.
 * No watchdog handling here (must be in CpuX_Main.c only).
 */

/* Includes: only in .c as per rules */
#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ======================== Configuration Macros ======================== */
#define NUM_OF_CHANNELS                 (3U)
#define PWM_FREQUENCY_HZ                (20000.0f)          /* 20 kHz */
#define ISR_PRIORITY_ATOM               (20)

/* Initial duties in percent */
#define PHASE_U_DUTY                    (25.0f)
#define PHASE_V_DUTY                    (50.0f)
#define PHASE_W_DUTY                    (75.0f)
#define PHASE_DUTY_STEP                 (10.0f)

/* LED/debug pin (compound macro: expands to (&MODULE_P13, 0)) */
#define LED                             &MODULE_P13, 0

/*
 * Pin routing: use reference-project verified TOM1/TOUT pins (template-validated).
 * Complementary pairs per phase (HS pin as primary, LS as complementary):
 */
#define PHASE_U_HS                      (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS                      (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS                      (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS                      (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS                      (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS                      (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ======================== Module State ======================== */
typedef struct
{
    IfxGtm_Pwm              pwm;                                 /* driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];           /* persistent channels array */
    float32                 dutyCycles[NUM_OF_CHANNELS];          /* duty in percent */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];          /* stored dead-time settings */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3phState; /* zero-initialized */

/* ======================== ISR and Callback ======================== */
/* ISR: declared with IFX_INTERRUPT as required; minimal body delegates to driver */
IFX_INTERRUPT(interruptGtmTomPwm, 0, ISR_PRIORITY_ATOM);
void interruptGtmTomPwm(void)
{
    /* Delegate GTM PWM IRQ handling to the high-level driver. */
    IfxGtm_Pwm_interruptHandler(&g_gtmTom3phState.channels[0], NULL_PTR);

    /* LED toggle intentionally omitted here: no toggle API provided in allowed calls list. */
}

/* Period-event callback: MUST be empty by design. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ======================== Public API ======================== */
/**
 * Initialize 3-phase inverter PWM (TOM1, cluster 1) using IfxGtm_Pwm.
 * - 3 complementary channels (U,V,W), center-aligned, 20 kHz, 1 us dead-time.
 * - Synchronized start and synchronized updates enabled.
 * - Period event routed via InterruptConfig on base channel (channel 0).
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all required local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Initialize main PWM config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Populate output[] for three complementary pairs (HS/LS) */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;         /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;          /* LS active LOW  */
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

    /* 4) DTM configuration: 1 us rising/falling dead-time (units: seconds) */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Base channel interrupt configuration (period event on channel 0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction; /* only this one set */
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Per-channel settings using logical indices 0..2 */
    /* Channel 0: Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;               /* percent */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;           /* base channel */

    /* Channel 1: Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;                   /* single-base-channel routing */

    /* Channel 2: Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main IfxGtm_Pwm config fields */
    config.cluster              = IfxGtm_Cluster_1;                   /* TOM cluster 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;           /* TOM sub-module */
    config.alignment            = IfxGtm_Pwm_Alignment_center;        /* center-aligned */
    config.syncStart            = TRUE;                                /* synchronous start */
    config.syncUpdateEnabled    = TRUE;                                /* synchronized updates */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    /* Clock sources: use CMU CLK0 for TOM and DTM CMU clock0 per enable guard */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 8) Enable guard for GTM and CMU setup (MANDATORY pattern) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM (persistent handle and channels array in module state) */
    IfxGtm_Pwm_init(&g_gtmTom3phState.pwm, &g_gtmTom3phState.channels[0], &config);

    /* 10) Store initial duties and dead-time settings in module state */
    g_gtmTom3phState.dutyCycles[0] = PHASE_U_DUTY;
    g_gtmTom3phState.dutyCycles[1] = PHASE_V_DUTY;
    g_gtmTom3phState.dutyCycles[2] = PHASE_W_DUTY;
    g_gtmTom3phState.deadTimes[0]  = dtmConfig[0].deadTime;
    g_gtmTom3phState.deadTimes[1]  = dtmConfig[1].deadTime;
    g_gtmTom3phState.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 11) Configure the designated ISR/debug GPIO as output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Ensure ISR symbol is not optimized out (in case toolchain doesn't auto-link ISRs) */
    (void)interruptGtmTomPwm;
}

/**
 * Update three PWM duties with +10% step and wrap rule, apply immediately.
 * The sequence per channel: if (duty+step)>=100 then duty=0; then duty+=step.
 */
void updateGtmTom3phInvDuty(void)
{
    if ((g_gtmTom3phState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[2] = 0.0f; }

    g_gtmTom3phState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately and coherently to the synchronized group */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phState.pwm, (float32 *)g_gtmTom3phState.dutyCycles);
}
