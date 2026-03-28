/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief Production driver for GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm.
 *
 * Notes:
 * - Follows the unified IfxGtm_Pwm initialization and CMU enable guard patterns from iLLD.
 * - No watchdog API calls appear in this driver per AURIX architecture rules.
 * - Pin routing symbols for TOM/TOUT must be validated for the target device/pinout.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ============================== Configuration Macros ============================== */
#define NUM_OF_CHANNELS                (3u)
#define PWM_FREQUENCY_HZ               (20000.0f)   /* 20 kHz */

#define PHASE_U_DUTY_INIT_PERCENT      (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT      (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT      (75.0f)
#define PHASE_DUTY_STEP_PERCENT        (10.0f)

#define ISR_PRIORITY_ATOM              (20)

/* LED macro (compound: port, pin) for ISR toggle */
#define LED                             &MODULE_P13, 0

/* ============================== Pin Routing Placeholders ============================== */
/*
 * User-requested pins (highest priority):
 *  - U: HS=P02.0, LS=P02.7
 *  - V: HS=P02.1, LS=P02.4
 *  - W: HS=P02.2, LS=P02.5
 *
 * IMPORTANT: Map these to valid IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols for your device/package
 * using the official iLLD PinMap headers. As the validated list is empty in this context,
 * placeholders (NULL_PTR) are used here. Replace NULL_PTR with the correct symbols.
 */
#define PHASE_U_HS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* e.g., &IfxGtm_TOM1_<ch>_TOUT<Pxx_y>_P02_0_OUT */
#define PHASE_U_LS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* e.g., &IfxGtm_TOM1_<chN>_TOUT<Pxx_y>_P02_7_OUT */
#define PHASE_V_HS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* e.g., &IfxGtm_TOM1_<ch>_TOUT<Pxx_y>_P02_1_OUT */
#define PHASE_V_LS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* e.g., &IfxGtm_TOM1_<chN>_TOUT<Pxx_y>_P02_4_OUT */
#define PHASE_W_HS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* e.g., &IfxGtm_TOM1_<ch>_TOUT<Pxx_y>_P02_2_OUT */
#define PHASE_W_LS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR)  /* e.g., &IfxGtm_TOM1_<chN>_TOUT<Pxx_y>_P02_5_OUT */

/* ============================== Module State ============================== */
typedef struct
{
    IfxGtm_Pwm             pwm;                               /* unified PWM driver handle */
    IfxGtm_Pwm_Channel     channels[NUM_OF_CHANNELS];         /* persistent channels array */
    float32                dutyCycles[NUM_OF_CHANNELS];       /* duties in percent */
    float32                phases[NUM_OF_CHANNELS];           /* phases in degrees (0..360) */
    IfxGtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];        /* applied dead-times */
} GtmTom3PhPwm_State;

IFX_STATIC GtmTom3PhPwm_State g_gtmTom3PhPwm;

/* ============================== ISR Declaration and Callback ============================== */
/* ISR declaration (vector installed by iLLD via InterruptConfig priority/provider) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/**
 * @brief GTM period ISR: toggle debug LED.
 * Minimal processing per real-time best practices.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/**
 * @brief Empty no-op period event callback (assigned to InterruptConfig.periodEvent).
 * @param data Unused user data pointer (ignored)
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================== Public API ============================== */
/**
 * @brief Initialize 3-phase center-aligned complementary PWM on TOM1 (Cluster_1).
 *
 * - Uses unified IfxGtm_Pwm driver with complementary HS/LS outputs and 1 us dead-time.
 * - CMU enable guard follows the authoritative iLLD pattern.
 * - Interrupt configuration assigns a period event with CPU0 provider and ISR priority.
 * - LED GPIO for ISR toggle is configured after PWM init.
 */
void initGtmTomPwm(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary pairs, active-high HS and active-low LS */
    /* U phase */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;         /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;          /* LS active low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase */
    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase */
    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: 1.0 us rising and falling for all phases */
    dtmConfig[0].deadTime.rising = 1.0e-6f;  dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;  dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;  dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 4) InterruptConfig: period notify on base channel, CPU0 provider, priority per macro */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;        /* period notification */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;                   /* CPU0 */
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;   /* matches IFX_INTERRUPT arg */
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;        /* no-op callback */
    interruptConfig.dutyEvent   = NULL_PTR;                          /* not used */

    /* 6) Channel configuration for logical channels Ch_0..Ch_2 */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;                  /* base channel gets IRQ */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;                          /* no IRQ on non-base chans */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main PWM configuration */
    /* Cluster TOM1 (Cluster_1) selection must match the target; ensure symbol availability in your iLLD */
    /* config.cluster is left as initialized by initConfig if the exact enum isn't available in this context. */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;                /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM from CMU CLK0 */
    config.syncUpdateEnabled    = TRUE;

    /* 5) GTM enable guard and CMU configuration (authoritative pattern) */
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

    /* 6) Initialize the PWM driver with persistent state objects */
    IfxGtm_Pwm_init(&g_gtmTom3PhPwm.pwm, &g_gtmTom3PhPwm.channels[0], &config);

    /* 7) Initialize persistent state arrays */
    g_gtmTom3PhPwm.dutyCycles[0] = PHASE_U_DUTY_INIT_PERCENT;
    g_gtmTom3PhPwm.dutyCycles[1] = PHASE_V_DUTY_INIT_PERCENT;
    g_gtmTom3PhPwm.dutyCycles[2] = PHASE_W_DUTY_INIT_PERCENT;

    g_gtmTom3PhPwm.phases[0] = 0.0f;
    g_gtmTom3PhPwm.phases[1] = 0.0f;
    g_gtmTom3PhPwm.phases[2] = 0.0f;

    g_gtmTom3PhPwm.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3PhPwm.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3PhPwm.deadTimes[2] = dtmConfig[2].deadTime;

    /* 9) Configure LED GPIO for ISR debug toggle (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Step U/V/W duty cycles by +10% with wrap, then apply immediately.
 *
 * Algorithm (per spec):
 *   if ((duty + step) >= 100) duty = 0; then duty += step;  (repeat for each phase)
 * Applies the updated 3-entry array using IfxGtm_Pwm_updateChannelsDutyImmediate.
 */
void updateGtmTomPwmDutyCycles(void)
{
    /* Wrap checks (no loop, three explicit checks as specified) */
    if ((g_gtmTom3PhPwm.dutyCycles[0] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3PhPwm.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3PhPwm.dutyCycles[1] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3PhPwm.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3PhPwm.dutyCycles[2] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3PhPwm.dutyCycles[2] = 0.0f; }

    /* Unconditional step for each phase */
    g_gtmTom3PhPwm.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    g_gtmTom3PhPwm.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    g_gtmTom3PhPwm.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;

    /* Immediate coherent update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3PhPwm.pwm, (float32*)g_gtmTom3PhPwm.dutyCycles);
}
