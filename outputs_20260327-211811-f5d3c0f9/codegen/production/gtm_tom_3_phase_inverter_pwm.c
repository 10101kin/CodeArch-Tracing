/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm (TC3xx)
 */
#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (configuration constants) ========================= */
#define NUM_OF_CHANNELS          (3u)
#define PWM_FREQUENCY_HZ         (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM        (20)

/* Initial duties in percent (0..100) */
#define PHASE_U_DUTY             (25.0f)
#define PHASE_V_DUTY             (50.0f)
#define PHASE_W_DUTY             (75.0f)
#define PHASE_DUTY_STEP          (10.0f)

/* LED: P13.0 */
#define LED                      &MODULE_P13, 0

/*
 * Pin routing macros (TOUT mappings)
 * NOTE: The requested pads are: U: P02.0 / P02.7, V: P02.1 / P02.4, W: P02.2 / P02.5
 * Validated TOUT symbols for these pads must be provided by the target's IfxGtm PinMap header.
 * As no validated symbols were provided in this generation context, placeholders are set to NULL.
 * Update these macros to proper IfxGtm_TOMx_y_TOUTz_P02_n_OUT symbols for your device/package.
 */
#define PHASE_U_HS               ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_U_LS               ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_V_HS               ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_V_LS               ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_W_HS               ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_W_LS               ((IfxGtm_Pwm_ToutMap*)0)

/* ========================= Module state ========================= */
typedef struct
{
    IfxGtm_Pwm                 pwm;                                 /* driver handle */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];           /* persistent channels array (driver stores pointer) */
    float32                    dutyCycles[NUM_OF_CHANNELS];         /* duty array in percent */
    float32                    phases[NUM_OF_CHANNELS];             /* phases in percent of period (if used) */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];          /* stored dead-time values */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gti;

/* ========================= ISR and period callback ========================= */
/* ATOM ISR prototype (priority configured externally) */
IFX_INTERRUPT(interruptGtmAtom0Ch0, 0, ISR_PRIORITY_ATOM);

/*
 * Empty period-event callback required by the high-level PWM driver InterruptConfig.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/*
 * GTM ATOM interrupt service routine: toggle LED pin only (minimal ISR body)
 */
void interruptGtmAtom0Ch0(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API ========================= */
/**
 * Initialize a 3-phase complementary PWM using IfxGtm_Pwm on TOM1 Cluster_1,
 * center-aligned, 20 kHz, complementary HS/LS with 1 us dead-time, sync start/update.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Populate defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output routing and polarity (complementary HS/LS per phase) */
    /* Phase U */
    output[0].pin                     = PHASE_U_HS;         /* high-side */
    output[0].complementaryPin        = PHASE_U_LS;         /* low-side  */
    output[0].polarity                = Ifx_ActiveState_high;
    output[0].complementaryPolarity   = Ifx_ActiveState_low;  /* complementary output active LOW for proper dead-time */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = PHASE_V_HS;
    output[1].complementaryPin        = PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = PHASE_W_HS;
    output[2].complementaryPin        = PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM configuration: 1.0 us rising/falling */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* Interrupt configuration: period notification, CPU0, priority ISR_PRIORITY_ATOM */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2, initial duties 25/50/75% */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* base channel: enable period ISR */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;         /* no ISR on sync channels */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;         /* no ISR on sync channels */

    /* 4) Main configuration fields */
    config.gtmSFR              = &MODULE_GTM;
    config.cluster             = IfxGtm_Cluster_1;                 /* TOM1 Cluster_1 (TGC1) */
    config.subModule           = IfxGtm_Pwm_SubModule_tom;         /* Use TOM submodule */
    config.alignment           = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart           = TRUE;                             /* synchronous start */
    config.syncUpdateEnabled   = TRUE;                             /* synchronous shadow transfer */
    config.numChannels         = NUM_OF_CHANNELS;
    config.channels            = &channelConfig[0];
    config.frequency           = PWM_FREQUENCY_HZ;
    /* Clock sources: TOM/DTM use CMU CLK0 / FXCLK, enable via CMU guard below */
    config.dtmClockSource      = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock from CMU CLK0 */

    /* 5) GTM enable/clock guard */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize the unified PWM driver with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gti.pwm, &g_gti.channels[0], &config);

    /* 7) Persist initial duties/phases/dead-times in module state */
    g_gti.dutyCycles[0] = channelConfig[0].duty;
    g_gti.dutyCycles[1] = channelConfig[1].duty;
    g_gti.dutyCycles[2] = channelConfig[2].duty;

    g_gti.phases[0] = channelConfig[0].phase;
    g_gti.phases[1] = channelConfig[1].phase;
    g_gti.phases[2] = channelConfig[2].phase;

    g_gti.deadTimes[0] = dtmConfig[0].deadTime;
    g_gti.deadTimes[1] = dtmConfig[1].deadTime;
    g_gti.deadTimes[2] = dtmConfig[2].deadTime;

    /* 9) LED GPIO init: push-pull output, initial LOW */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(LED, IfxPort_State_low);
}

/**
 * Update the three-phase PWM duty cycles with wrap rule and apply immediately.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 then duty = 0; then always add step */
    if ((g_gti.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gti.dutyCycles[0] = 0.0f; }
    if ((g_gti.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gti.dutyCycles[1] = 0.0f; }
    if ((g_gti.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gti.dutyCycles[2] = 0.0f; }

    g_gti.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gti.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gti.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply updated duties synchronously in the same cycle */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gti.pwm, (float32*)g_gti.dutyCycles);
}
