/*
 * gtm_tom_3_phase_inverter_pwmgg.c
 *
 * Production PWM driver: GTM TOM 3-phase inverter using IfxGtm_Pwm (TC3xx)
 * Implements the SW Detailed Design behavior in a single public function:
 *  - First call: full hardware initialization + initial duty programming
 *  - Subsequent calls: increment duties by +10% with wrap and update synchronously
 */

#include "gtm_tom_3_phase_inverter_pwmgg.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* =============================
 * Driver/application context
 * ============================= */
typedef struct
{
    IfxGtm_Pwm          pwm;                               /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];         /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];       /* Duty cycles in percent */
    float32             phases[NUM_OF_CHANNELS];           /* Phase offsets (unused = 0) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];        /* Deadtime values (unused = 0) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;
static boolean s_initialized = FALSE;

/* =============================
 * Local helpers
 * ============================= */
static void prv_initIfNeeded(void)
{
    if (s_initialized == TRUE)
    {
        return;
    }

    /* 1) Enable GTM and set up CMU clocks (GCLK, CLK0, FXCLK) */
    IfxGtm_enable(&MODULE_GTM);
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        /* Enable FXCLK and CLK0 as functional clocks for TOM */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Route TOM output pins (single-ended) via PinMap API as per SW design */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_TOM, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_TOM, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_TOM, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 3) Prepare unified PWM configuration with three channels */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    chCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     outCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmCfg[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output configuration: single-ended (no complementary pin) */
    outCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_TOM;  /* U */
    outCfg[0].complementaryPin      = NULL_PTR;
    outCfg[0].polarity              = Ifx_ActiveState_high;
    outCfg[0].complementaryPolarity = Ifx_ActiveState_low; /* unused */
    outCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    outCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_TOM;  /* V */
    outCfg[1].complementaryPin      = NULL_PTR;
    outCfg[1].polarity              = Ifx_ActiveState_high;
    outCfg[1].complementaryPolarity = Ifx_ActiveState_low; /* unused */
    outCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    outCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_TOM;  /* W */
    outCfg[2].complementaryPin      = NULL_PTR;
    outCfg[2].polarity              = Ifx_ActiveState_high;
    outCfg[2].complementaryPolarity = Ifx_ActiveState_low; /* unused */
    outCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Deadtime configuration: disabled per requirements */
    dtmCfg[0].deadTime = PWM_DEADTIME_SEC;
    dtmCfg[1].deadTime = PWM_DEADTIME_SEC;
    dtmCfg[2].deadTime = PWM_DEADTIME_SEC;

    /* Channel configuration defaults + per-channel setup */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
        chCfg[i].phase     = 0.0f;                 /* No phase shift */
        chCfg[i].dtm       = &dtmCfg[i];
        chCfg[i].output    = &outCfg[i];
        chCfg[i].mscOut    = NULL_PTR;             /* Not used */
        chCfg[i].interrupt = NULL_PTR;             /* No ISR in this design */
    }

    /* Map logical channel indices to TOM1 channels 2/4/6 (single-ended) */
    chCfg[0].timerCh = IfxGtm_Pwm_SubModule_Ch_2;   /* U: TOM1 CH2 */
    chCfg[1].timerCh = IfxGtm_Pwm_SubModule_Ch_4;   /* V: TOM1 CH4 */
    chCfg[2].timerCh = IfxGtm_Pwm_SubModule_Ch_6;   /* W: TOM1 CH6 */

    /* Initial duties per requirements (percent) */
    chCfg[0].duty = DUTY_INIT_U_PERCENT;
    chCfg[1].duty = DUTY_INIT_V_PERCENT;
    chCfg[2].duty = DUTY_INIT_W_PERCENT;

    /* Top-level PWM configuration */
    config.cluster               = IfxGtm_Cluster_0;                  /* Cluster 0 (module-dependent) */
    config.subModule             = IfxGtm_Pwm_SubModule_tom;          /* TOM */
    config.alignment             = IfxGtm_Pwm_Alignment_center;       /* Center-aligned */
    config.syncStart             = TRUE;                               /* Start synced channels */
    config.syncUpdateEnabled     = TRUE;                               /* Shadow update on period match */
    config.numChannels           = NUM_OF_CHANNELS;
    config.channels              = &chCfg[0];
    config.frequency             = TIMING_PWM_FREQUENCY_HZ;
    config.clockSource.tom       = IfxGtm_Cmu_Fxclk_0;                 /* TOM uses Fxclk */
    config.dtmClockSource        = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM clock source */

    /* 4) Initialize PWM driver and start */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* Store runtime state (duty, deadtime) */
    g_gtmTom3phInv.dutyCycles[0] = chCfg[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = chCfg[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = chCfg[2].duty;
    g_gtmTom3phInv.deadTimes[0]  = dtmCfg[0].deadTime;
    g_gtmTom3phInv.deadTimes[1]  = dtmCfg[1].deadTime;
    g_gtmTom3phInv.deadTimes[2]  = dtmCfg[2].deadTime;

    /* Apply initial duties via multi-channel duty update, then start synced channels */
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
    IfxGtm_Pwm_startSyncedChannels(&g_gtmTom3phInv.pwm);

    s_initialized = TRUE;
}

/* =============================
 * Public API implementation
 * ============================= */

/**
 * See header for behavior description.
 * - First invocation performs full init + initial duty programming.
 * - Subsequent invocations increment each duty by +10 percentage points and wrap at 100% -> 0%,
 *   then update all channels synchronously via IfxGtm_Pwm_updateChannelsDuty.
 */
void updateGtmTomPwmDutyCycles(void)
{
    /* Lazy init to satisfy single API entry defined in SW Detailed Design */
    if (s_initialized == FALSE)
    {
        prv_initIfNeeded();
        return; /* Initialization + initial duty already applied */
    }

    /* Runtime behavior: increment and wrap duties */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        float32 next = g_gtmTom3phInv.dutyCycles[i] + DUTY_STEP_PERCENT;
        if (next >= DUTY_MAX_PERCENT)
        {
            next = DUTY_MIN_PERCENT;
        }
        g_gtmTom3phInv.dutyCycles[i] = next;
    }

    /* Apply updated duty array (queued; takes effect on next period due to syncUpdateEnabled) */
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
}
