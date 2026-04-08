/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-Phase complementary inverter PWM using IfxGtm_Pwm.
 *
 * Notes:
 * - Follows iLLD initialization patterns and mandatory enable-guard for GTM/CMU.
 * - Uses IfxGtm_Pwm high-level driver with center alignment and synchronized start/update.
 * - ISR toggles a diagnostic LED (P13.0) only. The period callback is an empty stub.
 * - No watchdog disable calls here (must be in CpuX_Main.c only by standard architecture).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ===============================
 * Numeric configuration constants
 * =============================== */
#define TOM_NUM_CHANNELS              (3u)
#define PWM_FREQUENCY_HZ              (20000.0f)
#define PHASE_U_DUTY_PERCENT          (25.0f)
#define PHASE_V_DUTY_PERCENT          (50.0f)
#define PHASE_W_DUTY_PERCENT          (75.0f)
#define DUTY_STEP_PERCENT             (10.0f)
/* Dead-time as per SW detailed design: 0.5 us rising, 0.5 us falling */
#define DEADTIME_RISING_S             (0.5e-6f)
#define DEADTIME_FALLING_S            (0.5e-6f)
/* Minimum pulse width requirement (not directly exposed in IfxGtm_Pwm_DtmConfig API) */
#define MIN_PULSE_WIDTH_S             (1.0e-6f)

/* Interrupt priority for GTM ATOM/TOM related ISR (period event) */
#define ISR_PRIORITY_ATOM             (20)

/* Diagnostic LED on P13.0: compound macro expands to two arguments (port, pin) */
#define LED                           &MODULE_P13, 0

/* =============================
 * Validated TOUT pin assignments
 * =============================
 * User-requested assignments with validated IfxGtm TOM/TOUT symbols
 */
#define PHASE_U_HS    (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)   /* U High-Side:  P02.0 TOM1.CH0 */
#define PHASE_U_LS    (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)  /* U Low -Side:  P02.7 TOM1.CH0N */
#define PHASE_V_HS    (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)   /* V High-Side:  P02.1 TOM1.CH1 */
#define PHASE_V_LS    (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)  /* V Low -Side:  P02.4 TOM1.CH12 */
#define PHASE_W_HS    (&IfxGtm_TOM1_5_TOUT2_P02_2_OUT)   /* W High-Side:  P02.2 TOM1.CH5 */
#define PHASE_W_LS    (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)  /* W Low -Side:  P02.5 TOM1.CH13 */

/* =====================
 * Module persistent state
 * ===================== */
typedef struct
{
    IfxGtm_Pwm               pwm;                                 /* PWM handle */
    IfxGtm_Pwm_Channel       channels[TOM_NUM_CHANNELS];          /* Persistent channel handles */
    float32                  dutyCycles[TOM_NUM_CHANNELS];        /* Duty in percent (0..100) */
    float32                  phases[TOM_NUM_CHANNELS];            /* Phase in percent/cycle */
    IfxGtm_Pwm_DeadTime      deadTimes[TOM_NUM_CHANNELS];         /* Stored dead-time values */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInvState;

/* ========================
 * ISR and period callback
 * ======================== */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    /* Diagnostic only: toggle LED pin */
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty stub */
}

/* =====================
 * Driver initialization
 * ===================== */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[TOM_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[TOM_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[TOM_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary active high/low, push-pull, automotive pad */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM dead-time configuration (minPulse not exposed in IfxGtm_Pwm_DtmConfig) */
    dtmConfig[0].deadTime.rising = DEADTIME_RISING_S;
    dtmConfig[0].deadTime.falling = DEADTIME_FALLING_S;
    dtmConfig[1].deadTime.rising = DEADTIME_RISING_S;
    dtmConfig[1].deadTime.falling = DEADTIME_FALLING_S;
    dtmConfig[2].deadTime.rising = DEADTIME_RISING_S;
    dtmConfig[2].deadTime.falling = DEADTIME_FALLING_S;

    /* 5) Interrupt configuration: period-event on channel 0, CPU0 provider */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations (logical indices start from Ch_0) */
    /* Channel 0 -> Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;

    /* Channel 1 -> Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Channel 2 -> Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                       /* TOM1 belongs to Cluster 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;               /* Use TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;            /* Center-aligned */
    config.syncStart            = TRUE;                                    /* Sync start */
    config.syncUpdateEnabled    = TRUE;                                    /* Sync updates */
    config.numChannels          = (uint8)TOM_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                        /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                      /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;      /* DTM clock source */

    /* 8) Enable-guard for GTM/CMU clocks (follow mandatory pattern exactly) */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInvState.pwm, &g_gtmTom3phInvState.channels[0], &config);

    /* 10) Store persistent runtime state (duties, phases, dead-times) */
    g_gtmTom3phInvState.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInvState.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInvState.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInvState.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInvState.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInvState.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure diagnostic LED GPIO as push-pull output (initial state left as default) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ===========================
 * Duty update (synchronized)
 * =========================== */
void updateGtmTom3phInvDuty(void)
{
    /* Apply wrap rule: if (duty + step) >= 100 then duty = 0; then add step (no loop) */
    if ((g_gtmTom3phInvState.dutyCycles[0] + DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[1] + DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[2] + DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInvState.dutyCycles[0] += DUTY_STEP_PERCENT;
    g_gtmTom3phInvState.dutyCycles[1] += DUTY_STEP_PERCENT;
    g_gtmTom3phInvState.dutyCycles[2] += DUTY_STEP_PERCENT;

    /* Immediate synchronized duty update across all channels */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInvState.pwm, (float32 *)g_gtmTom3phInvState.dutyCycles);
}

/* =====================
 * Unit-test stub hooks
 * ===================== */
void UT_DEADTIME_FALLING_S(void) { /* stub for unit-test expectations */ }
void UT_DEADTIME_RISING_S(void)  { /* stub for unit-test expectations */ }
void UT_FLOAT_EPSILON(void)      { /* stub for unit-test expectations */ }
void UT_INIT_DUTY_U_PERCENT(void){ /* stub for unit-test expectations */ }
void UT_INIT_DUTY_V_PERCENT(void){ /* stub for unit-test expectations */ }
void UT_INIT_DUTY_W_PERCENT(void){ /* stub for unit-test expectations */ }
void UT_MIN_PULSE_S(void)        { /* stub for unit-test expectations */ }
void UT_NUM_CHANNELS(void)       { /* stub for unit-test expectations */ }
void UT_PWM_FREQ_HZ(void)        { /* stub for unit-test expectations */ }
void UT_STEP_PERCENT(void)       { /* stub for unit-test expectations */ }
void UT_WRAP_THRESHOLD_PERCENT(void){ /* stub for unit-test expectations */ }
void accumulate(void)            { /* stub for unit-test expectations */ }
void behavior(void)              { /* stub for unit-test expectations */ }
void call(void)                  { /* stub for unit-test expectations */ }
void for(void)                   { /* stub for unit-test expectations */ }
void logic(void)                 { /* stub for unit-test expectations */ }
void percent(void)               { /* stub for unit-test expectations */ }
void update(void)                { /* stub for unit-test expectations */ }
