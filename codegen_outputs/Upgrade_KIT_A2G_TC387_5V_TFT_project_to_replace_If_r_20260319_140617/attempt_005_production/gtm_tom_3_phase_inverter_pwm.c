/*
 * gtm_tom_3_phase_inverter_pwm.c
 * GTM TOM 3-Phase Inverter PWM - Production Source
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* ========================= Driver instance and internal state ========================= */
typedef struct {
    IfxGtm_Pwm          pwm;                               /* Unified PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];         /* Channel state from init */
    float32             dutyCycles[NUM_OF_CHANNELS];        /* Duty cycles in percent [0..100] */
    float32             phases[NUM_OF_CHANNELS];            /* Phase shift in degrees or normalized */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];        /* Dead-time (seconds) per channel */
    boolean             initialized;                        /* Driver init state */
} GtmTom3phInv_t;

static GtmTom3phInv_t g_gtmTom3phInv = {0};

/* ========================= Optional TOM ISR (period event) ========================= */
IFX_INTERRUPT(interruptGtmTom, 0, ISR_PRIORITY_TOM);
void interruptGtmTom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback (invoked by unified driver if configured) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* No-op for production; hook for diagnostics */
}

/* ========================= Local helpers ========================= */
static inline float32 toSeconds_fromMicroseconds(float32 us)
{
    return us * 1.0e-6f;
}

static inline float32 clampf32(float32 v, float32 lo, float32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* ========================= Public API implementations ========================= */

void IfxGtm_Tom_PwmHl_init(void)
{
    /* Safety: clear driver state */
    g_gtmTom3phInv.initialized = FALSE;

    /* 1) GTM enable + CMU clock setup (fxclk domain) */
    IfxGtm_enable(&MODULE_GTM);
    /* Set GCLK to GTM module frequency, route CLK0 from GCLK, enable FXCLK domain */
    {
        float32 gtmFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, gtmFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Unified PWM configuration for TOM */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptCfg;

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3a) Interrupt configuration: bind to channel 0 period event */
    interruptCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptCfg.isrProvider = IfxSrc_Tos_cpu0;
    interruptCfg.priority    = ISR_PRIORITY_TOM; /* EXACT macro name */
    interruptCfg.periodEvent = IfxGtm_periodEventFunction;
    interruptCfg.dutyEvent   = NULL_PTR;

    /* 3b) Output pin configuration (complementary pairs); routing via unified driver only */
    {
        /* Phase U */
        output[0].pin                      = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        output[0].complementaryPin         = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        output[0].polarity                 = Ifx_ActiveState_high;   /* main active high */
        output[0].complementaryPolarity    = Ifx_ActiveState_low;    /* complementary active low */
        output[0].outputMode               = IfxPort_OutputMode_pushPull;
        output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        /* Phase V */
        output[1].pin                      = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        output[1].complementaryPin         = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        output[1].polarity                 = Ifx_ActiveState_high;
        output[1].complementaryPolarity    = Ifx_ActiveState_low;
        output[1].outputMode               = IfxPort_OutputMode_pushPull;
        output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        /* Phase W */
        output[2].pin                      = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        output[2].complementaryPin         = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        output[2].polarity                 = Ifx_ActiveState_high;
        output[2].complementaryPolarity    = Ifx_ActiveState_low;
        output[2].outputMode               = IfxPort_OutputMode_pushPull;
        output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    }

    /* 3c) DTM dead-time configuration per channel */
    {
        const float32 dead_s = toSeconds_fromMicroseconds(TIMING_DEADTIME_US);
        dtmCfg[0].deadTime = dead_s;
        dtmCfg[1].deadTime = dead_s;
        dtmCfg[2].deadTime = dead_s;
    }

    /* 3d) Channel configuration: duty (percent), phase, output, dtm, interrupt (on ch0) */
    {
        /* Channel 0 (Phase U) */
        channelCfg[0].duty      = INITIAL_DUTY_PERCENT_U;  /* percent */
        channelCfg[0].phase     = 0.0f;                    /* in degrees or normalized, as supported */
        channelCfg[0].output    = &output[0];
        channelCfg[0].dtm       = &dtmCfg[0];
        channelCfg[0].interrupt = &interruptCfg;           /* period ISR on first channel */
        /* Channel 1 (Phase V) */
        channelCfg[1].duty      = INITIAL_DUTY_PERCENT_V;
        channelCfg[1].phase     = 0.0f;
        channelCfg[1].output    = &output[1];
        channelCfg[1].dtm       = &dtmCfg[1];
        channelCfg[1].interrupt = NULL_PTR;
        /* Channel 2 (Phase W) */
        channelCfg[2].duty      = INITIAL_DUTY_PERCENT_W;
        channelCfg[2].phase     = 0.0f;
        channelCfg[2].output    = &output[2];
        channelCfg[2].dtm       = &dtmCfg[2];
        channelCfg[2].interrupt = NULL_PTR;
    }

    /* 4) Top-level PWM configuration */
    config.cluster            = IfxGtm_Cluster_0;
    config.subModule          = IfxGtm_Pwm_SubModule_tom;               /* TOM submodule */
    config.alignment          = IfxGtm_Pwm_Alignment_center;            /* center-aligned */
    config.syncStart          = TRUE;                                   /* auto-start channels */
    config.syncUpdateEnabled  = TRUE;                                   /* shadow-to-active sync update */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelCfg[0];
    config.frequency          = PWM_FREQUENCY_HZ;                       /* 20 kHz */
    config.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;                     /* fxclk0 */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 5) Initialize PWM driver (channels array passed as second argument) */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, g_gtmTom3phInv.channels, &config);

    /* 6) Store initial runtime values */
    g_gtmTom3phInv.dutyCycles[0] = channelCfg[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelCfg[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelCfg[2].duty;
    g_gtmTom3phInv.deadTimes[0]  = dtmCfg[0].deadTime;
    g_gtmTom3phInv.deadTimes[1]  = dtmCfg[1].deadTime;
    g_gtmTom3phInv.deadTimes[2]  = dtmCfg[2].deadTime;

    /* Configure LED as output for ISR heartbeat */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    g_gtmTom3phInv.initialized = TRUE;
}

void IfxGtm_Tom_PwmHl_setDuty(void)
{
    if (g_gtmTom3phInv.initialized == FALSE)
    {
        return; /* Early-exit if init failed or not called yet */
    }

    /* 1) Increment duties by +10 percentage points with wrap 0..100 */
    for (uint8 i = 0; i < NUM_OF_CHANNELS; i++)
    {
        g_gtmTom3phInv.dutyCycles[i] += DUTY_UPDATE_POLICY_INCREMENT_PERCENT;
        if (g_gtmTom3phInv.dutyCycles[i] >= 100.0f && DUTY_UPDATE_POLICY_WRAP_0_TO_100)
        {
            g_gtmTom3phInv.dutyCycles[i] = 0.0f;
        }
    }

    /* 2) Compute safe duty window from minPulse and 2*deadTime */
    const float32 period_s  = 1.0f / PWM_FREQUENCY_HZ;
    const float32 minPulse  = toSeconds_fromMicroseconds(TIMING_MIN_PULSE_US);
    const float32 deadTime  = toSeconds_fromMicroseconds(TIMING_DEADTIME_US);
    const float32 safeMin   = (minPulse + (2.0f * deadTime)) / period_s;  /* fraction [0..1] */
    const float32 safeMax   = 1.0f - safeMin;
    const float32 safeMinPct = safeMin * 100.0f;
    const float32 safeMaxPct = safeMax * 100.0f;

    /* 3) Clamp to [safeMin, safeMax] in percent */
    for (uint8 i = 0; i < NUM_OF_CHANNELS; i++)
    {
        g_gtmTom3phInv.dutyCycles[i] = clampf32(g_gtmTom3phInv.dutyCycles[i], safeMinPct, safeMaxPct);
    }

    /* 4) Apply synchronous update (shadow -> active at next period) */
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, g_gtmTom3phInv.dutyCycles);
}
