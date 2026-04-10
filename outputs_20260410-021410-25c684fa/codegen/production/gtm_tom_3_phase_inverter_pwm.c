/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for 3-phase complementary center-aligned PWM on GTM TOM
 * Follows iLLD IfxGtm_Pwm initialization patterns and migration requirements.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define TOM3PH_NUM_CHANNELS            (3u)
#define TOM3PH_PWM_FREQUENCY_HZ        (20000.0f)      /* 20 kHz */
#define ISR_PRIORITY_ATOM              (20)            /* Interrupt priority for PWM period event */

#define PHASE_U_DUTY_INIT              (25.0f)
#define PHASE_V_DUTY_INIT              (50.0f)
#define PHASE_W_DUTY_INIT              (75.0f)
#define PHASE_DUTY_STEP                (10.0f)

#define TOM3PH_PWM_DEADTIME_S          (5e-07f)        /* 0.5 us as per reconciled migration values */

/* LED/debug pin (compound macro expands to port, pin) */
#define LED                             (&MODULE_P13), (0u)

/*
 * Pin routing placeholders – replace NULL_PTR with validated TOUT mappings during integration.
 * The high-level driver uses IfxGtm_Pwm_OutputConfig with TOUT mapping symbols from device pinmap headers.
 * User-requested mapping (TBD validation):
 *  - Phase U: P02.0 (HS), P02.7 (LS)
 *  - Phase V: P02.1 (HS), P02.4 (LS)
 *  - Phase W: P02.2 (HS), P02.5 (LS)
 */
#define PHASE_U_HS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_0_TOUT0_P02_0_OUT */
#define PHASE_U_LS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_0N_TOUT7_P02_7_OUT */
#define PHASE_V_HS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_1_TOUT1_P02_1_OUT */
#define PHASE_V_LS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_11_TOUT4_P02_4_OUT */
#define PHASE_W_HS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_ATOM0_7N_TOUT2_P02_2_OUT */
#define PHASE_W_LS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_13_TOUT5_P02_5_OUT */

/* =============================
 * Module State
 * ============================= */
typedef struct
{
    IfxGtm_Pwm               pwm;                                           /* Driver handle */
    IfxGtm_Pwm_Channel       channels[TOM3PH_NUM_CHANNELS];                 /* Persistent channel objects */
    float32                  dutyCycles[TOM3PH_NUM_CHANNELS];               /* Duty in percent */
    float32                  phases[TOM3PH_NUM_CHANNELS];                   /* Phase in percent (0..100) */
    IfxGtm_Pwm_DeadTime      deadTimes[TOM3PH_NUM_CHANNELS];                /* Dead-time per channel */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInvState;

/* =============================
 * ISR and Callback (declared before init)
 * ============================= */
/* ISR: Minimal processing – toggle LED and return */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: empty function body as required */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Public API Implementations
 * ============================= */

/*
 * Initialize a 3-channel complementary, center-aligned PWM on TOM with sync start and updates.
 * Follows mandatory iLLD initialization sequence and enable guard.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary pairs, active high (HS), active low (LS) */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Interrupt configuration (periodic notification on base channel only) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = (IfxGtm_Pwm_callBack)NULL_PTR;

    /* 5) Channel configuration (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    dtmConfig[0].deadTime.rising = TOM3PH_PWM_DEADTIME_S;
    dtmConfig[0].deadTime.falling = TOM3PH_PWM_DEADTIME_S;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = (IfxGtm_Trig_MscOut*)NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    dtmConfig[1].deadTime.rising = TOM3PH_PWM_DEADTIME_S;
    dtmConfig[1].deadTime.falling = TOM3PH_PWM_DEADTIME_S;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = (IfxGtm_Trig_MscOut*)NULL_PTR;
    channelConfig[1].interrupt  = (IfxGtm_Pwm_InterruptConfig*)NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    dtmConfig[2].deadTime.rising = TOM3PH_PWM_DEADTIME_S;
    dtmConfig[2].deadTime.falling = TOM3PH_PWM_DEADTIME_S;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = (IfxGtm_Trig_MscOut*)NULL_PTR;
    channelConfig[2].interrupt  = (IfxGtm_Pwm_InterruptConfig*)NULL_PTR;

    /* 6) Main configuration (TOM, center-aligned, sync start & update, 20 kHz, DTM clock CMU0, FXCLK0) */
    config.cluster              = IfxGtm_Cluster_1;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)TOM3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = TOM3PH_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;      /* Use FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 7) Enable guard: enable GTM and setup CMU clocks if not already enabled */
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

    /* 8) Initialize PWM (bind persistent channels array from state) */
    IfxGtm_Pwm_init(&g_tom3phInvState.pwm, &g_tom3phInvState.channels[0], &config);

    /* 9) Store initial state for later updates */
    g_tom3phInvState.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_tom3phInvState.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_tom3phInvState.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_tom3phInvState.phases[0] = 0.0f;
    g_tom3phInvState.phases[1] = 0.0f;
    g_tom3phInvState.phases[2] = 0.0f;

    g_tom3phInvState.deadTimes[0].rising = TOM3PH_PWM_DEADTIME_S;
    g_tom3phInvState.deadTimes[0].falling = TOM3PH_PWM_DEADTIME_S;
    g_tom3phInvState.deadTimes[1].rising = TOM3PH_PWM_DEADTIME_S;
    g_tom3phInvState.deadTimes[1].falling = TOM3PH_PWM_DEADTIME_S;
    g_tom3phInvState.deadTimes[2].rising = TOM3PH_PWM_DEADTIME_S;
    g_tom3phInvState.deadTimes[2].falling = TOM3PH_PWM_DEADTIME_S;

    /* 10) Configure LED/debug GPIO as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update duty cycles with step and apply synchronous immediate update.
 * Duty values are in percent and wrap at 100% using the mandated rule.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: check then unconditional add (three separate if-blocks) */
    if ((g_tom3phInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInvState.dutyCycles[2] = 0.0f; }

    g_tom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate multi-channel duty update (percent units) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInvState.pwm, (float32*)g_tom3phInvState.dutyCycles);
}

/*
 * ISR function implemented via IFX_INTERRUPT macro above; explicit symbol kept for linker/tests.
 * Body toggles LED only (already defined in macro body).
 */
void interruptGtmAtom(void);

/*
 * Empty function for tests or linkage as per requirements.
 */
void values(void)
{
    /* Intentionally empty: behavior defined by unit tests */
}
