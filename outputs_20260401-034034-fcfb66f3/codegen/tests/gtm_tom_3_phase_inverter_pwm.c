/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for 3-phase complementary PWM (TOM1, Cluster_1) using IfxGtm_Pwm.
 * - Center-aligned 20 kHz, sync start/updates via shared TOM timer.
 * - Complementary outputs with software-configured dead-time.
 * - Period-event callback assigned; minimal ISR toggles LED.
 *
 * Notes:
 * - Watchdog disable must only be done in CpuX main files (not here).
 * - Pin symbols for TOUT mapping must be validated for the target board.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration Macros (Values)
 * ============================= */
#define GTM_TOM3PH_NUM_CHANNELS        (3U)
#define GTM_TOM3PH_PWM_FREQUENCY_HZ    (20000.0f)     /* 20 kHz */
#define GTM_TOM3PH_DUTY_INIT_U         (25.0f)        /* percent */
#define GTM_TOM3PH_DUTY_INIT_V         (50.0f)        /* percent */
#define GTM_TOM3PH_DUTY_INIT_W         (75.0f)        /* percent */
#define GTM_TOM3PH_DUTY_STEP           (10.0f)        /* percent */
#define GTM_TOM3PH_DUTY_MIN            (10.0f)        /* percent */
#define GTM_TOM3PH_DUTY_MAX            (90.0f)        /* percent */
#define GTM_TOM3PH_DEADTIME_RISE_S     (5e-07f)       /* 0.5 us rising  */
#define GTM_TOM3PH_DEADTIME_FALL_S     (5e-07f)       /* 0.5 us falling */

/* ISR priority for GTM periodic activity (CPU0) */
#define ISR_PRIORITY_ATOM              (20)

/* LED: compound macro used as two arguments (port, pin) */
#define LED                            &MODULE_P13, 0  /* P13.0 */

/* =============================
 * Pin Map Macros (Placeholders)
 * =============================
 * Replace NULL_PTR with validated TOUT symbols from the proper PinMap header
 * for KIT_A2G_TC387_5V_TFT once confirmed. Keep complementary mapping per phase.
 */
#define PHASE_U_HS                     (NULL_PTR) /* TODO: &IfxGtm_TOM1_<HS_CH>_TOUT<PIN>_P02_0_OUT */
#define PHASE_U_LS                     (NULL_PTR) /* TODO: &IfxGtm_TOM1_<LS_CH>_TOUT<PIN>_P02_7_OUT */
#define PHASE_V_HS                     (NULL_PTR) /* TODO: &IfxGtm_TOM1_<HS_CH>_TOUT<PIN>_P02_1_OUT */
#define PHASE_V_LS                     (NULL_PTR) /* TODO: &IfxGtm_TOM1_<LS_CH>_TOUT<PIN>_P02_4_OUT */
#define PHASE_W_HS                     (NULL_PTR) /* TODO: &IfxGtm_TOM1_<HS_CH>_TOUT<PIN>_P02_2_OUT */
#define PHASE_W_LS                     (NULL_PTR) /* TODO: &IfxGtm_TOM1_<LS_CH>_TOUT<PIN>_P02_5_OUT */

/* =============================
 * Module State
 * ============================= */
typedef struct
{
    IfxGtm_Pwm              pwm;                                      /* Driver handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM3PH_NUM_CHANNELS];        /* Persistent channel handles */
    IfxGtm_Tom_Timer        timer;                                    /* Shared TOM timer for update gating */
    float32                 dutyCycles[GTM_TOM3PH_NUM_CHANNELS];      /* Duty in percent */
    float32                 phases[GTM_TOM3PH_NUM_CHANNELS];          /* Phase in percent of period */
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM3PH_NUM_CHANNELS];       /* Rising/Falling dead-time seconds */
} GtmTom3Ph_State;

IFX_STATIC GtmTom3Ph_State g_tom3ph;

/* =============================
 * ISR and Callback (defined before init)
 * ============================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    /* Minimal ISR: toggle LED only */
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Empty period-event callback as required */
}

/* =============================
 * Public API Implementations
 * ============================= */

/**
 * Initialize GTM TOM 3-phase complementary PWM using IfxGtm_Pwm with a shared TOM timer for
 * synchronized start and update gating. Pins are configured via OutputConfig array; dead-time
 * is applied via DTM configuration. LED pin configured as push-pull output for ISR toggle.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Tom_Timer_Config     timerConfig;

    /* 2) Load default PWM config */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for U, V, W (HS + complementary LS) */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;           /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;            /* LS active low  */
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

    /* 4) DTM configuration: 0.5us rising/falling dead-time, DTM clock CMU_CLK0 */
    dtmConfig[0].deadTime.rising = GTM_TOM3PH_DEADTIME_RISE_S;
    dtmConfig[0].deadTime.falling = GTM_TOM3PH_DEADTIME_FALL_S;
    dtmConfig[1].deadTime.rising = GTM_TOM3PH_DEADTIME_RISE_S;
    dtmConfig[1].deadTime.falling = GTM_TOM3PH_DEADTIME_FALL_S;
    dtmConfig[2].deadTime.rising = GTM_TOM3PH_DEADTIME_RISE_S;
    dtmConfig[2].deadTime.falling = GTM_TOM3PH_DEADTIME_FALL_S;

    /* 5) Channel configuration: logical channels 0,1,2 with initial phases/duties */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = GTM_TOM3PH_DUTY_INIT_U;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = NULL_PTR; /* assigned below for ch-0 */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = GTM_TOM3PH_DUTY_INIT_V;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = GTM_TOM3PH_DUTY_INIT_W;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 6) Base-channel interrupt configuration (period event on ch-0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;
    channelConfig[1].interrupt  = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                         /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;                 /* Use TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;              /* Center-aligned (up-down) */
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM3PH_PWM_FREQUENCY_HZ;              /* 20 kHz */
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;               /* TOM uses FXCLK_0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;         /* DTM clock CMU_CLK0 */

    /* 8) GTM enable guard and CMU clocks setup (FXCLK + CLK0) */
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

    /* 9) Initialize PWM (handle + persistent channels) */
    IfxGtm_Pwm_init(&g_tom3ph.pwm, &g_tom3ph.channels[0], &config);

    /* Persist initial duty/phase/dead-time into module state */
    g_tom3ph.dutyCycles[0] = channelConfig[0].duty;
    g_tom3ph.dutyCycles[1] = channelConfig[1].duty;
    g_tom3ph.dutyCycles[2] = channelConfig[2].duty;
    g_tom3ph.phases[0]     = channelConfig[0].phase;
    g_tom3ph.phases[1]     = channelConfig[1].phase;
    g_tom3ph.phases[2]     = channelConfig[2].phase;
    g_tom3ph.deadTimes[0]  = dtmConfig[0].deadTime;
    g_tom3ph.deadTimes[1]  = dtmConfig[1].deadTime;
    g_tom3ph.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 10) Initialize shared TOM timer for update gating */
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    /* Use default config values; ensure timebase/clock selection is consistent with PWM.
       Selection of TOM instance/channel must avoid collision with PWM outputs (to be
       confirmed/adjusted at integration using concrete timerConfig fields). */
    {
        boolean timerOk = IfxGtm_Tom_Timer_init(&g_tom3ph.timer, &timerConfig);
        if (timerOk == FALSE)
        {
            /* Timer init failed; leave function early. PWM remains configured. */
            return;
        }
    }

    /* 11) Configure LED GPIO (push-pull output) for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update three phase duty cycles with wrap and clamp, then apply atomically using
 * TOM timer update gating. No delays/timing here; scheduling is external.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty+step) >= 100 then reset to 0 before adding step */
    if ((g_tom3ph.dutyCycles[0] + GTM_TOM3PH_DUTY_STEP) >= 100.0f) { g_tom3ph.dutyCycles[0] = 0.0f; }
    if ((g_tom3ph.dutyCycles[1] + GTM_TOM3PH_DUTY_STEP) >= 100.0f) { g_tom3ph.dutyCycles[1] = 0.0f; }
    if ((g_tom3ph.dutyCycles[2] + GTM_TOM3PH_DUTY_STEP) >= 100.0f) { g_tom3ph.dutyCycles[2] = 0.0f; }

    g_tom3ph.dutyCycles[0] += GTM_TOM3PH_DUTY_STEP;
    g_tom3ph.dutyCycles[1] += GTM_TOM3PH_DUTY_STEP;
    g_tom3ph.dutyCycles[2] += GTM_TOM3PH_DUTY_STEP;

    /* Clamp to min/max range */
    if (g_tom3ph.dutyCycles[0] < GTM_TOM3PH_DUTY_MIN) { g_tom3ph.dutyCycles[0] = GTM_TOM3PH_DUTY_MIN; }
    if (g_tom3ph.dutyCycles[1] < GTM_TOM3PH_DUTY_MIN) { g_tom3ph.dutyCycles[1] = GTM_TOM3PH_DUTY_MIN; }
    if (g_tom3ph.dutyCycles[2] < GTM_TOM3PH_DUTY_MIN) { g_tom3ph.dutyCycles[2] = GTM_TOM3PH_DUTY_MIN; }

    if (g_tom3ph.dutyCycles[0] > GTM_TOM3PH_DUTY_MAX) { g_tom3ph.dutyCycles[0] = GTM_TOM3PH_DUTY_MAX; }
    if (g_tom3ph.dutyCycles[1] > GTM_TOM3PH_DUTY_MAX) { g_tom3ph.dutyCycles[1] = GTM_TOM3PH_DUTY_MAX; }
    if (g_tom3ph.dutyCycles[2] > GTM_TOM3PH_DUTY_MAX) { g_tom3ph.dutyCycles[2] = GTM_TOM3PH_DUTY_MAX; }

    /* Synchronized commit using TOM timer update gating */
    IfxGtm_Tom_Timer_disableUpdate(&g_tom3ph.timer);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3ph.pwm, (float32*)g_tom3ph.dutyCycles);
    IfxGtm_Tom_Timer_applyUpdate(&g_tom3ph.timer);
}
