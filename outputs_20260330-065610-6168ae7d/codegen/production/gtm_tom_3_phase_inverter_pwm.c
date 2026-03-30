/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm driver
 * - Center-aligned complementary mode
 * - 20 kHz PWM
 * - 1 us dead-time (rise/fall)
 * - Sync start and sync update enabled
 * - Immediate duty update API used in runtime
 *
 * Notes:
 * - No watchdog manipulation in this file (CpuX_Main.c only)
 * - No STM timing in this driver (timing belongs to CpuX_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ============================ Macros and Defines ============================ */

/* Channel count: 3 complementary pairs (U, V, W) */
#define NUM_OF_CHANNELS            (3u)

/* PWM frequency in Hz */
#define PWM_FREQUENCY_HZ           (20000.0f)

/* ISR priority (provider CPU0, priority 20) */
#define ISR_PRIORITY_ATOM          (20)

/* Initial duty cycle percentages (not applied during init; stored for first update) */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)

/* Duty update step (%) */
#define PHASE_DUTY_STEP            (10.0f)

/* LED pin macro: expands to two args (port pointer, pin index) */
#define LED                        &MODULE_P13, 0

/*
 * Pin mapping placeholders:
 * No validated TOUT pin symbols were provided in the template context.
 * Replace NULL_PTR with validated &IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols during board integration.
 */
#define PHASE_U_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTx_P02_0_OUT */
#define PHASE_U_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTy_P02_7_OUT */
#define PHASE_V_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTx_P02_1_OUT */
#define PHASE_V_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTy_P02_4_OUT */
#define PHASE_W_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTx_P02_2_OUT */
#define PHASE_W_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTy_P02_5_OUT */

/* ============================ Forward Declarations ========================= */

/* Period-event callback: empty body, assigned into InterruptConfig */
void IfxGtm_periodEventFunction(void *data);

/* ISR declaration (provider CPU0, priority ISR_PRIORITY_ATOM) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/* ============================ Module State ================================= */

typedef struct
{
    IfxGtm_Pwm             pwm;                               /* PWM handle */
    IfxGtm_Pwm_Channel     channels[NUM_OF_CHANNELS];         /* Persistent channels array */
    float32                dutyCycles[NUM_OF_CHANNELS];        /* Duty cycles in percent */
    float32                phases[NUM_OF_CHANNELS];            /* Phase offsets in percent */
    IfxGtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];         /* Dead-time settings */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;                 /* Persistent module state */

/* ============================ ISR and Callback ============================= */

/*
 * ISR body for GTM PWM-routed interrupt: toggle LED and return.
 * The high-level driver routes the interrupt; no handler call here.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Empty period-event callback assigned to InterruptConfig.periodEvent.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================ Public API =================================== */

/*
 * Initialize GTM TOM1 for 3-phase inverter PWM using unified IfxGtm_Pwm.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 3) Load safe defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 4) Output configuration: complementary pairs with polarity and pad settings */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;           /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;            /* LS active low  */
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

    /* 4) DTM dead-time configuration: 1 us rise/fall for all channels */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 5) Base channel interrupt configuration (only for channelConfig[0]) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 5) Per-channel configuration: logical indices 0..2 */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = 0.0f;                     /* Do not apply initial duty in init */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;         /* Base channel ISR routing */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = 0.0f;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = 0.0f;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7/Overall) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                 /* Target TOM1 cluster if routed */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;               /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;
    config.syncUpdateEnabled    = TRUE;

    /* 2) GTM enable + CMU clocks inside guard */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent handle and channels[] from module state */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 10) Store configured phases and dead-times, then store initial duties for first runtime update */
    g_gtmTom3phInv.phases[0]    = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1]    = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2]    = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    g_gtmTom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_gtmTom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_gtmTom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    /* 8) Configure LED GPIO as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Percentage-based duty update for 3 logical channels using wrap rule, then apply immediately.
 */
void updateGtmTom3phInvDuty(void)
{
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
