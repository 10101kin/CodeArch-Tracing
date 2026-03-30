/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production 3-phase inverter PWM driver using IfxGtm_Pwm on TOM submodule.
 * - Center-aligned complementary PWM, 20 kHz
 * - 1 us rising/falling dead time via DTM
 * - Synchronous start and synchronous update enabled
 * - Minimal ISR toggling LED P13.0
 *
 * Notes:
 * - Watchdog handling must be placed in CpuX_Main.c (not here).
 * - No STM-based timing in this module.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ========================= Macros and Configuration Constants ========================= */
#define NUM_OF_CHANNELS            (3U)
#define PWM_FREQUENCY_HZ           (20000.0f)
#define ISR_PRIORITY_ATOM          (20)

/* User-requested pins (no validated TOUT symbols provided) - use NULL_PTR placeholders.
 * Integrators must replace NULL_PTR with valid &IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols. */
#define PHASE_U_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTx_P02_0_OUT */
#define PHASE_U_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTy_P02_7_OUT */
#define PHASE_V_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTx_P02_1_OUT */
#define PHASE_V_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTy_P02_4_OUT */
#define PHASE_W_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTx_P02_2_OUT */
#define PHASE_W_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTy_P02_5_OUT */

/* Duty control (percent) */
#define PHASE_U_DUTY_INIT_PERCENT  (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT  (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT  (75.0f)
#define PHASE_DUTY_STEP_PERCENT    (10.0f)

/* LED: P13.0 (compound macro expands to 2 args: port, pin) */
#define LED                        &MODULE_P13, 0

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm             pwm;                                   /* PWM handle */
    IfxGtm_Pwm_Channel     channels[NUM_OF_CHANNELS];             /* Persistent channels array */
    float32                dutyCycles[NUM_OF_CHANNELS];            /* Duty in percent */
    float32                phases[NUM_OF_CHANNELS];                /* Phase in degrees */
    IfxGtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];            /* Dead time configuration snapshot */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInv;                         /* Persistent driver state */

/* ========================= ISR and Callback Declarations ========================= */

/* ISR declaration with provider CPU0 and configured priority */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/*
 * Empty period-event callback assigned to PWM driver interrupt configuration.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= ISR Definition ========================= */

/*
 * ISR body routed from GTM PWM base channel interrupt: toggle LED and return.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API Implementations ========================= */

/*
 * Initialize the GTM for a 3-phase inverter PWM using IfxGtm_Pwm on TOM.
 * - Center-aligned complementary mode at 20 kHz
 * - Dead time: 1 us rising + 1 us falling
 * - Synchronous start and synchronous update enabled
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare configuration objects as locals */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];

    /* 2) GTM enable sequence inside guard */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 3) Load default configuration for the unified PWM driver */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 4) Output configuration for complementary pairs (U, V, W) */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;           /* HS active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;            /* LS active low  */
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

    /* DTM configuration: 1 us rising and falling dead time for each pair */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* Base channel interrupt configuration */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;   /* pulse notify mode */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;              /* CPU0 */
    interruptConfig.priority    = ISR_PRIORITY_ATOM;            /* priority 20 */
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;   /* empty callback */
    interruptConfig.dutyEvent   = NULL_PTR;                     /* not used */

    /* 5) Per-channel configuration (logical indices Ch_0..Ch_2) */
    /* Channel 0 -> Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = 0.0f;                         /* do not apply initial duties in init */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;             /* base channel interrupt */

    /* Channel 1 -> Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = 0.0f;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;                     /* only base channel has IRQ */

    /* Channel 2 -> Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = 0.0f;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 6) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                 /* target cluster */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;         /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart            = TRUE;                              /* synchronous start */
    config.syncUpdateEnabled    = TRUE;                              /* synchronous update */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                  /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM clock source */

    /* Initialize the PWM (persistent handle and channels from module state) */
    IfxGtm_Pwm_init(&g_tom3phInv.pwm, &g_tom3phInv.channels[0], &config);

    /* 7) Store initial state (do not apply duties here) */
    g_tom3phInv.dutyCycles[0] = PHASE_U_DUTY_INIT_PERCENT;
    g_tom3phInv.dutyCycles[1] = PHASE_V_DUTY_INIT_PERCENT;
    g_tom3phInv.dutyCycles[2] = PHASE_W_DUTY_INIT_PERCENT;

    g_tom3phInv.phases[0] = channelConfig[0].phase;
    g_tom3phInv.phases[1] = channelConfig[1].phase;
    g_tom3phInv.phases[2] = channelConfig[2].phase;

    g_tom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure LED GPIO as push-pull output */
    IfxPort_setPinMode(LED, IfxPort_Mode_outputPushPullGeneral);
}

/*
 * Update duty cycles for U, V, W with percentage step and wrap rule, then apply immediately.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 then duty = 0; duty += step; */
    if ((g_tom3phInv.dutyCycles[0] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_tom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[1] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_tom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[2] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_tom3phInv.dutyCycles[2] = 0.0f; }

    g_tom3phInv.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    g_tom3phInv.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    g_tom3phInv.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;

    /* Apply the new duty cycle requests immediately (driver manages shadow transfer) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInv.pwm, (float32 *)g_tom3phInv.dutyCycles);
}
