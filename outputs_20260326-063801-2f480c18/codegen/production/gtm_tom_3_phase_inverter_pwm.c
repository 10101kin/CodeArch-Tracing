/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: GTM TOM 3-Phase Inverter PWM (single-ended) using IfxGtm_Pwm
 * Target: TC3xx (e.g., TC387), TOM1 timebase on CH0, outputs on CH2/CH4/CH6 (P00.[3/5/7])
 *
 * Behavior:
 *  - 20 kHz center-aligned PWM, unified cluster, synchronous shadow updates.
 *  - Initial duties: U=25%, V=50%, W=75%.
 *  - Runtime update: +10% per call with wrap (0..100 inclusive policy managed by caller timing).
 *
 * Notes:
 *  - No watchdog disable here (must be in CpuX main only).
 *  - No STM timing here; scheduling done by application.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxPort.h"

/* =============================
 * Numeric configuration macros
 * ============================= */
#define DEVICE_TC38X                         (1)
#define IFX_PIN_PACKAGE_516                  (1)

/* Timing requirements */
#define PWM_FREQUENCY_HZ                     (20000.0f)   /* 20 kHz */
#define PWM_ALIGNMENT_CENTER                 (1)
#define PWM_DUTY_STEP_PERCENT                (10.0f)
#define PWM_UPDATE_INTERVAL_MS               (500U)

/* Channels / TOM configuration */
#define PWM_TOM_MODULE                       IfxGtm_Tom_1
#define PWM_TIMEBASE_CHANNEL                 IfxGtm_Tom_Ch_0
#define PWM_CHANNEL_U                        IfxGtm_Tom_Ch_2
#define PWM_CHANNEL_V                        IfxGtm_Tom_Ch_4
#define PWM_CHANNEL_W                        IfxGtm_Tom_Ch_6
#define PWM_TIMEBASE_CLOCK_SOURCE            IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0

/* Unified PWM (GTM cluster) */
#define IFXGTM_CLUSTER                       (0)          /* default cluster 0 */

/* Initial duties (percent) */
#define DUTY_INIT_U_PERCENT                  (25.0f)
#define DUTY_INIT_V_PERCENT                  (50.0f)
#define DUTY_INIT_W_PERCENT                  (75.0f)

/* Channel count */
#define PWM_NUM_CHANNELS                     (3U)

/* ISR priority for PWM period events (if used by unified driver) */
#define PWM_ISR_PRIORITY                     (10)

/* LED for minimal ISR callback activity (port, pin) */
#define LED                                  &MODULE_P13, 0

/* =============================
 * Validated TOM pin macros (from reference project)
 * ============================= */
#define PHASE_U_HS                &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_V_HS                &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_W_HS                &IfxGtm_TOM1_6_TOUT16_P00_7_OUT

/* =============================
 * Local driver state
 * ============================= */
typedef struct
{
    IfxGtm_Tom_Timer          timer;                       /* TOM timebase */
    IfxGtm_Pwm                pwm;                         /* Unified PWM driver */
    IfxGtm_Pwm_Channel        channels[PWM_NUM_CHANNELS];  /* Channel handles for unified PWM */
    float32                   dutyPercent[PWM_NUM_CHANNELS];
    boolean                   initialized;
    boolean                   error;
} GtmTomPwm_Driver;

static GtmTomPwm_Driver g_gtmTomPwm = {0};

/* =============================
 * Optional period-event callback used by unified PWM interrupt config
 * Signature required by high-level driver: void (*periodEvent)(void *data)
 * ============================= */
void gtmTomPwm_periodEvent(void *data)
{
    /* Minimal ISR workload: toggle a debug LED pin */
    (void)data;
    IfxPort_togglePin(LED);
}

/* =============================
 * Internal helpers
 * ============================= */
static inline void gtmTomPwm_storeInitialDuties(void)
{
    g_gtmTomPwm.dutyPercent[0] = DUTY_INIT_U_PERCENT;
    g_gtmTomPwm.dutyPercent[1] = DUTY_INIT_V_PERCENT;
    g_gtmTomPwm.dutyPercent[2] = DUTY_INIT_W_PERCENT;
}

/* =============================
 * Public API implementation
 * ============================= */

/*
 * initGtmTomPwm
 *
 * Steps per SW detailed design:
 * 1) Enable GTM and functional clocks for TOM (FXCLK/GCLK per iLLD pattern).
 * 2) Initialize TOM1 timebase on CH0 for 20 kHz, up/down (center-aligned).
 * 3) Initialize unified IfxGtm_Pwm for 3 single-ended outputs (U/V/W) with active-high push-pull pins.
 * 4) Start timebase and add CH2/CH4/CH6 to timer's update mask for synchronous shadow transfers.
 * 5) Stage initial duties (25/50/75%), disable update, write shadow registers, then apply a single update.
 */
void initGtmTomPwm(void)
{
    /* Debug LED pin mode (optional) */
    IfxPort_setPinMode(LED, IfxPort_OutputMode_pushPull);

    /* 1) Enable GTM and functional clocks (follow authoritative iLLD pattern) */
    IfxGtm_enable(&MODULE_GTM);
    {
        float32 modFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, modFreq);
        /* Enable all GTM clocks (including FXCLK) per pattern */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_ALL);
    }

    /* 2) Initialize TOM timebase (TOM1 CH0, FXCLK0, center-aligned target) */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
        /* Basic TOM selection */
        timerCfg.tom          = PWM_TOM_MODULE;
        timerCfg.timerChannel = PWM_TIMEBASE_CHANNEL;
        timerCfg.clock        = PWM_TIMEBASE_CLOCK_SOURCE;
        /* Base settings for frequency and center alignment */
        timerCfg.base.frequency = PWM_FREQUENCY_HZ;              /* target PWM frequency */
        timerCfg.base.isrPriority = 0;                           /* no timer ISR required here */
        timerCfg.base.isrProvider = IfxSrc_Tos_none;             /* no ISR provider */
        timerCfg.base.countDir = IfxStdIf_Timer_CountDir_upAndDown; /* center-aligned up/down */
        /* Initialize timebase */
        if (IfxGtm_Tom_Timer_init(&g_gtmTomPwm.timer, &timerCfg) == FALSE)
        {
            g_gtmTomPwm.error = TRUE;
            return;
        }
        IfxGtm_Tom_Timer_updateInputFrequency(&g_gtmTomPwm.timer);
    }

    /* 3) Initialize unified PWM driver (single-ended U/V/W) */
    {
        IfxGtm_Pwm_Config           pwmCfg;
        IfxGtm_Pwm_OutputConfig     outputCfg[PWM_NUM_CHANNELS];
        IfxGtm_Pwm_DtmConfig        dtmCfg[PWM_NUM_CHANNELS];
        IfxGtm_Pwm_ChannelConfig    chCfg[PWM_NUM_CHANNELS];
        IfxGtm_Pwm_InterruptConfig  irqCfg;

        IfxGtm_Pwm_initConfig(&pwmCfg, &MODULE_GTM);

        /* Output pin configurations: single-ended, push-pull, active HIGH */
        outputCfg[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
        outputCfg[0].complementaryPin      = NULL_PTR;
        outputCfg[0].polarity              = Ifx_ActiveState_high;
        outputCfg[0].complementaryPolarity = Ifx_ActiveState_low;
        outputCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
        outputCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outputCfg[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
        outputCfg[1].complementaryPin      = NULL_PTR;
        outputCfg[1].polarity              = Ifx_ActiveState_high;
        outputCfg[1].complementaryPolarity = Ifx_ActiveState_low;
        outputCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
        outputCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outputCfg[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
        outputCfg[2].complementaryPin      = NULL_PTR;
        outputCfg[2].polarity              = Ifx_ActiveState_high;
        outputCfg[2].complementaryPolarity = Ifx_ActiveState_low;
        outputCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
        outputCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* Dead-time configuration: single-ended (no dead-time) */
        dtmCfg[0].deadTime.rising = 0.0f;
        dtmCfg[0].deadTime.falling = 0.0f;
        dtmCfg[1].deadTime.rising = 0.0f;
        dtmCfg[1].deadTime.falling = 0.0f;
        dtmCfg[2].deadTime.rising = 0.0f;
        dtmCfg[2].deadTime.falling = 0.0f;

        /* Interrupt configuration for unified PWM period events (optional) */
        irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
        irqCfg.isrProvider = IfxSrc_Tos_cpu0;
        irqCfg.priority    = PWM_ISR_PRIORITY;
        irqCfg.periodEvent = gtmTomPwm_periodEvent;
        irqCfg.dutyEvent   = NULL_PTR;

        /* Per-channel setup */
        chCfg[0].timerCh    = (IfxGtm_Pwm_SubModule_Ch)PWM_CHANNEL_U;  /* TOM1 CH2 */
        chCfg[0].phase      = 0.0f;
        chCfg[0].duty       = DUTY_INIT_U_PERCENT;
        chCfg[0].dtm        = &dtmCfg[0];
        chCfg[0].output     = &outputCfg[0];
        chCfg[0].mscOut     = NULL_PTR;
        chCfg[0].interrupt  = &irqCfg;

        chCfg[1].timerCh    = (IfxGtm_Pwm_SubModule_Ch)PWM_CHANNEL_V;  /* TOM1 CH4 */
        chCfg[1].phase      = 0.0f;
        chCfg[1].duty       = DUTY_INIT_V_PERCENT;
        chCfg[1].dtm        = &dtmCfg[1];
        chCfg[1].output     = &outputCfg[1];
        chCfg[1].mscOut     = NULL_PTR;
        chCfg[1].interrupt  = &irqCfg;

        chCfg[2].timerCh    = (IfxGtm_Pwm_SubModule_Ch)PWM_CHANNEL_W;  /* TOM1 CH6 */
        chCfg[2].phase      = 0.0f;
        chCfg[2].duty       = DUTY_INIT_W_PERCENT;
        chCfg[2].dtm        = &dtmCfg[2];
        chCfg[2].output     = &outputCfg[2];
        chCfg[2].mscOut     = NULL_PTR;
        chCfg[2].interrupt  = &irqCfg;

        /* Unified PWM driver configuration */
        pwmCfg.cluster           = (IfxGtm_Cluster)IFXGTM_CLUSTER;
        pwmCfg.subModule         = IfxGtm_Pwm_SubModule_tom;
        pwmCfg.alignment         = IfxGtm_Pwm_Alignment_center;
        pwmCfg.syncStart         = TRUE;
        pwmCfg.numChannels       = PWM_NUM_CHANNELS;
        pwmCfg.channels          = &chCfg[0];
        pwmCfg.frequency         = PWM_FREQUENCY_HZ;
        pwmCfg.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0;
        pwmCfg.syncUpdateEnabled = TRUE;

        /* Initialize unified PWM (follows iLLD mandatory pattern) */
        IfxGtm_Pwm_init(&g_gtmTomPwm.pwm, &g_gtmTomPwm.channels[0], &pwmCfg);
    }

    /* 4) Start timebase and add PWM channels to timer update mask */
    IfxGtm_Tom_Timer_addToChannelMask(&g_gtmTomPwm.timer, PWM_CHANNEL_U);
    IfxGtm_Tom_Timer_addToChannelMask(&g_gtmTomPwm.timer, PWM_CHANNEL_V);
    IfxGtm_Tom_Timer_addToChannelMask(&g_gtmTomPwm.timer, PWM_CHANNEL_W);
    IfxGtm_Tom_Timer_run(&g_gtmTomPwm.timer);

    /* 5) Stage initial duties and apply a single synchronous shadow transfer */
    gtmTomPwm_storeInitialDuties();
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTomPwm.timer);
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTomPwm.pwm, &g_gtmTomPwm.dutyPercent[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTomPwm.timer);

    g_gtmTomPwm.initialized = TRUE;
}

/*
 * updateGtmTomPwmDutyCycles
 *
 * Runtime behavior:
 * 1) Increase each channel duty by +10%.
 * 2) If (duty + step) >= 100, wrap to 0 first, then add step (allow 0..100 policy handled by caller scheduling).
 * 3) Disable timer update, write all three duty values in one call, apply a single shadow transfer.
 */
void updateGtmTomPwmDutyCycles(void)
{
    if ((g_gtmTomPwm.initialized == FALSE) || (g_gtmTomPwm.error == TRUE))
    {
        return;
    }

    /* Update each duty per mandatory wrap rule */
    for (uint32 i = 0; i < PWM_NUM_CHANNELS; ++i)
    {
        if ((g_gtmTomPwm.dutyPercent[i] + PWM_DUTY_STEP_PERCENT) >= 100.0f)
        {
            g_gtmTomPwm.dutyPercent[i] = 0.0f;
        }
        g_gtmTomPwm.dutyPercent[i] += PWM_DUTY_STEP_PERCENT;
    }

    /* Stage synchronous update to all channels */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTomPwm.timer);
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTomPwm.pwm, &g_gtmTomPwm.dutyPercent[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTomPwm.timer);
}
