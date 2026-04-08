/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for 3-phase inverter using IfxGtm_Pwm (TOM submodule)
 *
 * Notes:
 * - Follows iLLD unified PWM initialization patterns strictly
 * - Uses TOM1, Cluster_1, center-aligned, complementary outputs with dead-time
 * - Clock source: FXCLK0 for TOM
 * - GTM CMU enable performed inside guard only if GTM not already enabled
 * - No watchdog handling here (must be in CpuX_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and configuration constants ========================= */
#define GTM_TOM_3PH_NUM_CHANNELS      (3u)
#define PWM_FREQUENCY_HZ              (20000.0f)
#define ISR_PRIORITY_ATOM             (20)

/* Dead-time and min pulse (seconds) - reconciled migration values */
#define PWM_DEAD_TIME_S               (5.0e-07f)   /* 0.5 us */
#define PWM_MIN_PULSE_S               (1.0e-06f)   /* 1.0 us (not directly configurable in unified driver) */

/* Initial duty cycles in percent */
#define PHASE_U_DUTY_INIT             (25.0f)
#define PHASE_V_DUTY_INIT             (50.0f)
#define PHASE_W_DUTY_INIT             (75.0f)
#define PHASE_DUTY_STEP               (10.0f)

/* LED on P13.0: compound macro providing (port, pin) as two arguments */
#define LED                            &MODULE_P13, 0u

/* Validated TOUT mappings (user-requested pins) */
#define PHASE_U_HS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)   /* U high-side: P02.0 */
#define PHASE_U_LS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)  /* U low-side:  P02.7 */
#define PHASE_V_HS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)   /* V high-side: P02.1 */
#define PHASE_V_LS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)  /* V low-side:  P02.4 */
#define PHASE_W_HS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_5_TOUT2_P02_2_OUT)   /* W high-side: P02.2 */
#define PHASE_W_LS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)  /* W low-side:  P02.5 */

/* ========================= Module persistent state ========================= */
typedef struct
{
    IfxGtm_Pwm              pwm;                               /* unified PWM driver handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM_3PH_NUM_CHANNELS];/* persistent channels array */
    float32                 dutyCycles[GTM_TOM_3PH_NUM_CHANNELS];
    float32                 phases[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM_3PH_NUM_CHANNELS];
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInv; /* zero-initialized by default */

/* ========================= ISR and period-event callback ========================= */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* intentionally empty: required callback, no processing here */
}

/* Hardware ISR: toggles LED only. Priority is ISR_PRIORITY_ATOM. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API implementations ========================= */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary high/low, push-pull, automotive speed 1 */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;  /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;   /* LS active low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: dead-time and min-pulse (min-pulse noted; dead-time applied) */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_S;

    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_S;

    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_S;

    /* 5) Interrupt configuration: CPU0 provider, priority 20, periodic notification */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2, phases, duties, link output/dtm */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* base channel interrupt */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main config: TOM submodule, Cluster_1, center-aligned, 20 kHz, FXCLK0, sync */
    config.cluster              = IfxGtm_Cluster_1;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.numChannels          = (uint8)GTM_TOM_3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;       /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM from CMU CLK0 */
    config.syncUpdateEnabled    = TRUE;

    /* 8) GTM enable guard and CMU setup (inside guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_tom3phInv.pwm, &g_tom3phInv.channels[0], &config);

    /* Persist initial state: duties, phases, and dead-times */
    g_tom3phInv.dutyCycles[0] = channelConfig[0].duty; g_tom3phInv.phases[0] = channelConfig[0].phase; g_tom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInv.dutyCycles[1] = channelConfig[1].duty; g_tom3phInv.phases[1] = channelConfig[1].phase; g_tom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInv.dutyCycles[2] = channelConfig[2].duty; g_tom3phInv.phases[2] = channelConfig[2].phase; g_tom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure debug LED GPIO after PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule per channel (no loops; explicit sequence) */
    if ((g_tom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[2] = 0.0f; }

    g_tom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately to all channels synchronously */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInv.pwm, (float32*)g_tom3phInv.dutyCycles);
}

void duty(void)
{
    /* Unit-test hook: reuse runtime duty update behavior */
    updateGtmTom3phInvDuty();
}
