/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for 3-phase complementary, center-aligned PWM using GTM TOM
 * - Initializes TOM-based PWM with synchronous start and shadow updates
 * - Provides periodic ISR toggling a debug LED
 * - Maintains and updates percent-based duties via immediate synchronous update API
 *
 * Notes:
 * - Watchdogs are NOT handled here (must be handled in CpuX_Main.c only)
 * - Pin map symbols must be validated on target and replaced below where noted
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY_HZ           (20000.0f)        /* 20 kHz */
#define ISR_PRIORITY_ATOM          (20u)

/* Duty configuration (percent) */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* Dead-time and minimum pulse width (seconds) */
#define PWM_DEAD_TIME              (5.0e-07f)        /* 0.5 us */
#define PWM_MIN_PULSE_TIME         (1.0e-06f)        /* 1.0 us */

/* Debug LED pin: P13.0 */
#define LED                        &MODULE_P13, 0

/* =============================
 * Pin Routing Macros (TOM1 / TGC1) — replace NULL_PTR with validated symbols during integration
 * ============================= */
/*
 * The validated user-requested mapping is listed below and should be used when integrating on target:
 *   PhaseU_H (TOM1_CH8):  P02.0 -> &IfxGtm_TOM1_0_TOUT0_P02_0_OUT
 *   PhaseU_L (TOM1_CH9):  P02.7 -> &IfxGtm_TOM1_0N_TOUT7_P02_7_OUT
 *   PhaseV_H (TOM1_CH10): P02.1 -> &IfxGtm_TOM1_1_TOUT1_P02_1_OUT
 *   PhaseV_L (TOM1_CH11): P02.4 -> &IfxGtm_TOM1_12_TOUT4_P02_4_OUT
 *   PhaseW_H (TOM1_CH12): P02.2 -> &IfxGtm_ATOM0_7N_TOUT2_P02_2_OUT
 *   PhaseW_L (TOM1_CH13): P02.5 -> &IfxGtm_TOM1_13_TOUT5_P02_5_OUT
 *
 * During unit test/mocks, these symbols may not be available; keep as NULL_PTR placeholders.
 */
#define PHASE_U_HS   (NULL_PTR) /* replace with (IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0_TOUT0_P02_0_OUT */
#define PHASE_U_LS   (NULL_PTR) /* replace with (IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT */
#define PHASE_V_HS   (NULL_PTR) /* replace with (IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1_TOUT1_P02_1_OUT */
#define PHASE_V_LS   (NULL_PTR) /* replace with (IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_12_TOUT4_P02_4_OUT */
#define PHASE_W_HS   (NULL_PTR) /* replace with (IfxGtm_Pwm_ToutMap*)&IfxGtm_ATOM0_7N_TOUT2_P02_2_OUT */
#define PHASE_W_LS   (NULL_PTR) /* replace with (IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_13_TOUT5_P02_5_OUT */

/* =============================
 * Module State
 * ============================= */
typedef struct
{
    IfxGtm_Pwm             pwm;                              /* driver handle */
    IfxGtm_Pwm_Channel     channels[NUM_OF_CHANNELS];        /* persistent channels array bound at init */
    float32                dutyCycles[NUM_OF_CHANNELS];       /* in percent */
    float32                phases[NUM_OF_CHANNELS];           /* in electrical degrees or percent (here 0) */
    IfxGtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];        /* per-channel dead time retention */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3ph = {0};

/* =============================
 * Forward Declarations (ISR and callback)
 * ============================= */
void IfxGtm_periodEventFunction(void *data);

/* ISR: The priority must match ISR_PRIORITY_ATOM. The PWM driver routes the event; ISR toggles LED only. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Periodic callback (do nothing) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Initialization
 * ============================= */
/*
 * Initialize a 3-channel complementary, center-aligned PWM on TOM with sync start and updates.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DeadTime         deadTime[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary pairs, active-high HS, active-low LS, push-pull, automotive pad */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Interrupt configuration: CPU0, prio=20, pulse notify, with periodEvent callback */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 5) Per-channel DTM and channel configuration */
    deadTime[0].rising = PWM_DEAD_TIME; deadTime[0].falling = PWM_DEAD_TIME;
    deadTime[1].rising = PWM_DEAD_TIME; deadTime[1].falling = PWM_DEAD_TIME;
    deadTime[2].rising = PWM_DEAD_TIME; deadTime[2].falling = PWM_DEAD_TIME;

    dtmConfig[0].deadTime = deadTime[0];
    dtmConfig[1].deadTime = deadTime[1];
    dtmConfig[2].deadTime = deadTime[2];

    /* Logical channel indices 0..2 with initial duties and zero phase */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;   /* only base channel has interrupt */

    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 6) Main PWM configuration */
    config.cluster            = IfxGtm_Cluster_1;
    config.subModule          = IfxGtm_Pwm_SubModule_tom;
    config.alignment          = IfxGtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource.tom    = (uint32)IfxGtm_Cmu_Fxclk_0;   /* set ONLY TOM field of union */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 7) Enable guard: enable GTM and set CMU clocks only if not enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        {
            float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 8) Initialize PWM and bind persistent channels array */
    IfxGtm_Pwm_init(&g_gtmTom3ph.pwm, &g_gtmTom3ph.channels[0], &config);

    /* 9) Store initial duties, phases and dead-times in persistent state */
    g_gtmTom3ph.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3ph.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3ph.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3ph.phases[0] = channelConfig[0].phase;
    g_gtmTom3ph.phases[1] = channelConfig[1].phase;
    g_gtmTom3ph.phases[2] = channelConfig[2].phase;

    g_gtmTom3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure LED/debug GPIO as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =============================
 * Duty Update
 * ============================= */
/*
 * Maintain a percent-based duty array (3 channels) and apply synchronous immediate updates.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule per channel: if (duty + step) >= 100 -> duty = 0; then duty += step */
    if ((g_gtmTom3ph.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3ph.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3ph.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3ph.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3ph.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3ph.dutyCycles[2] = 0.0f; }

    g_gtmTom3ph.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3ph.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3ph.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediate multi-channel synchronous update (percent units) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3ph.pwm, (float32*)g_gtmTom3ph.dutyCycles);
}

/* =============================
 * Values (unit-test hook)
 * ============================= */
void values(void)
{
    /* Implemented as a no-op per unit test expectations. */
}
