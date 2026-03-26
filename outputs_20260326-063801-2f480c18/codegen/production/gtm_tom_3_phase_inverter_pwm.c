/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for TC3xx GTM TOM 3-phase inverter PWM using IfxGtm_Pwm
 * Single-ended outputs on TOM1 CH2/CH4/CH6, center-aligned @ 20 kHz.
 *
 * Initialization and updates strictly follow iLLD patterns and the provided
 * behavior specifications. No watchdog handling is performed here.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"

/* =====================================================================================
 * Configuration constants (from requirements)
 * ===================================================================================== */
#define DEVICE_TC38X                               (1)
#define IFX_PIN_PACKAGE_516                        (1)

/* Timing requirements */
#define GTM_TOM_PWM_FREQUENCY_HZ                   (20000.0f)   /* 20 kHz */
#define PWM_DUTY_UPDATE_STEP_PERCENT               (10.0f)
#define PWM_DUTY_UPDATE_INTERVAL_MS                (500U)       /* For scheduling in Cpu0_Main.c only */
#define PWM_DUTY_MIN_PERCENT                       (0.0f)
#define PWM_DUTY_MAX_PERCENT                       (100.0f)

/* Initial duties (percent, allow true 0 and 100) */
#define PWM_INIT_DUTY_U_PERCENT                    (25.0f)
#define PWM_INIT_DUTY_V_PERCENT                    (50.0f)
#define PWM_INIT_DUTY_W_PERCENT                    (75.0f)

/* TOM timebase selection */
#define GTM_TOM_TIMEBASE_TOM                       IfxGtm_Tom_1
#define GTM_TOM_TIMEBASE_CHANNEL                   IfxGtm_Tom_Ch_0
#define GTM_TOM_TIMEBASE_CLOCK                     IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0

/* Unified PWM: TOM submodule, channels U/V/W on TOM1 CH2/CH4/CH6 */
#define PWM_NUM_CHANNELS                           (3U)
#define PHASE_U_PWM_SUBMODULE_CH                   IfxGtm_Pwm_SubModule_Ch_2
#define PHASE_V_PWM_SUBMODULE_CH                   IfxGtm_Pwm_SubModule_Ch_4
#define PHASE_W_PWM_SUBMODULE_CH                   IfxGtm_Pwm_SubModule_Ch_6

/* For timebase channel mask (TOM channel enums) */
#define PHASE_U_TOM_CH                             IfxGtm_Tom_Ch_2
#define PHASE_V_TOM_CH                             IfxGtm_Tom_Ch_4
#define PHASE_W_TOM_CH                             IfxGtm_Tom_Ch_6

/* Dead-time policy: not used for single-ended; set to 0.0s */
#define PWM_DEADTIME_SECONDS                       (0.0f)

/* =====================================================================================
 * Pin routing (validated/reference TOM1 pins on P00.x)
 * ===================================================================================== */
#define PHASE_U_HS                                 &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_V_HS                                 &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_W_HS                                 &IfxGtm_TOM1_6_TOUT16_P00_7_OUT

/* =====================================================================================
 * Internal driver state
 * ===================================================================================== */
typedef struct
{
    IfxGtm_Tom_Timer     timer;                          /* TOM timebase driver */
    IfxGtm_Pwm           pwm;                            /* Unified PWM driver */
    IfxGtm_Pwm_Channel   chHandles[PWM_NUM_CHANNELS];    /* Channel handles for unified driver */
    float32              dutyPercent[PWM_NUM_CHANNELS];  /* Current duties in percent [0..100] */
} GtmTom3PhPwm_State;

static GtmTom3PhPwm_State s_gPwm = {0};

/* =====================================================================================
 * Local helpers
 * ===================================================================================== */
static void setInitialDuties(void)
{
    s_gPwm.dutyPercent[0] = PWM_INIT_DUTY_U_PERCENT;
    s_gPwm.dutyPercent[1] = PWM_INIT_DUTY_V_PERCENT;
    s_gPwm.dutyPercent[2] = PWM_INIT_DUTY_W_PERCENT;
}

/* =====================================================================================
 * Public API implementations
 * ===================================================================================== */
/** \brief Initialize GTM clocks, TOM timebase, and IfxGtm_Pwm for 3 single-ended channels.
 *
 * Behavior:
 * 1) Enable GTM and TOM functional clocks (FXCLK), set GCLK to module frequency.
 * 2) Initialize TOM1 timebase on CH0 using FXCLK0 at 20 kHz (center-aligned operation coordinated by PWM driver).
 * 3) Initialize unified PWM with 3 channels (U/V/W on CH2/4/6), active-high, push-pull, linked to TOM timebase.
 * 4) Start the timebase and add the three channels to the TOM timer's update mask.
 * 5) Stage initial duties (25/50/75%), disable updates, write to PWM shadow, apply a single shadow transfer.
 */
void initGtmTomPwm(void)
{
    /* 1) Enable GTM and functional clocks */
    IfxGtm_enable(&MODULE_GTM);

    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        /* Enable TOM FXCLK (ATOM CLK0 not required) */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Initialize TOM timebase */
    {
        IfxGtm_Tom_Timer_Config tmrCfg;
        IfxGtm_Tom_Timer_initConfig(&tmrCfg, &MODULE_GTM);

        tmrCfg.tom           = GTM_TOM_TIMEBASE_TOM;
        tmrCfg.timerChannel  = GTM_TOM_TIMEBASE_CHANNEL;
        tmrCfg.clock         = GTM_TOM_TIMEBASE_CLOCK;
        /* Set desired frequency; center-aligned operation is coordinated with the PWM driver */
        tmrCfg.base.frequency = GTM_TOM_PWM_FREQUENCY_HZ;
        /* Counting mode for center alignment is managed via PWM alignment; driver will configure as needed */

        if (FALSE == IfxGtm_Tom_Timer_init(&s_gPwm.timer, &tmrCfg))
        {
            /* Initialization failed: leave in safe state */
            return;
        }

        /* Update input frequency after clocks are set */
        IfxGtm_Tom_Timer_updateInputFrequency(&s_gPwm.timer);
    }

    /* 3) Initialize unified PWM (single-ended) */
    {
        IfxGtm_Pwm_Config           cfg;
        IfxGtm_Pwm_ChannelConfig    chCfg[PWM_NUM_CHANNELS];
        IfxGtm_Pwm_OutputConfig     outCfg[PWM_NUM_CHANNELS];
        IfxGtm_Pwm_DtmConfig        dtmCfg[PWM_NUM_CHANNELS];

        IfxGtm_Pwm_initConfig(&cfg, &MODULE_GTM);

        /* Output configuration: pin routing, polarity, and pad */
        outCfg[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS; /* U: TOM1 CH2 -> P00.3 */
        outCfg[0].complementaryPin      = NULL_PTR;                        /* low side unused */
        outCfg[0].polarity              = Ifx_ActiveState_high;
        outCfg[0].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS; /* V: TOM1 CH4 -> P00.5 */
        outCfg[1].complementaryPin      = NULL_PTR;
        outCfg[1].polarity              = Ifx_ActiveState_high;
        outCfg[1].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS; /* W: TOM1 CH6 -> P00.7 */
        outCfg[2].complementaryPin      = NULL_PTR;
        outCfg[2].polarity              = Ifx_ActiveState_high;
        outCfg[2].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* DTM (dead-time) configuration: not used for single-ended (set to 0) */
        dtmCfg[0].deadTime.rising = PWM_DEADTIME_SECONDS;
        dtmCfg[0].deadTime.falling = PWM_DEADTIME_SECONDS;
        dtmCfg[1].deadTime.rising = PWM_DEADTIME_SECONDS;
        dtmCfg[1].deadTime.falling = PWM_DEADTIME_SECONDS;
        dtmCfg[2].deadTime.rising = PWM_DEADTIME_SECONDS;
        dtmCfg[2].deadTime.falling = PWM_DEADTIME_SECONDS;

        /* Channel configurations */
        chCfg[0].timerCh    = PHASE_U_PWM_SUBMODULE_CH;  /* CH2 */
        chCfg[0].phase      = 0.0f;
        chCfg[0].duty       = PWM_INIT_DUTY_U_PERCENT;   /* percent */
        chCfg[0].dtm        = &dtmCfg[0];
        chCfg[0].output     = &outCfg[0];
        chCfg[0].mscOut     = NULL_PTR;
        chCfg[0].interrupt  = NULL_PTR;                 /* No PWM ISR required by spec */

        chCfg[1].timerCh    = PHASE_V_PWM_SUBMODULE_CH;  /* CH4 */
        chCfg[1].phase      = 0.0f;
        chCfg[1].duty       = PWM_INIT_DUTY_V_PERCENT;
        chCfg[1].dtm        = &dtmCfg[1];
        chCfg[1].output     = &outCfg[1];
        chCfg[1].mscOut     = NULL_PTR;
        chCfg[1].interrupt  = NULL_PTR;

        chCfg[2].timerCh    = PHASE_W_PWM_SUBMODULE_CH;  /* CH6 */
        chCfg[2].phase      = 0.0f;
        chCfg[2].duty       = PWM_INIT_DUTY_W_PERCENT;
        chCfg[2].dtm        = &dtmCfg[2];
        chCfg[2].output     = &outCfg[2];
        chCfg[2].mscOut     = NULL_PTR;
        chCfg[2].interrupt  = NULL_PTR;

        /* Unified PWM configuration */
        cfg.cluster           = (IfxGtm_Cluster)0;                 /* Default cluster */
        cfg.subModule         = IfxGtm_Pwm_SubModule_tom;
        cfg.alignment         = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
        cfg.syncStart         = TRUE;                              /* sync start */
        cfg.numChannels       = PWM_NUM_CHANNELS;
        cfg.channels          = &chCfg[0];
        cfg.frequency         = GTM_TOM_PWM_FREQUENCY_HZ;
        cfg.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock (not used here) */
        cfg.syncUpdateEnabled = TRUE;                              /* enable sync update */

        /* Initialize PWM driver and channel handles */
        IfxGtm_Pwm_init(&s_gPwm.pwm, &s_gPwm.chHandles[0], &cfg);
    }

    /* 4) Start timebase and add PWM channels to update mask */
    IfxGtm_Tom_Timer_run(&s_gPwm.timer);
    IfxGtm_Tom_Timer_addToChannelMask(&s_gPwm.timer, PHASE_U_TOM_CH);
    IfxGtm_Tom_Timer_addToChannelMask(&s_gPwm.timer, PHASE_V_TOM_CH);
    IfxGtm_Tom_Timer_addToChannelMask(&s_gPwm.timer, PHASE_W_TOM_CH);

    /* 5) Stage initial duties and apply a single synchronous shadow transfer */
    setInitialDuties();
    IfxGtm_Tom_Timer_disableUpdate(&s_gPwm.timer);
    IfxGtm_Pwm_updateChannelsDuty(&s_gPwm.pwm, &s_gPwm.dutyPercent[0]);
    IfxGtm_Tom_Timer_applyUpdate(&s_gPwm.timer);
}

/** \brief Increment duties (+10%) with 0..100% wrap, then apply one synchronous update.
 *
 * Behavior (called by application every 500 ms):
 * 1) Increase each duty by +10%.
 * 2) If the new duty exceeds 100%, wrap to 0% (true 0%/100% supported).
 * 3) Write the updated percentages to the PWM driver using a synchronous shadow transfer.
 */
void updateGtmTomPwmDutyCycles(void)
{
    /* Update duties in percent with 0..100 wrap as specified */
    for (uint32 i = 0; i < PWM_NUM_CHANNELS; ++i)
    {
        float32 next = s_gPwm.dutyPercent[i] + PWM_DUTY_UPDATE_STEP_PERCENT;
        if (next > PWM_DUTY_MAX_PERCENT)
        {
            next = PWM_DUTY_MIN_PERCENT; /* wrap to 0% */
        }
        s_gPwm.dutyPercent[i] = next;
    }

    /* Stage synchronous update via TOM timebase */
    IfxGtm_Tom_Timer_disableUpdate(&s_gPwm.timer);
    IfxGtm_Pwm_updateChannelsDuty(&s_gPwm.pwm, &s_gPwm.dutyPercent[0]);
    IfxGtm_Tom_Timer_applyUpdate(&s_gPwm.timer);
}
