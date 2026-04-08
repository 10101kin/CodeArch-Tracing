/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for a 3-phase complementary inverter PWM using the
 * unified IfxGtm_Pwm driver on GTM TOM submodule (TC3xx).
 *
 * Follows mandatory iLLD initialization patterns and migration constraints.
 *
 * - Submodule: TOM
 * - Cluster:   Cluster_1
 * - Alignment: Center-aligned
 * - Frequency: 20 kHz
 * - Clock:     FXCLK0 (config.clockSource.tom)
 * - Complementary pairs per phase with dead-time and min-pulse constraints
 * - Interrupt: Period event on base channel; ISR toggles LED (P13.0)
 *
 * Watchdog policy: Do NOT disable watchdogs here. Only CpuX_Main.c may do that.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD core types and drivers */
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
/* Pin map for TOM TOUT symbols used in OutputConfig */
#include "IfxGtm_PinMap.h"
#include "IfxPort_Pinmap.h"

/* ========================================================
 * Macros and configuration constants (migration values)
 * ======================================================== */

/* Channel count */
#define GTM_TOM_3PH_NUM_CHANNELS      (3u)

/* PWM switching frequency (Hz) */
#define GTM_TOM_3PH_PWM_FREQUENCY     (20000.0f)

/* Interrupt priority for GTM period event/ISR */
#define ISR_PRIORITY_ATOM             (20)

/* Phase initial duty cycles in percent */
#define PHASE_U_INIT_DUTY             (25.0f)
#define PHASE_V_INIT_DUTY             (50.0f)
#define PHASE_W_INIT_DUTY             (75.0f)

/* Duty step increment in percent */
#define PHASE_DUTY_STEP               (10.0f)

/* Dead-time settings (seconds) per SW detailed design (0.5 us) */
#define PWM_DEADTIME_RISING_S         (0.5e-6f)
#define PWM_DEADTIME_FALLING_S        (0.5e-6f)

/* Diagnostic LED on P13.0: compound macro expands to two arguments (port, pin) */
#define LED                           &MODULE_P13, 0

/*
 * User-requested TOUT mappings for TC3xx (validated symbols from PinMap headers):
 *   U: HS=P02.0 (TOM1_0_TOUT0),   LS=P02.7 (TOM1_0N_TOUT7)
 *   V: HS=P02.1 (TOM1_1_TOUT1),   LS=P02.4 (TOM1_12_TOUT4)
 *   W: HS=P02.2 (TOM1_5_TOUT2),   LS=P02.5 (TOM1_13_TOUT5)
 */
#define PHASE_U_HS_TOUT   (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS_TOUT   (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS_TOUT   (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS_TOUT   (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS_TOUT   (&IfxGtm_TOM1_5_TOUT2_P02_2_OUT)
#define PHASE_W_LS_TOUT   (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* ========================================================
 * Module state
 * ======================================================== */

typedef struct
{
    IfxGtm_Pwm            pwm;                                        /* PWM driver handle */
    IfxGtm_Pwm_Channel    channels[GTM_TOM_3PH_NUM_CHANNELS];         /* Persistent channels array */
    float32               dutyCycles[GTM_TOM_3PH_NUM_CHANNELS];       /* Duty in percent */
    float32               phases[GTM_TOM_3PH_NUM_CHANNELS];           /* Phase in seconds */
    IfxGtm_Pwm_DeadTime   deadTimes[GTM_TOM_3PH_NUM_CHANNELS];        /* Dead-time settings */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_tom3phState;                              /* Persistent module state */

/* ========================================================
 * ISR and callback (declared before init as required)
 * ======================================================== */

/* ISR declaration with priority and provider cpu0. Body toggles diagnostic LED only. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback assigned to InterruptConfig.periodEvent. Empty body. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================================================
 * Initialization
 * ======================================================== */

/**
 * Initialize the unified GTM PWM for a 3-phase complementary inverter using the high-level IfxGtm_Pwm driver.
 * - Configures three logical channels (0..2) mapped to phases U, V, W with complementary outputs
 * - Center-aligned 20 kHz switching, FXCLK0 source, sync start/update enabled
 * - Period event interrupt on base channel; ISR toggles P13.0
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;                                   /* main config */
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_3PH_NUM_CHANNELS];  /* per-channel config */
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_3PH_NUM_CHANNELS];         /* output pins */
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_3PH_NUM_CHANNELS];      /* dead-time */
    IfxGtm_Pwm_InterruptConfig  interruptConfig;                          /* base channel interrupt */

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: HS/LS pins, polarity, mode, pad driver */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS_TOUT;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS_TOUT;
    output[0].polarity               = Ifx_ActiveState_high;               /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;                /* LS active low (complementary) */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS_TOUT;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS_TOUT;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS_TOUT;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS_TOUT;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: exact dead-time values; minPulse note (handled internally by driver constraints) */
    dtmConfig[0].deadTime.rising = PWM_DEADTIME_RISING_S;
    dtmConfig[0].deadTime.falling = PWM_DEADTIME_FALLING_S;
    dtmConfig[1].deadTime.rising = PWM_DEADTIME_RISING_S;
    dtmConfig[1].deadTime.falling = PWM_DEADTIME_FALLING_S;
    dtmConfig[2].deadTime.rising = PWM_DEADTIME_RISING_S;
    dtmConfig[2].deadTime.falling = PWM_DEADTIME_FALLING_S;

    /* 5) Interrupt configuration: period-event on base channel only */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;              /* period notification */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;              /* callback stub */
    interruptConfig.dutyEvent   = NULL_PTR;                                 /* not used */

    /* 6) Channel configuration (logical indices 0..2) */
    /* Channel 0 -> Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;                        /* base channel has interrupt */

    /* Channel 1 -> Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;                                 /* no interrupt */

    /* Channel 2 -> Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;                                 /* no interrupt */

    /* 7) Main config (submodule, cluster, alignment, clocks, sync, frequency) */
    config.cluster              = IfxGtm_Cluster_1;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM_3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_3PH_PWM_FREQUENCY;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                        /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 8) Enable guard: enable GTM and CMU clocks only if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_tom3phState.pwm, &g_tom3phState.channels[0], &config);

    /* 10) Store persistent state for runtime updates */
    g_tom3phState.dutyCycles[0] = channelConfig[0].duty;
    g_tom3phState.dutyCycles[1] = channelConfig[1].duty;
    g_tom3phState.dutyCycles[2] = channelConfig[2].duty;

    g_tom3phState.phases[0] = channelConfig[0].phase;
    g_tom3phState.phases[1] = channelConfig[1].phase;
    g_tom3phState.phases[2] = channelConfig[2].phase;

    g_tom3phState.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phState.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure diagnostic LED GPIO (P13.0) as push-pull output */
    IfxPort_setPinModeOutput(&MODULE_P13, 0, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ========================================================
 * Runtime update
 * ======================================================== */

/**
 * Atomically update the three phase duties using the immediate group update API.
 * Wrap rule per migration spec: if (duty + step) >= 100 => duty = 0; then duty += step; (always add after check)
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap checks (no loops per structural rule) */
    if ((g_tom3phState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phState.dutyCycles[0] = 0.0f; }
    if ((g_tom3phState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phState.dutyCycles[1] = 0.0f; }
    if ((g_tom3phState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phState.dutyCycles[2] = 0.0f; }

    /* Increment duties */
    g_tom3phState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply synchronized immediate update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phState.pwm, (float32 *)g_tom3phState.dutyCycles);
}
