/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-phase inverter PWM using IfxGtm_Pwm
 */
#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ========================= Configuration Macros ========================= */
#define GTM_TOM_INV_NUM_CHANNELS         (3u)
#define GTM_TOM_INV_PWM_FREQUENCY_HZ     (20000.0f)     /* 20 kHz */

#define PHASE_U_INIT_DUTY                (25.0f)        /* percent */
#define PHASE_V_INIT_DUTY                (50.0f)        /* percent */
#define PHASE_W_INIT_DUTY                (75.0f)        /* percent */

#define PHASE_DUTY_STEP                  (10.0f)        /* percent */
#define PHASE_DUTY_MIN                   (10.0f)        /* percent */
#define PHASE_DUTY_MAX                   (90.0f)        /* percent */

/* LED debug pin (port, pin) */
#define LED                              &MODULE_P13, 0

/*
 * Pin routing: preserve TOM1 high-side pins (single-ended)
 * The symbols are provided by the iLLD pin map headers in the integration environment.
 */
#define PHASE_U_HS                       &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_V_HS                       &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_W_HS                       &IfxGtm_TOM1_6_TOUT16_P00_7_OUT

/* ========================= Module State ========================= */
typedef struct
{
    IfxGtm_Pwm             pwm;                                        /* driver handle */
    IfxGtm_Pwm_Channel     channels[GTM_TOM_INV_NUM_CHANNELS];         /* persistent channel objects */
    float32                dutyCycles[GTM_TOM_INV_NUM_CHANNELS];       /* percent */
    float32                phases[GTM_TOM_INV_NUM_CHANNELS];           /* phase offsets (deg or %) */
    IfxGtm_Pwm_DeadTime    deadTimes[GTM_TOM_INV_NUM_CHANNELS];        /* stored dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv = {0};

/* ========================= Public API ========================= */
/**
 * Initialize 3-phase center-aligned PWM on TOM using IfxGtm_Pwm with U,V,W channels.
 * - Single-ended on high-side pins P00.3/P00.5/P00.7 (complementaryPin = NULL)
 * - Duties: U=25%, V=50%, W=75%
 * - 20 kHz, center-aligned, syncStart + syncUpdate enabled
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_INV_NUM_CHANNELS];

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration and per-channel initial setup */
    /* Phase U (index 0) */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = NULL_PTR; /* single-ended */
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (index 1) */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = NULL_PTR;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (index 2) */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = NULL_PTR;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM (dead-time) configuration — zero since complementary outputs are not used */
    dtmConfig[0].deadTime.rising = 0.0f; dtmConfig[0].deadTime.falling = 0.0f;
    dtmConfig[1].deadTime.rising = 0.0f; dtmConfig[1].deadTime.falling = 0.0f;
    dtmConfig[2].deadTime.rising = 0.0f; dtmConfig[2].deadTime.falling = 0.0f;

    /* Channel logical indices 0..2 with initial duties 25/50/75 percent */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = NULL_PTR; /* no ISR used */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 4) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM_INV_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_INV_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;              /* TOM uses FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;

    /* 5) GTM enable guard and CMU clocks (dynamic frequency) */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent handle and channels */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 7) Store persistent state for runtime updates */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* LED pin as push-pull output for optional ISR/debug (no ISR configured here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duties in percent and apply synchronously.
 * Each call: +10% on U,V,W; if new value >= 90% then wrap to 10%.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Update state (explicitly per-channel; no loop as per coding standard) */
    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    if (g_gtmTom3phInv.dutyCycles[0] >= PHASE_DUTY_MAX) { g_gtmTom3phInv.dutyCycles[0] = PHASE_DUTY_MIN; }

    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    if (g_gtmTom3phInv.dutyCycles[1] >= PHASE_DUTY_MAX) { g_gtmTom3phInv.dutyCycles[1] = PHASE_DUTY_MIN; }

    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;
    if (g_gtmTom3phInv.dutyCycles[2] >= PHASE_DUTY_MAX) { g_gtmTom3phInv.dutyCycles[2] = PHASE_DUTY_MIN; }

    /* Apply synchronously to all configured channels */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)&g_gtmTom3phInv.dutyCycles[0]);
}
