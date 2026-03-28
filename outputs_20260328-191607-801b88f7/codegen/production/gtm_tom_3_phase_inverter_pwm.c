/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: 3-phase complementary PWM on GTM TOM1 Cluster_1 using IfxGtm_Pwm.
 *
 * Requirements satisfied:
 * - Center-aligned 20 kHz PWM, complementary outputs with 1 us HW dead-time (DTM)
 * - Sync start and sync update enabled
 * - Pins: U(P02.0/P02.7), V(P02.1/P02.4), W(P02.2/P02.5)
 * - Period-event routed by IfxGtm_Pwm via InterruptConfig; empty period callback provided
 * - Separate ATOM ISR stub toggling P13.0 (priority 20 on CPU0)
 * - GTM enable/clock guarded: only if disabled; GCLK=module frequency; CLK0=GCLK; FXCLK+CLK0 enabled
 * - No watchdog, no STM delay logic, no external SRC/IRQ install
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =====================================================================================
 * Macros (numeric constants and pins)
 * ===================================================================================== */
#define NUM_OF_CHANNELS           (3)
#define PWM_FREQUENCY_HZ          (20000.0f)   /* 20 kHz */

#define PHASE_U_INIT_DUTY         (25.0f)
#define PHASE_V_INIT_DUTY         (50.0f)
#define PHASE_W_INIT_DUTY         (75.0f)
#define PHASE_DUTY_STEP           (10.0f)

#define ISR_PRIORITY_ATOM         (20)

/* LED/debug pin for ISR toggle */
#define LED                       &MODULE_P13, 0

/* User-requested pin assignments (validated TOUT symbols) */
#define PHASE_U_HS                (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS                (&IfxGtm_TOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS                (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* =====================================================================================
 * Module state
 * ===================================================================================== */

typedef struct
{
    IfxGtm_Pwm             pwm;                               /* PWM driver handle */
    IfxGtm_Pwm_Channel     channels[NUM_OF_CHANNELS];         /* Persistent channels array (owned by driver) */
    float32                dutyCycles[NUM_OF_CHANNELS];        /* Duty in percent (0..100) */
    float32                phases[NUM_OF_CHANNELS];            /* Phase in degrees/percent (unused: init to 0) */
    IfxGtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];         /* Applied dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;

/* =====================================================================================
 * ISR and callback stubs (must appear before init)
 * ===================================================================================== */

/* ATOM ISR: priority 20 on CPU0, body toggles P13.0 only */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Empty period-event callback (assigned via InterruptConfig) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =====================================================================================
 * Initialization
 * ===================================================================================== */

/**
 * Initializes a 3-phase complementary PWM on TOM1 Cluster_1 using IfxGtm_Pwm.
 * Sequence strictly follows iLLD initialization pattern and design requirements.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects + use persistent module state (g_gtmTom3phInv) */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load default configuration */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output and DTM configuration (complementary, push-pull, cmosAutomotiveSpeed1) */
    /* U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;          /* high-side active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;           /* low-side active low (complementary) */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM: 1.0 us rising/falling dead time on all channels */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* Sync settings */
    config.syncStart         = TRUE;
    config.syncUpdateEnabled = TRUE;

    /* Interrupt configuration: period event on CPU0, prio=20, mode=pulseNotify, empty callback */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel configurations (logical indices 0..2 map to phases U,V,W) */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig; /* base channel interrupt */

    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 4) Complete main configuration fields */
    config.cluster       = IfxGtm_Cluster_1;                 /* TOM1 Cluster_1 */
    config.subModule     = IfxGtm_Pwm_SubModule_tom;         /* TOM */
    config.alignment     = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.numChannels   = (uint8)NUM_OF_CHANNELS;
    config.channels      = &channelConfig[0];
    config.frequency     = PWM_FREQUENCY_HZ;                 /* 20 kHz */
    config.clockSource.atom = IfxGtm_Cmu_Fxclk_0;            /* FXCLK0 as source */
    config.dtmClockSource   = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 5) GTM enable/clock guard (inside guard ONLY) */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize the PWM with persistent channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 7) Store initial state (duties, phases, dead-times) */
    g_gtmTom3phInv.dutyCycles[0] = PHASE_U_INIT_DUTY;
    g_gtmTom3phInv.dutyCycles[1] = PHASE_V_INIT_DUTY;
    g_gtmTom3phInv.dutyCycles[2] = PHASE_W_INIT_DUTY;

    g_gtmTom3phInv.phases[0] = 0.0f;
    g_gtmTom3phInv.phases[1] = 0.0f;
    g_gtmTom3phInv.phases[2] = 0.0f;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure LED/debug GPIO (P13.0) as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =====================================================================================
 * Duty update
 * ===================================================================================== */

/**
 * Updates U/V/W duty cycles by STEP=10 with wrap rule, then applies immediate synced update.
 * Duty units are percent (0..100), passed directly to the unified PWM driver.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule per-phase (explicit, no loop): if (duty+STEP) >= 100, set to 0; then add STEP */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediate multi-channel update with synchronous semantics */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
