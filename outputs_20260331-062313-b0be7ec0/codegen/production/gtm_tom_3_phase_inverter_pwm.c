/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: 3-phase center-aligned PWM on GTM TOM using IfxGtm_Pwm
 *
 * Notes:
 * - Follows iLLD initialization patterns for IfxGtm_Pwm
 * - GTM CMU clocks enabled inside enable-guard
 * - Single-ended HS outputs on TOM1 CH2/4/6 (P00.3 / P00.5 / P00.7)
 * - Dead-time values are configured per migration rules, even if not used
 * - No watchdog handling here (placed only in CpuX_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ========================= Macros and Constants ========================= */

/* Channel count */
#define GTM_TOM_INV_NUM_CHANNELS          (3u)

/* PWM frequency (Hz) */
#define GTM_TOM_INV_PWM_FREQUENCY_HZ      (20000.0f)   /* 20 kHz */

/* Duty cycle initialization (percent) */
#define PHASE_U_DUTY_INIT_PERCENT         (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT         (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT         (75.0f)

/* Duty cycle update step and bounds (percent) */
#define PHASE_DUTY_STEP_PERCENT           (10.0f)
#define PHASE_DUTY_MIN_PERCENT            (10.0f)
#define PHASE_DUTY_MAX_PERCENT            (90.0f)

/* Dead-time (seconds) per migration rule (applied even if complementary disabled) */
#define PWM_DEAD_TIME_S                   (1.0e-6f)

/* Optional minimum pulse width requirement from migration values (not enforced here) */
#define PWM_MIN_PULSE_TIME_S              (1.0e-6f)

/* LED macro (port, pin) for optional debug; configured after PWM init */
#define LED                               &MODULE_P13, 0u

/* Validated TOM1 HS pins (preserved mapping) */
#define PHASE_U_HS                        (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_V_HS                        (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_W_HS                        (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                      /* PWM driver handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM_INV_NUM_CHANNELS];       /* Persistent channels array */
    float32                 dutyCycles[GTM_TOM_INV_NUM_CHANNELS];     /* Duty in percent */
    float32                 phases[GTM_TOM_INV_NUM_CHANNELS];         /* Phase in percent (0..100) */
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM_INV_NUM_CHANNELS];      /* Stored dead-time values */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInvState;

/* ========================= Public API Implementations ========================= */

/**
 * Initialize 3-phase center-aligned PWM using IfxGtm_Pwm on TOM submodule.
 * - Configures TOM1 HS outputs: U = P00.3 (TOM1 CH2), V = P00.5 (CH4), W = P00.7 (CH6)
 * - Center-aligned, synchronous start and synchronous update enabled
 * - Frequency = 20 kHz
 * - Initial duties: U=25%, V=50%, W=75%
 * - Dead-time configured to 1us per migration rule (not used without complementary outputs)
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures as locals */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_INV_NUM_CHANNELS];

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: bind to TOM1 HS pins, single-ended mode */
    /* Phase U (index 0) */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = NULL_PTR;                            /* single-ended */
    output[0].polarity               = Ifx_ActiveState_high;                /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;                 /* LS active low (unused) */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (index 1) */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = NULL_PTR;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (index 2) */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = NULL_PTR;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (per migration rule) */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_S;

    /* 5) Channel configuration (logical indices 0..2) */
    /* U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = NULL_PTR;    /* no ISR used */

    /* V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 6) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM_INV_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_INV_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;   /* TOM uses FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;

    /* 7) Enable-guard for GTM and CMU clocks */
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

    /* 8) Initialize PWM with persistent handle and channels array in module state */
    IfxGtm_Pwm_init(&g_gtmTom3phInvState.pwm, &g_gtmTom3phInvState.channels[0], &config);

    /* 9) Store persistent state for runtime updates */
    g_gtmTom3phInvState.dutyCycles[0] = PHASE_U_DUTY_INIT_PERCENT;
    g_gtmTom3phInvState.dutyCycles[1] = PHASE_V_DUTY_INIT_PERCENT;
    g_gtmTom3phInvState.dutyCycles[2] = PHASE_W_DUTY_INIT_PERCENT;

    g_gtmTom3phInvState.phases[0] = 0.0f;
    g_gtmTom3phInvState.phases[1] = 0.0f;
    g_gtmTom3phInvState.phases[2] = 0.0f;

    g_gtmTom3phInvState.deadTimes[0].rising  = dtmConfig[0].deadTime.rising;
    g_gtmTom3phInvState.deadTimes[0].falling = dtmConfig[0].deadTime.falling;
    g_gtmTom3phInvState.deadTimes[1].rising  = dtmConfig[1].deadTime.rising;
    g_gtmTom3phInvState.deadTimes[1].falling = dtmConfig[1].deadTime.falling;
    g_gtmTom3phInvState.deadTimes[2].rising  = dtmConfig[2].deadTime.rising;
    g_gtmTom3phInvState.deadTimes[2].falling = dtmConfig[2].deadTime.falling;

    /* 10) Configure LED pin as push-pull output (debug) after PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update duty cycles of three phases and apply synchronously in one shadow transfer.
 * Algorithm per design:
 *  - For each phase (U,V,W): add +10.0 percent
 *  - If the new value becomes >= 90.0 percent, wrap to 10.0 percent
 *  - Apply with IfxGtm_Pwm_updateChannelsDutyImmediate
 */
void updateGtmTom3phInvDuty(void)
{
    /* Phase U */
    g_gtmTom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    if (g_gtmTom3phInvState.dutyCycles[0] >= PHASE_DUTY_MAX_PERCENT)
    {
        g_gtmTom3phInvState.dutyCycles[0] = PHASE_DUTY_MIN_PERCENT;
    }

    /* Phase V */
    g_gtmTom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    if (g_gtmTom3phInvState.dutyCycles[1] >= PHASE_DUTY_MAX_PERCENT)
    {
        g_gtmTom3phInvState.dutyCycles[1] = PHASE_DUTY_MIN_PERCENT;
    }

    /* Phase W */
    g_gtmTom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;
    if (g_gtmTom3phInvState.dutyCycles[2] >= PHASE_DUTY_MAX_PERCENT)
    {
        g_gtmTom3phInvState.dutyCycles[2] = PHASE_DUTY_MIN_PERCENT;
    }

    /* Apply immediate multi-channel update (percent units) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInvState.pwm, (float32 *)g_gtmTom3phInvState.dutyCycles);
}
