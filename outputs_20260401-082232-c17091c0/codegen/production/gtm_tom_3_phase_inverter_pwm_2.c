/*
 * gtm_tom_3_phase_inverter_pwm_2.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm (TC3xx).
 *
 * Notes:
 * - Follows iLLD IfxGtm_Pwm initialization pattern with CMU guard.
 * - Complementary outputs configured via OutputConfig array.
 * - ISR only toggles LED; period callback is empty per design rules.
 * - No watchdog handling here (must be in CpuX_Main.c only).
 */

#include "gtm_tom_3_phase_inverter_pwm_2.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ============================= Macros and Configuration Constants ============================= */

/* Channel count */
#define GTM_TOM_INV_NUM_CHANNELS          (3U)

/* PWM operating frequency (Hz) */
#define GTM_TOM_INV_PWM_FREQUENCY_HZ      (20000.0f)

/* ISR priority (used both in IFX_INTERRUPT and InterruptConfig.priority) */
#define ISR_PRIORITY_ATOM                 (20)

/* Initial phase duties in percent */
#define PHASE_U_DUTY_INIT                 (25.0f)
#define PHASE_V_DUTY_INIT                 (50.0f)
#define PHASE_W_DUTY_INIT                 (75.0f)

/* Debug LED macro (compound: port, pin) */
#define LED                               &MODULE_P13, 0

/* TOUT routing placeholders (no validated mapping available in this context):
 * Replace NULL_PTR with the proper &IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols matching the requested pins:
 *   Phase U: P02.0 (HS) / P02.7 (LS)
 *   Phase V: P02.1 (HS) / P02.4 (LS)
 *   Phase W: P02.2 (HS) / P02.5 (LS)
 * Validate against IfxGtm_PinMap headers for the target device/package.
 */
#define PHASE_U_HS                        (NULL_PTR) /* TODO: &IfxGtm_TOM1_0_TOUTxx_P02_0_OUT */
#define PHASE_U_LS                        (NULL_PTR) /* TODO: &IfxGtm_TOM1_1_TOUTxx_P02_7_OUT */
#define PHASE_V_HS                        (NULL_PTR) /* TODO: &IfxGtm_TOM1_2_TOUTxx_P02_1_OUT */
#define PHASE_V_LS                        (NULL_PTR) /* TODO: &IfxGtm_TOM1_3_TOUTxx_P02_4_OUT */
#define PHASE_W_HS                        (NULL_PTR) /* TODO: &IfxGtm_TOM1_4_TOUTxx_P02_2_OUT */
#define PHASE_W_LS                        (NULL_PTR) /* TODO: &IfxGtm_TOM1_5_TOUTxx_P02_5_OUT */

/* ============================= Module State ============================= */

typedef struct
{
    IfxGtm_Pwm                pwm;                                        /* PWM driver handle */
    IfxGtm_Pwm_Channel        channels[GTM_TOM_INV_NUM_CHANNELS];         /* Persistent channels storage */
    float32                   dutyCycles[GTM_TOM_INV_NUM_CHANNELS];       /* Duty in percent (0..100) */
    float32                   phases[GTM_TOM_INV_NUM_CHANNELS];           /* Phase in degrees (or percent of period) */
    IfxGtm_Pwm_DeadTime       deadTimes[GTM_TOM_INV_NUM_CHANNELS];        /* Dead-time per channel */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInvState;

/* ============================= ISR and Callback (must appear before init) ============================= */

/* Period callback for IfxGtm_Pwm interrupt config — required symbol, empty body by design */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR: Toggles diagnostic LED on each PWM period interrupt.
 * Priority macro must match InterruptConfig.priority.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* ============================= Public API Implementation ============================= */

/**
 * Initialize 3-phase complementary PWM on GTM TOM (Cluster_1), center-aligned at 20 kHz.
 * - Configures three logical channels (ordinal indices 0..2) with complementary outputs.
 * - Enables GTM and CMU clocks if not already enabled (guarded).
 * - Sets dead-time, sync start and sync update, and assigns a period callback.
 * - Sets up a debug LED pin for ISR toggling; does not modify its output level here.
 */
void initGtmTom3phInv(void)
{
    /* Local configuration structures per iLLD pattern */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* ---------------- Output configuration: complementary pairs ---------------- */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;  /* HS */
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;  /* LS */
    output[0].polarity               = Ifx_ActiveState_high;             /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;              /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* ---------------- Dead-time configuration ---------------- */
    dtmConfig[0].deadTime.rising = 5e-07f;  /* 0.5 us per design */
    dtmConfig[0].deadTime.falling = 5e-07f;

    dtmConfig[1].deadTime.rising = 5e-07f;
    dtmConfig[1].deadTime.falling = 5e-07f;

    dtmConfig[2].deadTime.rising = 5e-07f;
    dtmConfig[2].deadTime.falling = 5e-07f;

    /* ---------------- Interrupt configuration ---------------- */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;     /* pulse/period notify */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;                /* CPU0 */
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;/* same as IFX_INTERRUPT */
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;     /* empty callback */
    interruptConfig.dutyEvent   = NULL_PTR;                       /* no duty callback */

    /* ---------------- Channel configuration (ordinal indices 0..2) ---------------- */
    /* CH0 → Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;     /* base channel interrupt */

    /* CH1 → Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;             /* only CH0 has interrupt */

    /* CH2 → Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* ---------------- Main PWM configuration ---------------- */
    config.cluster              = IfxGtm_Cluster_1;                      /* Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;              /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;           /* center-aligned */
    config.syncStart            = TRUE;                                   /* sync start */
    config.syncUpdateEnabled    = TRUE;                                   /* sync update */
    config.numChannels          = (uint8)GTM_TOM_INV_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_INV_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;            /* TOM uses FXCLK_0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;      /* DTM clock */

    /* ---------------- Enable guard: GTM + CMU clocks ---------------- */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        /* Dynamic CMU frequency (no hard-coded value) */
        {
            float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }

    /* ---------------- Initialize PWM (handle + persistent channels) ---------------- */
    IfxGtm_Pwm_init(&g_gtmTom3phInvState.pwm, &g_gtmTom3phInvState.channels[0], &config);

    /* ---------------- Persist initial state for runtime updates ---------------- */
    g_gtmTom3phInvState.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInvState.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInvState.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInvState.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInvState.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInvState.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* ---------------- Configure diagnostic LED GPIO as push-pull output ---------------- */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
