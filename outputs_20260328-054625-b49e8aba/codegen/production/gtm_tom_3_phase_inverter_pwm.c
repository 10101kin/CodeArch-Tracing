/**
 * GTM TOM 3-Phase Inverter PWM driver (IfxGtm_Pwm-based)
 * - TC3xx family
 * - Migration target: IfxGtm_Tom_PwmHl -> IfxGtm_Pwm
 *
 * Implementation notes (authoritative patterns applied):
 * - Uses IfxGtm_Pwm unified driver with TOM submodule
 * - Center-aligned, 3 complementary pairs, sync start/update
 * - 20 kHz PWM, 1 us dead time (rise/fall)
 * - GTM enable guard with CMU configuration inside guard
 * - Output pin routing placeholders (NULL_PTR) must be replaced during board integration
 * - Period-event callback is empty; ISR toggles LED P13.0
 * - No watchdog handling here (must be in CpuX_Main.c only)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define TOM_NUM_CHANNELS            (3U)
#define PWM_FREQUENCY_HZ            (20000.0f)
#define ISR_PRIORITY_ATOM           (20U)

/* Initial duties in percent */
#define PHASE_U_DUTY_INIT_PERCENT   (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT   (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT   (75.0f)
#define PHASE_DUTY_STEP_PERCENT     (10.0f)

/* Cluster selection: TOM1 Cluster_1 (index 1) */
#define GTM_TOM_CLUSTER             ((IfxGtm_Cluster)1)

/* LED/debug pin P13.0 compound macro for port/pin arguments */
#define LED                         &MODULE_P13, 0

/*
 * User-requested pin assignments (Phase U/V/W HS/LS):
 *   U: P02.0 (HS) / P02.7 (LS)
 *   V: P02.1 (HS) / P02.4 (LS)
 *   W: P02.2 (HS) / P02.5 (LS)
 * No validated TOUT symbols provided in the template. Use NULL_PTR placeholders
 * and replace with proper IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols during integration.
 */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_<ch>_TOUT<PIN>_P02_0_OUT */
#define PHASE_U_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_<chN>_TOUT<PIN>_P02_7_OUT */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_<ch>_TOUT<PIN>_P02_1_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_<chN>_TOUT<PIN>_P02_4_OUT */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_<ch>_TOUT<PIN>_P02_2_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_<chN>_TOUT<PIN>_P02_5_OUT */

/* =============================
 * Module State
 * ============================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                  /* Driver handle */
    IfxGtm_Pwm_Channel      channels[TOM_NUM_CHANNELS];           /* Persistent channels array */
    float32                 dutyCycles[TOM_NUM_CHANNELS];         /* Duty in percent */
    float32                 phases[TOM_NUM_CHANNELS];             /* Phase offsets in degrees */
    IfxGtm_Pwm_DeadTime     deadTimes[TOM_NUM_CHANNELS];          /* Dead times (s) */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_tom3ph; /* Persistent module state */

/* =============================
 * ISR and Callback (must appear before init)
 * ============================= */

/* Period-event callback (assigned in InterruptConfig). Must be empty. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Optional helper as specified; not used by driver routing. */
void interruptGtmTomPeriod(void)
{
    IfxPort_togglePin(LED);
}

/* ISR: Routed internally by the high-level PWM driver via InterruptConfig. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* =============================
 * Public API
 * ============================= */

/**
 * Initialize GTM-based 3-phase complementary PWM using IfxGtm_Pwm (TOM submodule).
 *
 * Order of operations strictly follows the iLLD initialization pattern:
 *  1) Declare all configuration structures as locals
 *  2) Fill main and per-channel configuration in memory (no HW touch yet)
 *  3) GTM enable guard + CMU clock setup (inside guard)
 *  4) Initialize PWM with persistent handle and channels array
 *  5) Store initial state (duty, dead-times, phases)
 *  6) Configure LED/debug pin output
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[TOM_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[TOM_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_ChannelConfig    channelConfig[TOM_NUM_CHANNELS];

    /* 2) Initialize main config to defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output configuration for complementary pairs (U, V, W) */
    /* Polarity convention: high-side active HIGH, low-side (complementary) active LOW */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;  /* Phase U HS */
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;  /* Phase U LS */
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;  /* Phase V HS */
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;  /* Phase V LS */
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;  /* Phase W HS */
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;  /* Phase W LS */
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: 1 us rising and falling for each pair */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* Base-channel interrupt configuration (period event) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel configurations (logical indices 0..2) */
    /* Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* Only base channel raises period event */

    /* Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;         /* No IRQ from non-base channels */

    /* Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;         /* No IRQ from non-base channels */

    /* Main PWM configuration */
    config.cluster              = GTM_TOM_CLUSTER;                      /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;             /* Use TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;          /* Center-aligned */
    config.syncStart            = TRUE;                                  /* Synchronous start */
    config.numChannels          = (uint8)TOM_NUM_CHANNELS;              /* 3 complementary pairs */
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                      /* 20 kHz */
    config.clockSource.atom     = (uint32)IfxGtm_Cmu_Fxclk_0;            /* First available FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;    /* DTM clock source */
    config.syncUpdateEnabled    = TRUE;                                  /* Synchronous duty updates */

    /* 3) GTM enable guard: enable and configure clocks once */
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

    /* 4) Initialize PWM driver with persistent state (channels array in g_tom3ph) */
    IfxGtm_Pwm_init(&g_tom3ph.pwm, &g_tom3ph.channels[0], &config);

    /* 5) Store initial state for future updates */
    g_tom3ph.dutyCycles[0] = PHASE_U_DUTY_INIT_PERCENT;
    g_tom3ph.dutyCycles[1] = PHASE_V_DUTY_INIT_PERCENT;
    g_tom3ph.dutyCycles[2] = PHASE_W_DUTY_INIT_PERCENT;

    g_tom3ph.phases[0] = 0.0f;
    g_tom3ph.phases[1] = 0.0f;
    g_tom3ph.phases[2] = 0.0f;

    g_tom3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* 6) Configure LED/debug pin for ISR toggle */
    IfxPort_setPinModeOutput(&MODULE_P13, 0, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Note: Synchronous start is handled by config.syncStart = TRUE in the driver. */
}

/**
 * Update U/V/W duties using stepped pattern and apply atomically to all channels.
 * Wrap rule: if (duty + step) >= 100 then duty := 0; then duty += step;
 */
void updateGtmTom3phInvDuty(void)
{
    if ((g_tom3ph.dutyCycles[0] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_tom3ph.dutyCycles[0] = 0.0f; }
    if ((g_tom3ph.dutyCycles[1] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_tom3ph.dutyCycles[1] = 0.0f; }
    if ((g_tom3ph.dutyCycles[2] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_tom3ph.dutyCycles[2] = 0.0f; }

    g_tom3ph.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    g_tom3ph.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    g_tom3ph.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;

    /* Apply new duties synchronously in one transaction */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3ph.pwm, (float32 *)g_tom3ph.dutyCycles);
}
