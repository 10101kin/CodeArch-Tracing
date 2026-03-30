/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief Production driver for GTM TOM 3-phase inverter PWM using IfxGtm_Pwm unified driver
 *
 * Notes:
 * - This module follows the unified IfxGtm_Pwm initialization pattern.
 * - Watchdog disable must NOT be placed here; keep it in Cpu0_Main.c only.
 * - No STM timing logic is implemented here. Scheduling belongs to Cpu0_Main.c.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ==========================================================
 * Configuration macros (from system requirements)
 * ========================================================== */
#define NUM_OF_CHANNELS             (3u)
#define PWM_FREQUENCY_HZ            (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM           (20)

/* Phase initial duty in percent */
#define PHASE_U_DUTY                (25.0f)
#define PHASE_V_DUTY                (50.0f)
#define PHASE_W_DUTY                (75.0f)

/* Duty step in percent */
#define PHASE_DUTY_STEP             (10.0f)

/* LED/debug pin (compound macro: port, pin) */
#define LED                         &MODULE_P13, 0

/* ==========================================================
 * TOUT pin routing placeholders
 *
 * IMPORTANT:
 *  - Pin routing is performed via IfxGtm_Pwm_OutputConfig in this driver.
 *  - Use ONLY validated iLLD PinMap symbols for your device/package.
 *  - Since validated symbols are not provided here, placeholders are set to NULL_PTR.
 *  - Integrators must replace NULL_PTR with valid &IfxGtm_TOM1_x_TOUTy_P02_z_OUT symbols
 *    matching the required pins:
 *      U: P02.0 (high-side), P02.7 (low-side)
 *      V: P02.1 (high-side), P02.4 (low-side)
 *      W: P02.2 (high-side), P02.5 (low-side)
 * ========================================================== */
#define PHASE_U_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_ch_TOUTxx_P02_0_OUT */
#define PHASE_U_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_ch_TOUTxx_P02_7_OUT */
#define PHASE_V_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_ch_TOUTxx_P02_1_OUT */
#define PHASE_V_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_ch_TOUTxx_P02_4_OUT */
#define PHASE_W_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_ch_TOUTxx_P02_2_OUT */
#define PHASE_W_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_ch_TOUTxx_P02_5_OUT */

/* ==========================================================
 * Module state
 * ========================================================== */
typedef struct
{
    IfxGtm_Pwm               pwm;                              /* PWM driver handle */
    IfxGtm_Pwm_Channel       channels[NUM_OF_CHANNELS];        /* Channel handles (must persist) */
    float32                  dutyCycles[NUM_OF_CHANNELS];      /* Duty in percent */
    float32                  phases[NUM_OF_CHANNELS];          /* Phase in radians */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];       /* Dead-time per channel */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInvState;

/* ==========================================================
 * Private ISR and callback
 * ========================================================== */
/* ISR: toggle LED on event. Priority is ISR_PRIORITY_ATOM on CPU0 vector table */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: empty hook assigned via InterruptConfig */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ==========================================================
 * Public API implementation
 * ========================================================== */
/**
 * @brief Initialize 3-phase center-aligned PWM using TOM submodule (TOM1, cluster 1)
 * - 3 complementary pairs (U,V,W)
 * - 20 kHz, 1 us dead-time both edges
 * - Sync start and sync update enabled
 * - Clock source: FXCLK0 (config.clockSource.tom)
 * - GTM enable guard (enable+GCLK+CLK0+FXCLK only if not already enabled)
 * - Period-event callback assigned via InterruptConfig (on base channel index 0)
 * - Persistent state holds PWM handle, channels array, and initial duties [25,50,75]
 * - LED/debug pin configured as output at the end
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary pairs U, V, W */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;      /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;       /* LS active low  */
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

    /* 4) DTM configuration: dead-time of 1 us on both edges for each pair */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Interrupt configuration: attach to base channel (index 0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical channel indices 0..2 (not HW channel numbers) */
    /* Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* Base channel gets interrupt */

    /* Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;         /* Only base channel has interrupt */

    /* Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                /* TOM1 is in cluster 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;        /* Use TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;     /* Center-aligned */
    config.syncStart            = TRUE;                             /* Start all channels in sync */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                 /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;               /* TOM functional clock FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM uses CMU CLK0 domain */
    config.syncUpdateEnabled    = TRUE;                             /* Sync update enable */

    /* 8) Enable guard: enable GTM + clocks only if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM driver with persistent channel storage */
    IfxGtm_Pwm_init(&g_gtmTom3phInvState.pwm, &g_gtmTom3phInvState.channels[0], &config);

    /* 10) Store initial state (duties, phases, dead-times) */
    g_gtmTom3phInvState.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInvState.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInvState.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInvState.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInvState.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInvState.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED/debug GPIO as output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Update U/V/W duties with step-and-wrap pattern, then apply immediately.
 * - If (duty + STEP) >= 100, wrap to 0, then add STEP (resulting in STEP)
 * - Always add STEP after the wrap check
 * - Pass the state's duty array directly to the immediate update API
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap checks (separate if-blocks, then unconditional add) */
    if ((g_gtmTom3phInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInvState.pwm, (float32*)g_gtmTom3phInvState.dutyCycles);
}
