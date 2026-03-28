/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief Production driver: 3-Phase complementary PWM using IfxGtm_Pwm on TOM1 Cluster_1 (TC3xx)
 *
 * Design summary:
 * - Unified IfxGtm_Pwm driver
 * - TOM1 Cluster_1, center-aligned @ 20 kHz
 * - Complementary outputs with 1.0 us rising/falling hardware dead time (DTM)
 * - Synchronous start and synchronous updates enabled
 * - Period event routed via InterruptConfig (callback stub provided)
 * - ATOM ISR toggles LED P13.0 (priority 20 on CPU0) — minimal ISR body
 *
 * Notes:
 * - No watchdog handling here (must stay in CpuX_Main.c)
 * - No manual SRC/IRQ installation; the high-level PWM driver routes interrupts via InterruptConfig
 * - No STM/timing logic in this module
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ============================= Configuration macros ============================= */
#define NUM_OF_CHANNELS            (3U)
#define PWM_FREQUENCY_HZ           (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM          (20)           /* CPU0 priority for PWM period event */
#define PHASE_DUTY_STEP            (10.0f)        /* percent step */
#define PHASE_U_DUTY_INIT          (25.0f)        /* percent */
#define PHASE_V_DUTY_INIT          (50.0f)        /* percent */
#define PHASE_W_DUTY_INIT          (75.0f)        /* percent */

/* LED/debug pin: P13.0 (compound macro expands to 2 args: port, pin) */
#define LED                        &MODULE_P13, 0

/* ================================ Pin routing ================================ */
/* Use only validated pin symbols (KIT_A2G_TC387_5V_TFT, TC3xx, LFBGA516) */
#define PHASE_U_HS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* ============================== Module state =============================== */
typedef struct
{
    IfxGtm_Pwm            pwm;                                  /* Driver handle */
    IfxGtm_Pwm_Channel    channels[NUM_OF_CHANNELS];            /* Persistent channel handles (owned by driver) */
    float32               dutyCycles[NUM_OF_CHANNELS];          /* Duty in percent [0..100] for U,V,W */
    float32               phases[NUM_OF_CHANNELS];              /* Phase offsets in [0..1], currently 0 */
    IfxGtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];           /* Effective dead-times per channel */
} GtmTom3phInv_State;

/* IFX_STATIC is mandated by architecture (defined by toolchain headers) */
IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;

/* ========================== ISR and callback stubs ========================== */
/*
 * ATOM ISR stub: priority and provider must match InterruptConfig — ISR toggles LED only.
 * The high-level PWM driver routes the interrupt internally; keep ISR body minimal.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback (empty) assigned via InterruptConfig */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================== Public API ================================ */
/**
 * @brief Initializes a 3-phase complementary PWM on TOM1 Cluster_1.
 *        Center-aligned at 20 kHz, 1 us rising/falling dead-time, sync start/update.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration containers + persistent module state (global) */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output setup: complementary pairs, push-pull, cmosAutomotiveSpeed1,
          HS active-high, LS active-low (complementary) */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM dead-time: 1.0 us (rise/fall) */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* Interrupt configuration: period event on CPU0 with prio 20 */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel configurations (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* Base channel gets interrupt */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 4) Main configuration fields */
    config.cluster              = IfxGtm_Cluster_1;                  /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;          /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;       /* Center-aligned */
    config.syncStart            = TRUE;                               /* Synchronous start */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;            /* 3 complementary pairs */
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                   /* 20 kHz */
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;                 /* Use FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;   /* DTM runs from CMU CLK0 */
    config.syncUpdateEnabled    = TRUE;                               /* Synchronous update */

    /* 5) GTM enable guard and CMU clock setup */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 7) Store initial state (duties, phases, dead-times) */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) LED/debug GPIO init: P13.0 push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Updates the U/V/W duty cycles with step=10 and wrap rule, then applies immediately.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule (three explicit checks; no loops) */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;  /* U */
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;  /* V */
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;  /* W */

    /* Apply immediate coherent update for all complementary pairs */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
