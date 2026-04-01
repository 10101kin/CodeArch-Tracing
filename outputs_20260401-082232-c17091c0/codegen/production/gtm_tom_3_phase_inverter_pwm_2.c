/*
 * gtm_tom_3_phase_inverter_pwm_2.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm
 *
 * Implementation follows authoritative iLLD patterns and MIGRATION specification.
 * - Submodule: TOM (Cluster_1)
 * - Center-aligned, sync start and sync update enabled
 * - 3 complementary channels with DTM dead-time
 * - Period ISR toggles a diagnostic LED (P13.0)
 */

#include "gtm_tom_3_phase_inverter_pwm_2.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =====================================================================================
 * Configuration macros (use migration/requirements values; no device/package macros)
 * ===================================================================================== */
#define GTM_TOM_3PH_NUM_CHANNELS      (3U)
#define GTM_TOM_PWM_FREQUENCY_HZ      (20000.0f)     /* PWM switching frequency */
#define ISR_PRIORITY_ATOM             (20)           /* Period ISR priority (CPU0) */

/* Initial duty cycles in percent */
#define PHASE_U_INITIAL_DUTY          (25.0f)
#define PHASE_V_INITIAL_DUTY          (50.0f)
#define PHASE_W_INITIAL_DUTY          (75.0f)

/* Duty step (percent) for runtime updates (kept for state mirroring) */
#define PHASE_DUTY_STEP               (10.0f)

/* Dead-time (seconds) per migration spec */
#define PWM_DEADTIME_S                (5e-07f)

/* LED pin macro (compound form: port, pin) */
#define LED                           &MODULE_P13, 0

/*
 * PWM output pin placeholders:
 * No validated TOUT symbols were provided. Use NULL_PTR placeholders and
 * replace with actual &IfxGtm_TOM1_x_TOUTy_Pzz_w_OUT symbols during board integration.
 */
#define PHASE_U_HS                    (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTx_P02_0_OUT (HS) */
#define PHASE_U_LS                    (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTx_P02_7_OUT (LS) */
#define PHASE_V_HS                    (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTx_P02_1_OUT (HS) */
#define PHASE_V_LS                    (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTx_P02_4_OUT (LS) */
#define PHASE_W_HS                    (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTx_P02_2_OUT (HS) */
#define PHASE_W_LS                    (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTx_P02_5_OUT (LS) */

/* =====================================================================================
 * Module state (persistent) — channels[] must be persistent for driver internal refs
 * ===================================================================================== */
typedef struct
{
    IfxGtm_Pwm              pwm;                                        /* PWM handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM_3PH_NUM_CHANNELS];         /* Persistent channels */
    float32                 dutyCycles[GTM_TOM_3PH_NUM_CHANNELS];       /* Duty in percent */
    float32                 phases[GTM_TOM_3PH_NUM_CHANNELS];           /* Phase in percent-of-period */
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM_3PH_NUM_CHANNELS];        /* Per-channel dead-time */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;

/* =====================================================================================
 * Period callback (empty) — assigned via InterruptConfig.periodEvent
 * ===================================================================================== */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty per design */
}

/* =====================================================================================
 * ISR — toggles diagnostic LED. Priority macro must match InterruptConfig.priority.
 * ===================================================================================== */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* =====================================================================================
 * initGtmTom3phInv — Initialize synchronized 3-phase complementary TOM PWM
 * ===================================================================================== */
void initGtmTom3phInv(void)
{
    /* Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output configuration (HS/LS per phase) */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;      /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;       /* LS active LOW  */
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

    /* DTM (dead-time) configuration per channel */
    dtmConfig[0].deadTime.rising = PWM_DEADTIME_S;
    dtmConfig[0].deadTime.falling = PWM_DEADTIME_S;
    dtmConfig[1].deadTime.rising = PWM_DEADTIME_S;
    dtmConfig[1].deadTime.falling = PWM_DEADTIME_S;
    dtmConfig[2].deadTime.rising = PWM_DEADTIME_S;
    dtmConfig[2].deadTime.falling = PWM_DEADTIME_S;

    /* Interrupt configuration: period notification on base channel (index 0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;   /* period-based notify */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;              /* CPU0 */
    interruptConfig.priority    = ISR_PRIORITY_ATOM;            /* matches IFX_INTERRUPT */
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;   /* empty callback */
    interruptConfig.dutyEvent   = NULL_PTR;                     /* not used */

    /* Channel configuration (ordinal timerCh indices 0..2) */
    /* Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INITIAL_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;             /* base channel ISR */

    /* Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INITIAL_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INITIAL_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Top-level PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM_3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;           /* TOM uses FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* Enable guard — enable GTM and CMU clocks if not already on */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* Initialize PWM — driver stores pointers to persistent state.channels[] */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* Persist initial duties, phases, and dead-times in module state */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* Configure diagnostic LED pin (push-pull). Do not set level here. */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
