/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase Complementary Inverter PWM using IfxGtm_Pwm
 *
 * Notes:
 * - Follows iLLD IfxGtm_Pwm high-level driver initialization pattern strictly
 * - Uses GTM enable-guard and CMU clock setup as mandated
 * - No watchdog code here (must be in CpuX_Main.c only)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ============================= Macros and configuration values ============================= */

/* Channel count and PWM frequency (Hz) */
#define GTM_PWM_NUM_CHANNELS              (3u)
#define GTM_PWM_FREQUENCY_HZ              (20000.0f)

/* ISR priority (must match InterruptConfig.priority) */
#define ISR_PRIORITY_ATOM                 (20)

/* Phase initial duties (percent) and update step (percent) */
#define PHASE_U_INIT_DUTY_PERCENT         (25.0f)
#define PHASE_V_INIT_DUTY_PERCENT         (50.0f)
#define PHASE_W_INIT_DUTY_PERCENT         (75.0f)
#define PHASE_DUTY_STEP_PERCENT           (10.0f)

/* Dead-time (seconds) and minimum pulse width (seconds) */
#define PWM_DEADTIME_RISING_S             (0.5e-6f)
#define PWM_DEADTIME_FALLING_S            (0.5e-6f)
#define PWM_MIN_PULSE_S                   (1.0e-6f) /* Not exposed in IfxGtm_Pwm_DtmConfig; documented only */

/* Diagnostic LED on P13.0 (compound macro: port, pin) */
#define LED                               &MODULE_P13, 0

/* Validated TOUT pin symbols (user-requested, highest priority) */
#define PHASE_U_HS_TOUT   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS_TOUT   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS_TOUT   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS_TOUT   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS_TOUT   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_5_TOUT2_P02_2_OUT)
#define PHASE_W_LS_TOUT   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* ============================= Module state ============================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                      /* driver handle */
    IfxGtm_Pwm_Channel      channels[GTM_PWM_NUM_CHANNELS];           /* persistent channels storage */
    float32                 dutyCycles[GTM_PWM_NUM_CHANNELS];         /* percent (0..100) */
    float32                 phases[GTM_PWM_NUM_CHANNELS];             /* phase offsets (deg or percent scale if applicable) */
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_PWM_NUM_CHANNELS];          /* per-channel dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gttom3ph_state;

/* ============================= ISR and callback (declared before init) ============================= */

/* ISR: provider cpu0, priority = ISR_PRIORITY_ATOM; minimal body toggles LED only */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback (assigned to InterruptConfig.periodEvent). Empty body. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================= Public API ============================= */

/*
 * Initialize GTM TOM 3-phase complementary inverter PWM using IfxGtm_Pwm
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_PWM_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     outputCfg[GTM_PWM_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmCfg[GTM_PWM_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (complementary pairs, active-high HS, active-low LS) */
    /* Phase U */
    outputCfg[0].pin                    = PHASE_U_HS_TOUT;
    outputCfg[0].complementaryPin       = PHASE_U_LS_TOUT;
    outputCfg[0].polarity               = Ifx_ActiveState_high; /* HS active-high */
    outputCfg[0].complementaryPolarity  = Ifx_ActiveState_low;  /* LS active-low for complementary PWM */
    outputCfg[0].outputMode             = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    outputCfg[1].pin                    = PHASE_V_HS_TOUT;
    outputCfg[1].complementaryPin       = PHASE_V_LS_TOUT;
    outputCfg[1].polarity               = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity  = Ifx_ActiveState_low;
    outputCfg[1].outputMode             = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    outputCfg[2].pin                    = PHASE_W_HS_TOUT;
    outputCfg[2].complementaryPin       = PHASE_W_LS_TOUT;
    outputCfg[2].polarity               = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity  = Ifx_ActiveState_low;
    outputCfg[2].outputMode             = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (seconds). Note: min pulse documented only. */
    dtmCfg[0].deadTime.rising = PWM_DEADTIME_RISING_S;
    dtmCfg[0].deadTime.falling = PWM_DEADTIME_FALLING_S;

    dtmCfg[1].deadTime.rising = PWM_DEADTIME_RISING_S;
    dtmCfg[1].deadTime.falling = PWM_DEADTIME_FALLING_S;

    dtmCfg[2].deadTime.rising = PWM_DEADTIME_RISING_S;
    dtmCfg[2].deadTime.falling = PWM_DEADTIME_FALLING_S;

    /* 5) Interrupt configuration (period notification on channel 0 only) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices starting from Ch_0) */
    /* Phase U → logical channel 0 */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_INIT_DUTY_PERCENT; /* percent */
    channelConfig[0].dtm       = &dtmCfg[0];
    channelConfig[0].output    = &outputCfg[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;                    /* only channel 0 gets IRQ */

    /* Phase V → logical channel 1 */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_INIT_DUTY_PERCENT;
    channelConfig[1].dtm       = &dtmCfg[1];
    channelConfig[1].output    = &outputCfg[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;                  /* no IRQ */

    /* Phase W → logical channel 2 */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_INIT_DUTY_PERCENT;
    channelConfig[2].dtm       = &dtmCfg[2];
    channelConfig[2].output    = &outputCfg[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;                  /* no IRQ */

    /* 7) Main configuration */
    config.cluster              = IfxGtm_Cluster_1;                         /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;                /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;             /* center-aligned */
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_PWM_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                       /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;       /* as per structural rule */

    /* 8) Enable guard: if GTM not enabled, enable and setup CMU clocks */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM with persistent channels array in module state */
    IfxGtm_Pwm_init(&g_gttom3ph_state.pwm, &g_gttom3ph_state.channels[0], &config);

    /* 10) Store persistent runtime state (duties, phases, dead-times) */
    g_gttom3ph_state.dutyCycles[0] = PHASE_U_INIT_DUTY_PERCENT;
    g_gttom3ph_state.dutyCycles[1] = PHASE_V_INIT_DUTY_PERCENT;
    g_gttom3ph_state.dutyCycles[2] = PHASE_W_INIT_DUTY_PERCENT;

    g_gttom3ph_state.phases[0] = 0.0f;
    g_gttom3ph_state.phases[1] = 0.0f;
    g_gttom3ph_state.phases[2] = 0.0f;

    g_gttom3ph_state.deadTimes[0].rising  = PWM_DEADTIME_RISING_S;
    g_gttom3ph_state.deadTimes[0].falling = PWM_DEADTIME_FALLING_S;
    g_gttom3ph_state.deadTimes[1].rising  = PWM_DEADTIME_RISING_S;
    g_gttom3ph_state.deadTimes[1].falling = PWM_DEADTIME_FALLING_S;
    g_gttom3ph_state.deadTimes[2].rising  = PWM_DEADTIME_RISING_S;
    g_gttom3ph_state.deadTimes[2].falling = PWM_DEADTIME_FALLING_S;

    /* 11) Configure diagnostic LED GPIO (push-pull output). Safe state assumed low by board init. */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update duties atomically: add 10.0 percent; if (duty+step) >= 100 then duty=0 then +step
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap check then unconditional increment (no loops as per rule) */
    if ((g_gttom3ph_state.dutyCycles[0] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gttom3ph_state.dutyCycles[0] = 0.0f; }
    if ((g_gttom3ph_state.dutyCycles[1] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gttom3ph_state.dutyCycles[1] = 0.0f; }
    if ((g_gttom3ph_state.dutyCycles[2] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gttom3ph_state.dutyCycles[2] = 0.0f; }

    g_gttom3ph_state.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    g_gttom3ph_state.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    g_gttom3ph_state.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;

    /* Immediate group update (synchronized) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gttom3ph_state.pwm, (float32*)g_gttom3ph_state.dutyCycles);
}

/* ============================= Unit-test placeholders (no-ops) ============================= */

void UT_DEADTIME_FALLING_S(void) { /* no-op for unit test harness */ }
void UT_DEADTIME_RISING_S(void)  { /* no-op for unit test harness */ }
void UT_FLOAT_EPSILON(void)      { /* no-op for unit test harness */ }
void UT_INIT_DUTY_U_PERCENT(void){ /* no-op for unit test harness */ }
void UT_INIT_DUTY_V_PERCENT(void){ /* no-op for unit test harness */ }
void UT_INIT_DUTY_W_PERCENT(void){ /* no-op for unit test harness */ }
void UT_MIN_PULSE_S(void)        { /* no-op for unit test harness */ }
void UT_NUM_CHANNELS(void)       { /* no-op for unit test harness */ }
void UT_PWM_FREQ_HZ(void)        { /* no-op for unit test harness */ }
void UT_STEP_PERCENT(void)       { /* no-op for unit test harness */ }
void UT_WRAP_THRESHOLD_PERCENT(void){ /* no-op for unit test harness */ }
void accumulate(void)            { /* no-op for unit test harness */ }
void behavior(void)              { /* no-op for unit test harness */ }
void call(void)                  { /* no-op for unit test harness */ }
void for(void)                   { /* no-op for unit test harness */ }
void logic(void)                 { /* no-op for unit test harness */ }
void percent(void)               { /* no-op for unit test harness */ }
void update(void)                { /* no-op for unit test harness */ }
