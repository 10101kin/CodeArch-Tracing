/*
 * gtm_tom_3_phase_inverter_pwmgg.c
 *
 * TC387 (TC3xx) GTM/TOM 3-phase inverter PWM using IfxGtm_Pwm unified driver.
 *
 * This module conforms to the SW Detailed Design API contract where a single
 * function updateGtmTomPwmDutyCycles() performs both the initial PWM setup on
 * first invocation and the periodic duty update (+10% step with wrap) on
 * subsequent invocations.
 */

#include "gtm_tom_3_phase_inverter_pwmgg.h"

/* iLLD headers (generic) */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ========================= Requirements-driven configuration ========================= */
#define NUM_OF_CHANNELS                   (3U)

/* From timing_requirements */
#define TIMING_PWM_FREQUENCY_HZ           (20000.0f)   /* 20 kHz */
#define TIMING_SYNCHRONOUS_UPDATE         (TRUE)
#define TIMING_DUTY_UPDATE_PERIOD_MS      (500U)       /* caller controlled */
#define TIMING_DUTY_STEP_PERCENT          (10.0f)      /* +10 percentage points */
#define TIMING_DEADTIME_NS                (0U)
#define TIMING_MIN_PULSE_NS               (0U)

/* From device_configuration */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ    (300U)
#define CLOCK_REQUIRES_XTAL               (TRUE)

/* Shared time base (TOM1 CH0) per requirements */
#define SHARED_TIMEBASE_MODULE_TOM        (1U)     /* TOM1 */
#define SHARED_TIMEBASE_MASTER_CHANNEL    (0U)     /* CH0  */

/* Initial duties per user requirement (percent representation) */
#define DUTY_INIT_U_PERCENT               (25.0f)
#define DUTY_INIT_V_PERCENT               (50.0f)
#define DUTY_INIT_W_PERCENT               (75.0f)

/* ========================= Pin selection (validated for TC387) ========================= */
/* Single-ended outputs on TOM1 channels 2/4/6, default proposal from user text. */
#define PHASE_U_TOUT   ((IfxGtm_Tom_ToutMap*)&IfxGtm_TOM1_2_TOUT12_P00_3_OUT) /* U = P00.3 TOM1 CH2 */
#define PHASE_V_TOUT   ((IfxGtm_Tom_ToutMap*)&IfxGtm_TOM1_4_TOUT14_P00_5_OUT) /* V = P00.5 TOM1 CH4 */
#define PHASE_W_TOUT   ((IfxGtm_Tom_ToutMap*)&IfxGtm_TOM1_6_TOUT16_P00_7_OUT) /* W = P00.7 TOM1 CH6 */

/* ========================= Driver context ========================= */
typedef struct
{
    IfxGtm_Pwm          pwm;                              /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];        /* Channel state returned by init */
    float32             dutyCycles[NUM_OF_CHANNELS];       /* Runtime duty array (percent) */
    float32             phases[NUM_OF_CHANNELS];           /* Optional phase (unused here) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];       /* Dead-times (0 for single-ended) */
} GtmTom3phInv;

static GtmTom3phInv s_drv;                 /* Static driver instance */
static boolean      s_initialized = FALSE; /* First-call initialization gate */

/* ========================= Internal helpers ========================= */
static inline void gtm_clockEnable(void)
{
    /* Enable GTM and set up CMU clocks (FXCLK and CLK0) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
    }

    {
        /* Use module frequency as GCLK, then set CLK0 to GCLK and enable FXCLK | CLK0 */
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        {
            float32 gclk = IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, gclk);
        }
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }
}

/* ========================= Public API implementation ========================= */
void updateGtmTomPwmDutyCycles(void)
{
    if (!s_initialized)
    {
        /* ------------------------- Initialization path (first call) ------------------------- */
        IfxGtm_Pwm_Config           config;
        IfxGtm_Pwm_ChannelConfig    chCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig     outCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_DtmConfig        dtmCfg[NUM_OF_CHANNELS];

        /* 1) Enable GTM and clocks (FXCLK, CLK0) */
        gtm_clockEnable();

        /* 2) Route pins (single-ended). Using PinMap per SW Detailed Design driver_calls. */
        IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_U_TOUT, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
        IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_V_TOUT, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
        IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_W_TOUT, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

        /* 3) Initialize top-level PWM config */
        IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

        /* 4) Output configuration (single-ended: complementaryPin=NULL_PTR). */
        outCfg[0].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_U_TOUT;
        outCfg[0].complementaryPin        = NULL_PTR;
        outCfg[0].polarity                = Ifx_ActiveState_high;
        outCfg[0].complementaryPolarity   = Ifx_ActiveState_low; /* ignored for single-ended */
        outCfg[0].outputMode              = IfxPort_OutputMode_pushPull;
        outCfg[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[1].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_V_TOUT;
        outCfg[1].complementaryPin        = NULL_PTR;
        outCfg[1].polarity                = Ifx_ActiveState_high;
        outCfg[1].complementaryPolarity   = Ifx_ActiveState_low;
        outCfg[1].outputMode              = IfxPort_OutputMode_pushPull;
        outCfg[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[2].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_W_TOUT;
        outCfg[2].complementaryPin        = NULL_PTR;
        outCfg[2].polarity                = Ifx_ActiveState_high;
        outCfg[2].complementaryPolarity   = Ifx_ActiveState_low;
        outCfg[2].outputMode              = IfxPort_OutputMode_pushPull;
        outCfg[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* 5) Zero dead-time for single-ended outputs (DTM still provided as array). */
        dtmCfg[0].deadTime.rising  = 0.0f;
        dtmCfg[0].deadTime.falling = 0.0f;
        dtmCfg[1].deadTime.rising  = 0.0f;
        dtmCfg[1].deadTime.falling = 0.0f;
        dtmCfg[2].deadTime.rising  = 0.0f;
        dtmCfg[2].deadTime.falling = 0.0f;

        /* 6) Initialize and fill each channel config. Shared time base: TOM1 CH0, channels: 2/4/6. */
        IfxGtm_Pwm_initChannelConfig(&chCfg[0]);
        IfxGtm_Pwm_initChannelConfig(&chCfg[1]);
        IfxGtm_Pwm_initChannelConfig(&chCfg[2]);

        /* Channel indices correspond to TOM1 channels 2, 4, 6 respectively */
        chCfg[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;  /* U */
        chCfg[0].phase     = 0.0f;
        chCfg[0].duty      = DUTY_INIT_U_PERCENT;
        chCfg[0].dtm       = &dtmCfg[0];
        chCfg[0].output    = &outCfg[0];
        chCfg[0].mscOut    = NULL_PTR;
        chCfg[0].interrupt = NULL_PTR; /* No ISR required */

        chCfg[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_4;  /* V */
        chCfg[1].phase     = 0.0f;
        chCfg[1].duty      = DUTY_INIT_V_PERCENT;
        chCfg[1].dtm       = &dtmCfg[1];
        chCfg[1].output    = &outCfg[1];
        chCfg[1].mscOut    = NULL_PTR;
        chCfg[1].interrupt = NULL_PTR;

        chCfg[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_6;  /* W */
        chCfg[2].phase     = 0.0f;
        chCfg[2].duty      = DUTY_INIT_W_PERCENT;
        chCfg[2].dtm       = &dtmCfg[2];
        chCfg[2].output    = &outCfg[2];
        chCfg[2].mscOut    = NULL_PTR;
        chCfg[2].interrupt = NULL_PTR;

        /* 7) Populate global PWM config fields */
        config.cluster             = IfxGtm_Cluster_1;                /* TOM1 cluster */
        config.subModule           = IfxGtm_Pwm_SubModule_tom;        /* TOM */
        config.alignment           = IfxGtm_Pwm_Alignment_center;     /* center-aligned */
        config.syncStart           = FALSE;                           /* we'll call startSyncedChannels() */
        config.syncUpdateEnabled   = TIMING_SYNCHRONOUS_UPDATE;       /* shadow-update at period */
        config.numChannels         = NUM_OF_CHANNELS;
        config.channels            = &chCfg[0];
        config.frequency           = TIMING_PWM_FREQUENCY_HZ;
        config.clockSource.tom     = IfxGtm_Cmu_Fxclk_0;              /* TOM uses Fxclk */
        config.dtmClockSource      = IfxGtm_Dtm_ClockSource_cmuClock0;
        /* Shared time base must use TOM1 CH0 per requirement - handled internally by driver */

        /* 8) Initialize PWM driver */
        IfxGtm_Pwm_init(&s_drv.pwm, &s_drv.channels[0], &config);

        /* 9) Store runtime duty array and apply initial update into shadow regs */
        s_drv.dutyCycles[0] = DUTY_INIT_U_PERCENT;
        s_drv.dutyCycles[1] = DUTY_INIT_V_PERCENT;
        s_drv.dutyCycles[2] = DUTY_INIT_W_PERCENT;
        IfxGtm_Pwm_updateChannelsDuty(&s_drv.pwm, &s_drv.dutyCycles[0]);

        /* 10) Start synchronized channels so updates take effect on period-match */
        IfxGtm_Pwm_startSyncedChannels(&s_drv.pwm);

        s_initialized = TRUE;
        return;
    }

    /* ------------------------- Runtime update path (subsequent calls) ------------------------- */
    {
        uint8 i;
        for (i = 0U; i < NUM_OF_CHANNELS; i++)
        {
            s_drv.dutyCycles[i] += TIMING_DUTY_STEP_PERCENT;
            if (s_drv.dutyCycles[i] >= 100.0f)
            {
                s_drv.dutyCycles[i] = 0.0f;
            }
        }
        /* Apply updated duties (shadowed + synchronous) */
        IfxGtm_Pwm_updateChannelsDuty(&s_drv.pwm, &s_drv.dutyCycles[0]);
    }
}
