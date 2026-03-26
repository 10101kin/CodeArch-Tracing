/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-phase inverter PWM (single-ended) on TC3xx.
 * - Timebase: IfxGtm_Tom_Timer on TOM1 CH0, FXCLK0, center-aligned 20 kHz
 * - Outputs: U=P00.3 (TOM1 CH2), V=P00.5 (TOM1 CH4), W=P00.7 (TOM1 CH6)
 * - Single-ended (no dead-time), push-pull, active-high, synchronous shadow updates
 * - Duties initialized to 25%, 50%, 75%
 * - updateGtmTomPwmDutyCycles(): +10% every call, wrap 0..100 inclusive, synchronous apply
 *
 * Mandatory notes:
 * - DO NOT place watchdog disable here (CpuX_Main.c only)
 * - Use only iLLD APIs from provided signatures
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* =====================================================================
 * Configuration Macros (from requirements)
 * ===================================================================== */
#define DEVICE_TC38X                       (1)
#define IFX_PIN_PACKAGE_516                (1)

/* PWM timing/alignment */
#define PWM_FREQUENCY_HZ                   (20000.0f)     /* 20 kHz */
#define PWM_ALIGNMENT_CENTER               (1)            /* informational */

/* Timebase selection */
#define GTM_TOM_TIMEBASE_TOM               IfxGtm_Tom_1
#define GTM_TOM_TIMEBASE_CHANNEL           IfxGtm_Tom_Ch_0
#define GTM_TOM_TIMEBASE_CLOCK             IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0

/* Channel mapping (single-ended U/V/W) */
#define PHASE_U_CH                         IfxGtm_Pwm_SubModule_Ch_2
#define PHASE_V_CH                         IfxGtm_Pwm_SubModule_Ch_4
#define PHASE_W_CH                         IfxGtm_Pwm_SubModule_Ch_6

/* Validated pin symbols from reference project (TOM1 on P00.x) */
#define PHASE_U_HS                         &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_V_HS                         &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_W_HS                         &IfxGtm_TOM1_6_TOUT16_P00_7_OUT

/* Channel count */
#define PWM_CHANNELS                       (3U)

/* Initial duties (%) */
#define DUTY_U_INIT_PERCENT                (25.0f)
#define DUTY_V_INIT_PERCENT                (50.0f)
#define DUTY_W_INIT_PERCENT                (75.0f)

/* Update behavior */
#define PWM_DUTY_STEP_PERCENT              (10.0f)        /* +10% per call */
#define PWM_DUTY_MAX_PERCENT               (100.0f)
#define PWM_DUTY_MIN_PERCENT               (0.0f)

/* =====================================================================
 * Local driver state
 * ===================================================================== */

typedef struct
{
    IfxGtm_Tom_Timer          timer;                       /* TOM timebase */
    IfxGtm_Pwm                pwm;                         /* Unified PWM driver */
    IfxGtm_Pwm_Channel        channels[PWM_CHANNELS];

    /* Config mirrors (kept for clarity/maintenance) */
    IfxGtm_Pwm_OutputConfig   outCfg[PWM_CHANNELS];
    IfxGtm_Pwm_DtmConfig      dtmCfg[PWM_CHANNELS];
    IfxGtm_Pwm_ChannelConfig  chCfg[PWM_CHANNELS];

    /* Runtime duties (percent and fractional) */
    float32                   dutyPercent[PWM_CHANNELS];   /* 0..100 */
    float32                   dutyFraction[PWM_CHANNELS];  /* 0.0..1.0 */

    boolean                   initOk;
} GtmTom3PhPwm_State;

static IFX_STATIC GtmTom3PhPwm_State g_gtnPwm = {0};

/* Convenience */
static Ifx_GTM * const gtm = &MODULE_GTM;

/* =====================================================================
 * Internal helpers (local-only)
 * ===================================================================== */
static void local_convertPercentToFraction(const float32 *percent, float32 *fraction, uint32 count)
{
    uint32 i;
    for (i = 0; i < count; ++i)
    {
        fraction[i] = percent[i] * (1.0f / 100.0f);
        if (fraction[i] < 0.0f)   { fraction[i] = 0.0f; }
        if (fraction[i] > 1.0f)   { fraction[i] = 1.0f; }
    }
}

/* =====================================================================
 * Public API
 * ===================================================================== */

/*
 * initGtmTomPwm
 * 1) Enable GTM and clocks, set GCLK to module frequency
 * 2) Configure TOM1 CH0 as timebase for center-aligned 20 kHz (up/down)
 * 3) Initialize unified PWM driver (IfxGtm_Pwm) with 3 single-ended channels, active-high, push-pull
 * 4) Start timebase, add PWM channels (2/4/6) to update mask for synchronous shadow transfers
 * 5) Stage initial duties (25/50/75%), disable update, write shadow, apply single update
 */
void initGtmTomPwm(void)
{
    /* Step 1: Enable GTM module and functional clocks (FXCLK/CLK0 via ALL to be safe) */
    IfxGtm_enable(gtm);

    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(gtm);
        IfxGtm_Cmu_setGclkFrequency(gtm, moduleFreq);
        /* Enable all clocks to ensure TOM timebase and channels are clocked */
        IfxGtm_Cmu_enableClocks(gtm, (uint32)IFXGTM_CMU_CLKEN_ALL);
    }

    /* Step 2: Initialize TOM1 CH0 as timebase */
    {
        IfxGtm_Tom_Timer_Config tcfg;
        IfxGtm_Tom_Timer_initConfig(&tcfg, gtm);

        /* Basic timebase selection */
        tcfg.tom           = GTM_TOM_TIMEBASE_TOM;
        tcfg.timerChannel  = GTM_TOM_TIMEBASE_CHANNEL;
        tcfg.clock         = GTM_TOM_TIMEBASE_CLOCK;     /* FXCLK0 */
        tcfg.triggerOut    = NULL_PTR;                   /* no external trigger */

        /* Initialize timer */
        if (IfxGtm_Tom_Timer_init(&g_gtnPwm.timer, &tcfg) == FALSE)
        {
            g_gtnPwm.initOk = FALSE;
            return;
        }

        /* Update input frequency info so resolution/period calculations are coherent */
        IfxGtm_Tom_Timer_updateInputFrequency(&g_gtnPwm.timer);

        /* Compute center-aligned period ticks = f(FXCLK0) / (2 * Fpwm) */
        {
            float32 fxclk0 = IfxGtm_Cmu_getFxClkFrequency(gtm, IfxGtm_Cmu_Fxclk_0);
            float32 periodTicksF = (fxclk0 / (2.0f * PWM_FREQUENCY_HZ));

            /* Stage timebase period */
            IfxGtm_Tom_Timer_disableUpdate(&g_gtnPwm.timer);
            IfxStdIf_Timer_setPeriod(&g_gtnPwm.timer, (Ifx_TimerValue)periodTicksF);
            IfxGtm_Tom_Timer_applyUpdate(&g_gtnPwm.timer);
        }
    }

    /* Step 3: Initialize unified PWM driver (IfxGtm_Pwm) with 3 channels, center-aligned */
    {
        IfxGtm_Pwm_Config pcfg;
        IfxGtm_Pwm_initConfig(&pcfg, gtm);

        /* Output configuration (pins, active-high, push-pull) */
        g_gtnPwm.outCfg[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
        g_gtnPwm.outCfg[0].complementaryPin      = NULL_PTR;
        g_gtnPwm.outCfg[0].polarity              = Ifx_ActiveState_high;
        g_gtnPwm.outCfg[0].complementaryPolarity = Ifx_ActiveState_low;
        g_gtnPwm.outCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
        g_gtnPwm.outCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        g_gtnPwm.outCfg[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
        g_gtnPwm.outCfg[1].complementaryPin      = NULL_PTR;
        g_gtnPwm.outCfg[1].polarity              = Ifx_ActiveState_high;
        g_gtnPwm.outCfg[1].complementaryPolarity = Ifx_ActiveState_low;
        g_gtnPwm.outCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
        g_gtnPwm.outCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        g_gtnPwm.outCfg[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
        g_gtnPwm.outCfg[2].complementaryPin      = NULL_PTR;
        g_gtnPwm.outCfg[2].polarity              = Ifx_ActiveState_high;
        g_gtnPwm.outCfg[2].complementaryPolarity = Ifx_ActiveState_low;
        g_gtnPwm.outCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
        g_gtnPwm.outCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* DTM configuration (no dead-time for single-ended) */
        g_gtnPwm.dtmCfg[0].deadTime.rising = 0.0f; g_gtnPwm.dtmCfg[0].deadTime.falling = 0.0f;
        g_gtnPwm.dtmCfg[1].deadTime.rising = 0.0f; g_gtnPwm.dtmCfg[1].deadTime.falling = 0.0f;
        g_gtnPwm.dtmCfg[2].deadTime.rising = 0.0f; g_gtnPwm.dtmCfg[2].deadTime.falling = 0.0f;

        /* Channel configuration */
        g_gtnPwm.chCfg[0].timerCh    = PHASE_U_CH;
        g_gtnPwm.chCfg[0].phase      = 0.0f;
        g_gtnPwm.chCfg[0].duty       = DUTY_U_INIT_PERCENT;
        g_gtnPwm.chCfg[0].dtm        = &g_gtnPwm.dtmCfg[0];
        g_gtnPwm.chCfg[0].output     = &g_gtnPwm.outCfg[0];
        g_gtnPwm.chCfg[0].mscOut     = NULL_PTR;
        g_gtnPwm.chCfg[0].interrupt  = NULL_PTR; /* period IRQ not used here */

        g_gtnPwm.chCfg[1].timerCh    = PHASE_V_CH;
        g_gtnPwm.chCfg[1].phase      = 0.0f;
        g_gtnPwm.chCfg[1].duty       = DUTY_V_INIT_PERCENT;
        g_gtnPwm.chCfg[1].dtm        = &g_gtnPwm.dtmCfg[1];
        g_gtnPwm.chCfg[1].output     = &g_gtnPwm.outCfg[1];
        g_gtnPwm.chCfg[1].mscOut     = NULL_PTR;
        g_gtnPwm.chCfg[1].interrupt  = NULL_PTR;

        g_gtnPwm.chCfg[2].timerCh    = PHASE_W_CH;
        g_gtnPwm.chCfg[2].phase      = 0.0f;
        g_gtnPwm.chCfg[2].duty       = DUTY_W_INIT_PERCENT;
        g_gtnPwm.chCfg[2].dtm        = &g_gtnPwm.dtmCfg[2];
        g_gtnPwm.chCfg[2].output     = &g_gtnPwm.outCfg[2];
        g_gtnPwm.chCfg[2].mscOut     = NULL_PTR;
        g_gtnPwm.chCfg[2].interrupt  = NULL_PTR;

        /* Top-level PWM configuration */
        pcfg.subModule           = IfxGtm_Pwm_SubModule_tom;
        pcfg.alignment           = IfxGtm_Pwm_Alignment_center;
        pcfg.syncStart           = TRUE;
        pcfg.numChannels         = PWM_CHANNELS;
        pcfg.channels            = &g_gtnPwm.chCfg[0];
        pcfg.frequency           = PWM_FREQUENCY_HZ;
        pcfg.dtmClockSource      = IfxGtm_Dtm_ClockSource_cmuClock0;
        pcfg.syncUpdateEnabled   = TRUE;

        /* Initialize the unified PWM driver */
        IfxGtm_Pwm_init(&g_gtnPwm.pwm, &g_gtnPwm.channels[0], &pcfg);
    }

    /* Step 4: Start timebase and add PWM channels to update mask for atomic/shadow updates */
    IfxGtm_Tom_Timer_addToChannelMask(&g_gtnPwm.timer, IfxGtm_Tom_Ch_2);
    IfxGtm_Tom_Timer_addToChannelMask(&g_gtnPwm.timer, IfxGtm_Tom_Ch_4);
    IfxGtm_Tom_Timer_addToChannelMask(&g_gtnPwm.timer, IfxGtm_Tom_Ch_6);
    IfxGtm_Tom_Timer_run(&g_gtnPwm.timer);

    /* Step 5: Stage initial duties and apply synchronously */
    g_gtnPwm.dutyPercent[0] = DUTY_U_INIT_PERCENT;
    g_gtnPwm.dutyPercent[1] = DUTY_V_INIT_PERCENT;
    g_gtnPwm.dutyPercent[2] = DUTY_W_INIT_PERCENT;
    local_convertPercentToFraction(g_gtnPwm.dutyPercent, g_gtnPwm.dutyFraction, PWM_CHANNELS);

    IfxGtm_Tom_Timer_disableUpdate(&g_gtnPwm.timer);
    IfxGtm_Pwm_updateChannelsDuty(&g_gtnPwm.pwm, &g_gtnPwm.dutyFraction[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtnPwm.timer);

    g_gtnPwm.initOk = TRUE;
}

/*
 * updateGtmTomPwmDutyCycles
 * - On each call: +10% per channel; if (duty + step) > 100 then wrap to 0; allow true 0% and 100%
 * - Convert to fractional; stage via disableUpdate + multi-channel duty write; apply one shadow transfer
 */
void updateGtmTomPwmDutyCycles(void)
{
    if (!g_gtnPwm.initOk)
    {
        return; /* Not initialized */
    }

    /* Update duties with wrap 0..100 inclusive, preserving true 100% until next call wraps to 0% */
    for (uint32 i = 0; i < PWM_CHANNELS; ++i)
    {
        float32 d = g_gtnPwm.dutyPercent[i];
        if ((d + PWM_DUTY_STEP_PERCENT) > PWM_DUTY_MAX_PERCENT)
        {
            d = PWM_DUTY_MIN_PERCENT;  /* wrap to 0% */
        }
        else
        {
            d += PWM_DUTY_STEP_PERCENT;
        }
        g_gtnPwm.dutyPercent[i] = d;
    }

    local_convertPercentToFraction(g_gtnPwm.dutyPercent, g_gtnPwm.dutyFraction, PWM_CHANNELS);

    /* Stage and apply synchronously */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtnPwm.timer);
    IfxGtm_Pwm_updateChannelsDuty(&g_gtnPwm.pwm, &g_gtnPwm.dutyFraction[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtnPwm.timer);
}
