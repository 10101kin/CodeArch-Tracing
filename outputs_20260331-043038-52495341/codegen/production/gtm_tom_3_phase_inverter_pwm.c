/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief Production driver: 3-phase complementary PWM on TOM using unified IfxGtm_Pwm
 *
 * - TC3xx GTM/TOM
 * - 3 complementary pairs (U, V, W)
 * - Center-aligned, synchronous start/update
 * - FXCLK0 as TOM clock source
 * - Period-event routed to CPU0 ISR (priority macro)
 *
 * Notes:
 * - Watchdog disable must NOT be placed here (only in CpuX_Main.c per AURIX rules)
 * - No STM timing/delay logic in this driver
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= User-confirmed numerical values (macros) ========================= */
#define TOM_3PH_NUM_CHANNELS          (3u)
#define TOM_PWM_FREQUENCY_HZ          (20000.0f)     /* PWM switching frequency: 20 kHz */
#define PHASE_U_INIT_DUTY             (25.0f)        /* Phase U initial duty [%] */
#define PHASE_V_INIT_DUTY             (50.0f)        /* Phase V initial duty [%] */
#define PHASE_W_INIT_DUTY             (75.0f)        /* Phase W initial duty [%] */
#define PHASE_DUTY_STEP               (10.0f)        /* Duty increment step [%] */
#define DUTY_MIN_PERCENT              (10.0f)        /* Wrap minimum duty [%] */
#define DUTY_MAX_PERCENT              (90.0f)        /* Wrap threshold [%] */
#define PWM_MIN_PULSE_TIME_SEC        (1.0e-6f)      /* Minimum pulse width [s] */
#define PWM_DEAD_TIME_SEC             (1.0e-6f)      /* Complementary dead-time [s] (user-confirmed) */

/* ========================= ISR configuration ========================= */
#define ISR_PRIORITY_ATOM             (10u)

/* LED/debug GPIO: compound macro (port, pin) */
#define LED                           &MODULE_P13, 0

/* ========================= Validated TOM pin assignments (user-requested) ========================= */
/* Phase U: CH2 (HS) / CH1 (LS) → P00.3 / P00.2 */
#define PHASE_U_HS                    (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS                    (&IfxGtm_TOM1_5_TOUT11_P00_2_OUT)
/* Phase V: CH4 (HS) / CH3 (LS) → P00.5 / P00.4 */
#define PHASE_V_HS                    (&IfxGtm_TOM1_1N_TOUT14_P00_5_OUT)
#define PHASE_V_LS                    (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
/* Phase W: CH6 (HS) / CH5 (LS) → P00.7 / P00.6 */
#define PHASE_W_HS                    (&IfxGtm_TOM1_3N_TOUT16_P00_7_OUT)
#define PHASE_W_LS                    (&IfxGtm_TOM1_2N_TOUT15_P00_6_OUT)

/* ========================= Module state ========================= */
typedef struct
{
    IfxGtm_Pwm              pwm;                                   /* unified PWM driver handle */
    IfxGtm_Pwm_Channel      channels[TOM_3PH_NUM_CHANNELS];        /* persistent channels storage */
    float32                 dutyCycles[TOM_3PH_NUM_CHANNELS];      /* duty in percent */
    float32                 phases[TOM_3PH_NUM_CHANNELS];          /* phase in percent/degrees (API units) */
    IfxGtm_Pwm_DeadTime     deadTimes[TOM_3PH_NUM_CHANNELS];       /* applied dead-times */
} GTM_TOM_3Phase_State;

IFX_STATIC GTM_TOM_3Phase_State g_tom3ph_state;

/* ========================= ISR and period-event callback ========================= */
/* ISR declaration at file scope (name and priority per driver-specific knowledge) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/**
 * @brief TOM/ATOM routed ISR: minimal body toggling a debug GPIO.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/**
 * @brief Period event callback assigned via IfxGtm_Pwm_InterruptConfig.
 *        Body intentionally empty; do not toggle GPIO here.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */
/**
 * @brief Initialize a persistent 3-channel complementary PWM on the TOM sub-module using the unified high-level driver.
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Populate defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Per-channel output configuration (HS + complementary LS) */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;          /* HS active-high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;           /* LS active-low  */
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

    /* 4) Dead-time configuration per complementary pair */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_SEC;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_SEC;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_SEC;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_SEC;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_SEC;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_SEC;

    /* 5) Interrupt configuration (period event on base channel) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;  /* pulse notify mode */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;             /* route to CPU0 */
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;  /* callback */
    interruptConfig.dutyEvent   = NULL_PTR;                    /* not used */

    /* 6) Per-channel configuration: logical channels 0..2 */
    /* Base channel 0 → Phase U (interrupt enabled on this one) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    /* Channel 1 → Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2 → Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Complete main configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;             /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;          /* center-aligned */
    config.syncStart            = TRUE;                                  /* synchronized start */
    config.numChannels          = TOM_3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = TOM_PWM_FREQUENCY_HZ;                  /* 20 kHz */
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;            /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;    /* DTM clock source */
    config.syncUpdateEnabled    = TRUE;                                  /* synchronized updates */

    /* 8) Enable-guard and CMU clocks (INSIDE guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM (applies initial duties with sync semantics) */
    IfxGtm_Pwm_init(&g_tom3ph_state.pwm, &g_tom3ph_state.channels[0], &config);

    /* 10) Store initial state for later updates */
    g_tom3ph_state.dutyCycles[0] = channelConfig[0].duty;
    g_tom3ph_state.dutyCycles[1] = channelConfig[1].duty;
    g_tom3ph_state.dutyCycles[2] = channelConfig[2].duty;

    g_tom3ph_state.phases[0] = channelConfig[0].phase;
    g_tom3ph_state.phases[1] = channelConfig[1].phase;
    g_tom3ph_state.phases[2] = channelConfig[2].phase;

    g_tom3ph_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3ph_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3ph_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED/debug GPIO (push-pull) for ISR toggling */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Update the three phase duties in percent and apply them immediately.
 */
void GTM_TOM_3_Phase_Inverter_PWM_updateDuties(void)
{
    /* Update U */
    g_tom3ph_state.dutyCycles[0] += PHASE_DUTY_STEP;
    if (g_tom3ph_state.dutyCycles[0] >= DUTY_MAX_PERCENT)
    {
        g_tom3ph_state.dutyCycles[0] = DUTY_MIN_PERCENT;
    }

    /* Update V */
    g_tom3ph_state.dutyCycles[1] += PHASE_DUTY_STEP;
    if (g_tom3ph_state.dutyCycles[1] >= DUTY_MAX_PERCENT)
    {
        g_tom3ph_state.dutyCycles[1] = DUTY_MIN_PERCENT;
    }

    /* Update W */
    g_tom3ph_state.dutyCycles[2] += PHASE_DUTY_STEP;
    if (g_tom3ph_state.dutyCycles[2] >= DUTY_MAX_PERCENT)
    {
        g_tom3ph_state.dutyCycles[2] = DUTY_MIN_PERCENT;
    }

    /* Apply immediately to all complementary pairs */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3ph_state.pwm, (float32 *)g_tom3ph_state.dutyCycles);
}
