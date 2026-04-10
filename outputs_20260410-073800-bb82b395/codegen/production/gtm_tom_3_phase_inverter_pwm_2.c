/*
 * gtm_tom_3_phase_inverter_pwm_2.c
 *
 * Production driver: 3-phase complementary PWM using GTM TOM via IfxGtm_Pwm
 * - TC3xx family, TOM submodule, center-aligned 20 kHz, complementary HS/LS
 * - Sync start and sync update enabled
 * - 1 us dead time (rising and falling)
 * - GTM enable guard with CMU configuration
 * - Minimal ISR toggles LED (P13.0)
 *
 * Notes:
 * - Watchdog handling must NOT be added here (handled only in CpuX_Main.c).
 * - No STM timing in this driver. Scheduling belongs to CpuX_Main.c.
 */

#include "gtm_tom_3_phase_inverter_pwm_2.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =========================================================================================
 * Configuration macros (values from requirements)
 * ========================================================================================= */
#define GTM_TOM_INV_NUM_CHANNELS      (3u)
#define GTM_TOM_INV_PWM_FREQUENCY     (20000.0f)     /* 20 kHz */
#define GTM_TOM_INV_DEADTIME_S        (1.0e-6f)      /* 1 us */
#define PHASE_U_INIT_DUTY             (25.0f)
#define PHASE_V_INIT_DUTY             (50.0f)
#define PHASE_W_INIT_DUTY             (75.0f)
#define PHASE_DUTY_STEP               (10.0f)
#define ISR_PRIORITY_ATOM             (20)

/* LED: P13.0 */
#define LED                           (&MODULE_P13), (0)

/* =========================================================================================
 * Pin macros
 *
 * No validated TOUT pin symbols were provided by the template for TC3xx in this context.
 * Replace the NULL_PTR placeholders below with the actual IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT
 * symbols that match your device/pinout (see IfxGtm_PinMap.h):
 *   U: P02.0 (HS) -> &IfxGtm_TOM1_0_TOUT0_P02_0_OUT,
 *      P02.7 (LS) -> &IfxGtm_TOM1_0N_TOUT7_P02_7_OUT
 *   V: P02.1 (HS) -> &IfxGtm_ATOM1_1_TOUT1_P02_1_OUT,
 *      P02.4 (LS) -> &IfxGtm_TOM1_12_TOUT4_P02_4_OUT
 *   W: P02.2 (HS) -> &IfxGtm_TOM1_10_TOUT2_P02_2_OUT,
 *      P02.5 (LS) -> &IfxGtm_ATOM0_2N_TOUT5_P02_5_OUT
 * ========================================================================================= */
#define PHASE_U_HS    (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUT0_P02_0_OUT */
#define PHASE_U_LS    (NULL_PTR) /* Replace with &IfxGtm_TOM1_0N_TOUT7_P02_7_OUT */
#define PHASE_V_HS    (NULL_PTR) /* Replace with &IfxGtm_ATOM1_1_TOUT1_P02_1_OUT */
#define PHASE_V_LS    (NULL_PTR) /* Replace with &IfxGtm_TOM1_12_TOUT4_P02_4_OUT */
#define PHASE_W_HS    (NULL_PTR) /* Replace with &IfxGtm_TOM1_10_TOUT2_P02_2_OUT */
#define PHASE_W_LS    (NULL_PTR) /* Replace with &IfxGtm_ATOM0_2N_TOUT5_P02_5_OUT */

/* =========================================================================================
 * Module state
 * ========================================================================================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                 /* PWM driver handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM_INV_NUM_CHANNELS];  /* Persistent channel objects */
    float32                 dutyCycles[GTM_TOM_INV_NUM_CHANNELS];
    float32                 phases[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM_INV_NUM_CHANNELS];
} GtmTom3phInv2_State;

IFX_STATIC GtmTom3phInv2_State g_gtmTom3phInv2;

/* =========================================================================================
 * Forward declarations (ISR and callback must appear before init per structure rules)
 * ========================================================================================= */

/* ISR: routed by GTM infrastructure; minimal body toggles LED */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period event callback: must remain empty */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =========================================================================================
 * initGtmTom3phInv
 * ========================================================================================= */
/** \brief Initialize 3-phase complementary PWM on TOM in center-aligned mode at 20 kHz. */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_DeadTime         deadTimesLocal[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Initialize main PWM config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (pins, polarities, pad/drive) */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high; /* HS active HIGH */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;  /* LS active LOW  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase V */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase W */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) Dead-time configuration per logical channel (1 us rising/falling) */
    deadTimesLocal[0].rising = GTM_TOM_INV_DEADTIME_S; deadTimesLocal[0].falling = GTM_TOM_INV_DEADTIME_S;
    deadTimesLocal[1].rising = GTM_TOM_INV_DEADTIME_S; deadTimesLocal[1].falling = GTM_TOM_INV_DEADTIME_S;
    deadTimesLocal[2].rising = GTM_TOM_INV_DEADTIME_S; deadTimesLocal[2].falling = GTM_TOM_INV_DEADTIME_S;

    dtmConfig[0].deadTime = deadTimesLocal[0];
    dtmConfig[1].deadTime = deadTimesLocal[1];
    dtmConfig[2].deadTime = deadTimesLocal[2];

    /* 6) Interrupt configuration for unified driver */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;   /* Period notification */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 4) Channel configuration (logical indices 0..2) */
    /* Channel 0 -> Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;   /* Base/logical channel gets interrupt */

    /* Channel 1 -> Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Channel 2 -> Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Complete main config */
    config.cluster              = IfxGtm_Cluster_1;                       /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM_INV_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_INV_PWM_FREQUENCY;
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;            /* FXCLK0 for TOM */

    /* 8) GTM enable guard with CMU configuration */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent state */
    IfxGtm_Pwm_init(&g_gtmTom3phInv2.pwm, &g_gtmTom3phInv2.channels[0], &config);

    /* 10) Store initial duty cycles and dead-times into persistent state */
    g_gtmTom3phInv2.dutyCycles[0] = PHASE_U_INIT_DUTY;
    g_gtmTom3phInv2.dutyCycles[1] = PHASE_V_INIT_DUTY;
    g_gtmTom3phInv2.dutyCycles[2] = PHASE_W_INIT_DUTY;

    g_gtmTom3phInv2.phases[0] = 0.0f;
    g_gtmTom3phInv2.phases[1] = 0.0f;
    g_gtmTom3phInv2.phases[2] = 0.0f;

    g_gtmTom3phInv2.deadTimes[0] = deadTimesLocal[0];
    g_gtmTom3phInv2.deadTimes[1] = deadTimesLocal[1];
    g_gtmTom3phInv2.deadTimes[2] = deadTimesLocal[2];

    /* 11) Configure LED GPIO (P13.0) as push-pull output */
    IfxPort_setPinModeOutput(&MODULE_P13, 0u, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =========================================================================================
 * updateGtmTom3phInvDuty
 * ========================================================================================= */
/** \brief Step-and-wrap duty update (+10%, wrap to 10% after reaching 100%) applied immediately */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 then duty=0; then add step (always) */
    if ((g_gtmTom3phInv2.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv2.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv2.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv2.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv2.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv2.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv2.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv2.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv2.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate multi-channel duty update (percent values) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv2.pwm, (float32 *)g_gtmTom3phInv2.dutyCycles);
}
