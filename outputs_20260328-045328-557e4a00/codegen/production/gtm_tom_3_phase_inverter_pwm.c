/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm API
 *
 * Requirements covered:
 * - Center-aligned, complementary PWM with DTM dead-time = 1 us
 * - TOM1, Cluster_1, 20 kHz, synchronous start and synchronous update
 * - Interrupt: CPU0 provider, priority 20; periodEvent callback stub provided
 * - Immediate duty update API used in update function
 * - GTM enable guard with dynamic CMU frequency configuration
 * - No watchdog handling in this module
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and constants ========================= */
#define NUM_OF_CHANNELS           (3u)
#define PWM_FREQUENCY             (20000.0f) /* 20 kHz */
#define ISR_PRIORITY_ATOM         (20)

/* LED pin (port, pin) compound macro for API calls */
#define LED                       &MODULE_P13, 0

/* Initial duties in percent */
#define PHASE_U_DUTY              (25.0f)
#define PHASE_V_DUTY              (50.0f)
#define PHASE_W_DUTY              (75.0f)

/* Duty step in percent */
#define PHASE_DUTY_STEP           (10.0f)

/*
 * Pin routing macros:
 * Use validated IfxGtm PinMap symbols for TC38x LFBGA516 when available.
 * As no validated TOUT symbols are provided in this context, placeholders are used.
 * Replace NULL placeholders with concrete IfxGtm_TOM1_*_TOUT*_P02_*_OUT symbols during board integration.
 */
#define PHASE_U_HS                ((IfxGtm_Pwm_ToutMap*)0) /* HS: P02.0 -> TOM1 */
#define PHASE_U_LS                ((IfxGtm_Pwm_ToutMap*)0) /* LS: P02.7 -> TOM1 */
#define PHASE_V_HS                ((IfxGtm_Pwm_ToutMap*)0) /* HS: P02.1 -> TOM1 */
#define PHASE_V_LS                ((IfxGtm_Pwm_ToutMap*)0) /* LS: P02.4 -> TOM1 */
#define PHASE_W_HS                ((IfxGtm_Pwm_ToutMap*)0) /* HS: P02.2 -> TOM1 */
#define PHASE_W_LS                ((IfxGtm_Pwm_ToutMap*)0) /* LS: P02.5 -> TOM1 */

/* ========================= Module state ========================= */
typedef struct
{
    IfxGtm_Pwm                 pwm;                                 /* Driver handle */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];           /* Persistent channel handles storage */
    float32                    dutyCycles[NUM_OF_CHANNELS];         /* Duty values in percent (0..100) */
    float32                    phases[NUM_OF_CHANNELS];             /* Phase offsets (fraction of period) */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];          /* Configured dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gta3phInvState;

/* ========================= Private ISR and callback ========================= */
/* ISR declaration (installed via priority and provider configured by driver) */
IFX_INTERRUPT(interruptGtmAtom0Ch0, 0, ISR_PRIORITY_ATOM);

/* Period-event callback for the unified PWM driver (empty by design) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR body: minimal processing per real-time constraints */
void interruptGtmAtom0Ch0(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API ========================= */
/**
 * Initialize GTM TOM 3-phase inverter PWM using IfxGtm_Pwm unified driver.
 * - Center-aligned TOM1 Cluster_1, complementary outputs with 1us DTM
 * - 20 kHz, sync start and sync update
 * - Period event routed to CPU0, priority 20, callback stub provided
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects for setup */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_InterruptConfig   irqCfg;
    IfxGtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];

    /* 2) Initialize main config defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output pin routing and pad configuration (complementary pairs) */
    /* Phase U */
    output[0].pin                    = PHASE_U_HS; /* HS */
    output[0].complementaryPin       = PHASE_U_LS; /* LS */
    output[0].polarity               = Ifx_ActiveState_high;  /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;   /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM/dead-time configuration (FXCLK0 based, 1 us) */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Period interrupt configuration (base channel only) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations (logical indices 0..2) */
    /* Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;         /* percent */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;              /* period event on base channel only */

    /* Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;         /* percent */
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;         /* percent */
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Complete main configuration */
    config.cluster              = IfxGtm_Cluster_1;                    /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;            /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;         /* center-aligned */
    config.syncStart            = TRUE;                                 /* synchronous start */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;              /* 3 complementary pairs */
    config.channels             = &channelConfig[0];                    /* channel configuration list */
    config.frequency            = PWM_FREQUENCY;                        /* 20 kHz */
    config.clockSource          = IfxGtm_Pwm_ClockSource_fxclk0;        /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;     /* DTM clock from CMU CLK0 domain */
    config.syncUpdateEnabled    = TRUE;                                 /* buffered (sync) update */

    /* 8) GTM enable guard with dynamic CMU configuration */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver (persistent handle and channels storage) */
    IfxGtm_Pwm_init(&g_gta3phInvState.pwm, &g_gta3phInvState.channels[0], &config);

    /* 10) Store configured duties, phases, and dead-times in module state */
    g_gta3phInvState.dutyCycles[0] = channelConfig[0].duty;
    g_gta3phInvState.dutyCycles[1] = channelConfig[1].duty;
    g_gta3phInvState.dutyCycles[2] = channelConfig[2].duty;

    g_gta3phInvState.phases[0] = channelConfig[0].phase;
    g_gta3phInvState.phases[1] = channelConfig[1].phase;
    g_gta3phInvState.phases[2] = channelConfig[2].phase;

    g_gta3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gta3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gta3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure debug LED GPIO as push-pull output (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update duties by +10% with wrap rule and apply immediately to all channels.
 * Rule: if ((duty + step) >= 100) duty = 0; then duty += step (always executed)
 */
void updateGtmTom3phInvDuty(void)
{
    /* Apply wrap rule individually per phase (no loop per requirements) */
    if ((g_gta3phInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gta3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_gta3phInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gta3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_gta3phInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gta3phInvState.dutyCycles[2] = 0.0f; }

    g_gta3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gta3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gta3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate update with percent values (0..100) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gta3phInvState.pwm, (float32*)g_gta3phInvState.dutyCycles);
}
