/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm (TC3xx)
 *
 * Notes:
 * - Follows iLLD IfxGtm_Pwm unified driver initialization pattern
 * - Center-aligned TOM PWM, 3 complementary pairs, 20 kHz, 1 us deadtime
 * - Uses sync start and sync update; duty interface in percent [0..100]
 * - GTM enable guard: enable + CMU GCLK/CLK0 inside guard only if GTM disabled
 * - No watchdog handling here (must be in CpuX main files only)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ============================ Macros and Constants ============================ */

/* Channel count: 3 complementary pairs (U, V, W) */
#define GTM_TOM_3PH_NUM_CHANNELS        (3u)

/* PWM frequency in Hz */
#define GTM_TOM_3PH_PWM_FREQUENCY_HZ    (20000.0f)

/* ISR priority for GTM (ATOM/TOM routed by HLD) */
#define ISR_PRIORITY_ATOM               (20)

/* Initial duties in percent */
#define PHASE_U_DUTY_INIT               (25.0f)
#define PHASE_V_DUTY_INIT               (50.0f)
#define PHASE_W_DUTY_INIT               (75.0f)

/* Duty step (percent) */
#define PHASE_DUTY_STEP                 (10.0f)

/* LED/debug pin: P13.0 */
#define LED                              &MODULE_P13, 0

/*
 * Pin routing placeholders: Validated pin symbols were not provided by the PinMap
 * database in this context. Keep these as NULL_PTR and replace with the proper
 * IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols during board integration.
 * Requested mapping (KIT_A2G_TC387_5V_TFT):
 *   U: P02.0 (HS) / P02.7 (LS)
 *   V: P02.1 (HS) / P02.4 (LS)
 *   W: P02.2 (HS) / P02.5 (LS)
 */
#define PHASE_U_HS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTx_P02_0_OUT */
#define PHASE_U_LS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTx_P02_7_OUT */
#define PHASE_V_HS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTx_P02_1_OUT */
#define PHASE_V_LS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTx_P02_4_OUT */
#define PHASE_W_HS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTx_P02_2_OUT */
#define PHASE_W_LS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTx_P02_5_OUT */

/* ============================ Module State ============================ */

typedef struct
{
    IfxGtm_Pwm              pwm;                                      /* PWM handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM_3PH_NUM_CHANNELS];       /* Persistent channel objects */
    float32                 dutyCycles[3];                             /* Percent duties [U,V,W] */
    float32                 phases[3];                                 /* Phase offsets for each logical channel */
    IfxGtm_Pwm_DeadTime     deadTimes[3];                              /* Dead-times configured per channel */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3phInv;

/* ============================ ISR and Callback ============================ */

/* ISR declaration with priority; provider cpu0 (0). The HLD routes to this ISR internally. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

void interruptGtmAtom(void)
{
    /* Minimal ISR body per guidelines: toggle debug LED pin */
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Empty period-event callback hook */
}

/* ============================ Public API ============================ */

void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults using unified initConfig */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for three complementary pairs (U, V, W) */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;           /* High-side active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;            /* Low-side  active LOW  */
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

    /* 4) Dead-time configuration: both edges = 1 us each channel (use 1e-6f literal) */
    dtmConfig[0].deadTime.rising = 1e-6f;  dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;  dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;  dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Base channel interrupt configuration with period callback */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2) */
    /* Channel 0 → Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;      /* Interrupt only on base channel */

    /* Channel 1 → Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;              /* No interrupt on this channel */

    /* Channel 2 → Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;              /* No interrupt on this channel */

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                  /* TOM1 cluster index 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;          /* Use TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;       /* Center-aligned */
    config.syncStart            = TRUE;                               /* Synchronized start */
    config.numChannels          = (uint8)GTM_TOM_3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_3PH_PWM_FREQUENCY_HZ;      /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                /* TOM clock source FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM source CMU CLK0 */
    config.syncUpdateEnabled    = TRUE;                               /* Synchronized update */

    /* 8) Enable guard: enable GTM and CMU clocks only if GTM is disabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver with persistent channels array in module state */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 10) Store initial state for duties, phases, and dead-times */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED/debug GPIO as output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateGtmTom3phInvDuty(void)
{
    /* Step-and-wrap update: check then unconditional add (no loops) */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately using the HLD API and the persistent state array */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);
}
