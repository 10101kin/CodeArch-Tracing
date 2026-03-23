/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production 3-phase PWM module using IfxGtm_Pwm unified driver (TC3xx, TOM).
 * Matches SW Detailed Design and uses real iLLD APIs with exact signatures.
 */
#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm.h"
#include "IfxGtm_Pwm.h"

/* ===================== Pin selection (single-ended outputs) =====================
 * Default proposal from requirements (validate on target board):
 *  - Phase U: P00.3  -> TOM1 CH2  (TOUT12)
 *  - Phase V: P00.5  -> TOM1 CH4  (TOUT14)
 *  - Phase W: P00.7  -> TOM1 CH6  (TOUT16)
 * Low-side (complementary) pins are unused in this migration.
 */
#define PHASE_U_TOM_TOUT   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_V_TOM_TOUT   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_W_TOM_TOUT   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)

/* ===================== Driver state ===================== */
typedef struct
{
    IfxGtm_Pwm          pwm;                             /* PWM Driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];       /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];      /* Duty cycle values (percent) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;  /* Internal module instance */

/* ===================== Local helpers (static) ===================== */
static void gtmTom3phInv_initPins(void)
{
    /* Map TOM TOUT pins with desired electrical characteristics */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_TOM_TOUT,
                              IfxPort_OutputMode_pushPull,
                              IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_TOM_TOUT,
                              IfxPort_OutputMode_pushPull,
                              IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_TOM_TOUT,
                              IfxPort_OutputMode_pushPull,
                              IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/* ===================== Public API implementations ===================== */
void initGtmTomPwm(void)
{
    /* 1) Enable GTM and functional clocks (FXCLK for TOM) */
    IfxGtm_enable(&MODULE_GTM);
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* 2) Configure pins for single-ended outputs (no complementary pin) */
    gtmTom3phInv_initPins();

    /* 3) Prepare unified PWM configuration */
    IfxGtm_Pwm_Config          config;
    IfxGtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* PWM core configuration (shared time-base on TOM1, center-aligned, sync updates) */
    config.cluster            = IfxGtm_Cluster_0;                 /* GTM cluster 0 */
    config.subModule          = IfxGtm_Pwm_SubModule_tom;         /* TOM submodule */
    config.alignment          = IfxGtm_Pwm_Alignment_center;      /* Center-aligned */
    config.syncStart          = FALSE;                            /* We'll start explicitly */
    config.syncUpdateEnabled  = TRUE;                             /* Shadow update at period */
    config.frequency          = TIMING_PWM_FREQUENCY_HZ;          /* 20 kHz */
    config.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;               /* FXCLK0 for TOM */

    /* Output routing via unified driver (single-ended) */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_TOM_TOUT; /* U - CH2 */
    output[0].complementaryPin      = NULL_PTR;                                 /* single-ended */
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;                      /* unused */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_TOM_TOUT; /* V - CH4 */
    output[1].complementaryPin      = NULL_PTR;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_TOM_TOUT; /* W - CH6 */
    output[2].complementaryPin      = NULL_PTR;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Initialize per-channel configs and assign shared time-base channels (TOM1 CH2/4/6) */
    IfxGtm_Pwm_initChannelConfig(&channelConfig[0]);
    IfxGtm_Pwm_initChannelConfig(&channelConfig[1]);
    IfxGtm_Pwm_initChannelConfig(&channelConfig[2]);

    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;  /* Phase U */
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = DUTY_25_PERCENT;
    channelConfig[0].dtm       = NULL_PTR;                   /* no dead-time */
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR;                   /* no ISR binding */

    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_4;  /* Phase V */
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = DUTY_50_PERCENT;
    channelConfig[1].dtm       = NULL_PTR;
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_6;  /* Phase W */
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = DUTY_75_PERCENT;
    channelConfig[2].dtm       = NULL_PTR;
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    config.numChannels = NUM_OF_CHANNELS;
    config.channels    = &channelConfig[0];

    /* 4) Initialize PWM driver and channels */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 5) Prepare persistent duty array and queue initial values */
    g_gtmTom3phInv.dutyCycles[0] = DUTY_25_PERCENT;  /* U */
    g_gtmTom3phInv.dutyCycles[1] = DUTY_50_PERCENT;  /* V */
    g_gtmTom3phInv.dutyCycles[2] = DUTY_75_PERCENT;  /* W */

    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);

    /* 6) Start synchronized channels so shadow updates apply at period-match */
    IfxGtm_Pwm_startSyncedChannels(&g_gtmTom3phInv.pwm);
}

void updateGtmTomPwmDutyCycles(void)
{
    /* Algorithm per SW Detailed Design: +10% step, wrap at 100% to 0% */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_gtmTom3phInv.dutyCycles[i] += DUTY_STEP;
        if (g_gtmTom3phInv.dutyCycles[i] >= 100.0f)
        {
            g_gtmTom3phInv.dutyCycles[i] = 0.0f;
        }
    }

    /* Apply via unified multi-channel update; takes effect synchronously */
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
}
