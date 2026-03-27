/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase complementary PWM using IfxGtm_Pwm (TC3xx)
 *
 * - Center-aligned, 20 kHz
 * - Complementary outputs with 1 us hardware dead-time
 * - Sync start and sync update enabled
 * - Cluster: TOM1
 * - LED P13.0 toggled by ATOM ISR stub (priority 20 on CPU0)
 *
 * Notes:
 * - Watchdogs must be handled only in CpuX_Main.c (not here).
 * - All CMU configuration is guarded inside the GTM enable-check block.
 * - Uses only APIs listed in the provided mock signatures.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and constants ========================= */
#define NUM_OF_CHANNELS          (3)
#define PWM_FREQUENCY_HZ         (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM        (20)

/* Initial duties in percent */
#define PHASE_U_DUTY_INIT        (25.0f)
#define PHASE_V_DUTY_INIT        (50.0f)
#define PHASE_W_DUTY_INIT        (75.0f)

/* Duty update step in percent */
#define PHASE_DUTY_STEP          (10.0f)

/* LED: compound macro (port, pin) */
#define LED                      &MODULE_P13, 0

/* GTM TOM1 Cluster_1 complementary outputs on P02.x (verify on target) */
#define PHASE_U_HS               &IfxGtm_TOM1_0_TOUT0_P02_0_OUT
#define PHASE_U_LS               &IfxGtm_TOM1_0N_TOUT7_P02_7_OUT
#define PHASE_V_HS               &IfxGtm_TOM1_1_TOUT1_P02_1_OUT
#define PHASE_V_LS               &IfxGtm_TOM1_1N_TOUT4_P02_4_OUT
#define PHASE_W_HS               &IfxGtm_TOM1_2_TOUT2_P02_2_OUT
#define PHASE_W_LS               &IfxGtm_TOM1_2N_TOUT5_P02_5_OUT

/* ========================= Local state ========================= */

typedef struct
{
    IfxGtm_Pwm           pwm;                                   /* driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];             /* persistent channel objects */
    float32              dutyCycles[NUM_OF_CHANNELS];            /* in percent */
    float32              phases[NUM_OF_CHANNELS];                /* reserved, 0.0f by default */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];            /* stored configured dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_state;                           /* zero-initialized */

/* ========================= ISR and callbacks ========================= */

/* ATOM ISR declaration (priority on CPU0) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/* ISR: minimal body, toggles debug LED */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* PWM period callback (empty by design) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */

/*
 * Initialize 3-phase complementary PWM on TOM1 Cluster_1 using the unified IfxGtm_Pwm driver.
 * - Follows iLLD initialization pattern strictly.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Load driver defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Main PWM settings: center-aligned 20 kHz with sync start/update on TOM1 cluster */
    config.cluster              = IfxGtm_Cluster_1;                     /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;             /* TOM sub-module */
    config.alignment            = IfxGtm_Pwm_Alignment_center;          /* center-aligned */
    config.syncStart            = TRUE;                                  /* synchronized start */
    config.syncUpdateEnabled    = TRUE;                                  /* synchronized updates */
    config.numChannels          = NUM_OF_CHANNELS;
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;                    /* TOM/ATOM FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;     /* DTM clock source */

    /* 4) Output configuration: complementary HS/LS per phase with active-high/active-low polarity */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;             /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;              /* LS active low  */
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

    /* 4) DTM configuration: 1 us dead-time on rising and falling edges */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Base-channel-only interrupt configuration (period event callback) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;  /* empty callback */
    irqCfg.dutyEvent   = NULL_PTR;                    /* not used */

    /* 6) Channel configurations: logical indices 0..2 (driver maps to TOM1 pins) */
    /* Channel 0 (Phase U) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;             /* base channel interrupt */

    /* Channel 1 (Phase V) */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;            /* base-only pattern */

    /* Channel 2 (Phase W) */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;            /* base-only pattern */

    /* Link channel array to main config */
    config.channels = &channelConfig[0];

    /* 7) GTM enable and CMU clock configuration (guarded) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        {
            float32 frequency = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, frequency);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, frequency);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 8) Initialize the PWM with persistent state channels[] */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

    /* 9) Store initial duties, phases, and dead-times in the state (init already applied them) */
    g_state.dutyCycles[0] = channelConfig[0].duty;
    g_state.dutyCycles[1] = channelConfig[1].duty;
    g_state.dutyCycles[2] = channelConfig[2].duty;

    g_state.phases[0] = channelConfig[0].phase;
    g_state.phases[1] = channelConfig[1].phase;
    g_state.phases[2] = channelConfig[2].phase;

    g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure LED GPIO as push-pull output; initial state is low by default */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 11) ATOM0 CH0 2 Hz periodic interrupt setup is not performed here due to API constraints.
     *     The ISR is declared (interruptGtmAtom) and ready to be hooked by the application.
     */
}

/*
 * Duty stepper: +10% with wrap rule per phase, then apply immediate synchronized update.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 -> duty = 0; then add step (always) */
    if ((g_state.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[0] = 0.0f; }
    if ((g_state.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[1] = 0.0f; }
    if ((g_state.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[2] = 0.0f; }

    g_state.dutyCycles[0] += PHASE_DUTY_STEP;
    g_state.dutyCycles[1] += PHASE_DUTY_STEP;
    g_state.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply as a synchronized group using the Immediate API (percent inputs) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32*)g_state.dutyCycles);
}
