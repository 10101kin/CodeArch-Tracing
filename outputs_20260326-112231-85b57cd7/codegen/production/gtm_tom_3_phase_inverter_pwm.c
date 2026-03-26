/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM-based 3-phase complementary PWM on TC3xx
 * using the unified IfxGtm_Pwm high-level driver.
 *
 * Initialization strictly follows the iLLD patterns provided in the
 * authoritative documentation and the constraints in the SW DETAILED DESIGN.
 *
 * Watchdog: This module does NOT touch any watchdog. Follow AURIX standard:
 *           watchdog control must be in CpuX_Main.c only.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================================================================
 * Configuration macros (from requirements)
 * ======================================================================== */
#define NUM_PWM_CHANNELS               (3)
#define PWM_FREQUENCY_HZ               (20000.0f)   /* 20 kHz */

#define PHASE_U_DUTY_INIT_PERCENT      (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT      (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT      (75.0f)
#define PHASE_DUTY_STEP_PERCENT        (10.0f)

/* Interrupt priority for the debug LED toggle ISR */
#define ISR_PRIORITY_ATOM              (20)

/* LED macro: compound form (port, pin) */
#define LED                            &MODULE_P13, 0

/* ========================================================================
 * TOUT pin routing macros
 * NOTE: Validate these pin symbols against your device/pin package pinmap.
 *       The high-level IfxGtm_Pwm driver routes outputs using these maps.
 *       Complementary convention: HS active high, LS active low.
 * ======================================================================== */
/* Phase U: HS=P02.0, LS=P02.7 (complementary) */
#define PHASE_U_HS   &IfxGtm_TOM1_8_TOUT8_P02_0_OUT
#define PHASE_U_LS   &IfxGtm_TOM1_8N_TOUT7_P02_7_OUT

/* Phase V: HS=P02.1, LS=P02.4 (complementary) */
#define PHASE_V_HS   &IfxGtm_TOM1_10_TOUT10_P02_1_OUT
#define PHASE_V_LS   &IfxGtm_TOM1_10N_TOUT4_P02_4_OUT

/* Phase W: HS=P02.2, LS=P02.5 (complementary) */
#define PHASE_W_HS   &IfxGtm_TOM1_12_TOUT12_P02_2_OUT
#define PHASE_W_LS   &IfxGtm_TOM1_12N_TOUT5_P02_5_OUT

/* ========================================================================
 * Module state (persistent) — MUST be static storage per iLLD pattern
 * ======================================================================== */

typedef struct
{
    IfxGtm_Pwm           pwm;                                 /* driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_PWM_CHANNELS];          /* persistent channel handles */
    float32              dutyCycles[NUM_PWM_CHANNELS];         /* percent 0..100 */
    float32              phases[NUM_PWM_CHANNELS];             /* degrees (unused, init to 0) */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_PWM_CHANNELS];          /* stored dead times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;                  /* module-level state */

/* ========================================================================
 * ISR and period-event callback (separate as mandated)
 * ======================================================================== */

/* ISR: toggles LED — minimal body. Declared with IFX_INTERRUPT macro. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback registered via IfxGtm_Pwm InterruptConfig — EMPTY BODY */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================================================================
 * Public API implementation
 * ======================================================================== */

/**
 * Initialize GTM TOM1-based 3-phase complementary PWM using IfxGtm_Pwm.
 * - Center-aligned 20 kHz
 * - 3 complementary pairs (U,V,W) with 1 us deadtime on rising and falling
 * - Synchronous start and synchronous shadow update enabled
 * - Time base on TOM1; channels assumed in Cluster_1: {8,10,12}
 * - LED P13.0 configured as output and driven low initially
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures as local variables */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_PWM_CHANNELS];

    /* 2) Load defaults for main config */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output configuration (complementary HS/LS) */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;         /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;          /* LS active low  */
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

    /* 2) Dead-time configuration: 1 us on rising and falling for all pairs */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;

    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;

    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 3) Interrupt configuration: pulse-notify, CPU0 provider, priority 20, empty period callback */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = &IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (base channel uses interruptConfig) */
    /* Phase U on TOM1 Ch8 */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_8;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    /* Phase V on TOM1 Ch10 */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_10;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;  /* single base interrupt */

    /* Phase W on TOM1 Ch12 */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_12;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 4) Complete the main config */
    config.cluster              = IfxGtm_Cluster_1;                      /* Cluster_1 per requirements */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;              /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;           /* center-aligned */
    config.syncStart            = TRUE;                                   /* synchronous start */
    config.syncUpdateEnabled    = TRUE;                                   /* synchronous shadow update */
    config.numChannels          = NUM_PWM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                       /* 20 kHz */
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;                     /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;       /* DTM from CMU Clock0 */

    /* 5) GTM enable guard and CMU clocking (inside guard) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 frequency = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, frequency);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, frequency);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize the PWM with persistent handle and channels[] */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 10/7) Store initial duties, phases, and dead-times into persistent state */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure LED GPIO output and drive it low initially */
    IfxPort_setPinModeOutput(&MODULE_P13, 0, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(&MODULE_P13, 0);
}

/**
 * Update PWM duties with +10% step and wrap rule, then apply immediately and synchronously.
 * No timing or delay is performed here; call this from the application loop.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (old + step) >= 100, reset to 0 before adding step */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;

    /* Apply immediately and synchronously across the configured group */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);
}
