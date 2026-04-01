/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-Phase complementary PWM using IfxGtm_Pwm
 *
 * Notes:
 * - Follows iLLD IfxGtm_Pwm initialization pattern
 * - TOM1 Cluster_1, center-aligned, sync start and sync updates
 * - Dead-time configured via DTM (CMU Clock0)
 * - Update gating via shared IfxGtm_Tom_Timer
 * - LED toggle in ISR only
 *
 * Watchdog: DO NOT disable watchdogs here (CpuX_Main.c only)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Configuration Constants ========================= */
#define GTM_TOM3PH_NUM_CHANNELS        (3U)

/* PWM numeric constants (user-confirmed migration values) */
#define PWM_SWITCH_FREQ_HZ             (20000.0f)    /* 20 kHz */
#define PHASE_U_INIT_DUTY_PERCENT      (25.0f)
#define PHASE_V_INIT_DUTY_PERCENT      (50.0f)
#define PHASE_W_INIT_DUTY_PERCENT      (75.0f)
#define PHASE_DUTY_STEP_PERCENT        (10.0f)
#define PWM_DEAD_TIME_S                (5.0e-07f)    /* 0.5 us */
#define PWM_MIN_PULSE_TIME_S           (1.0e-06f)    /* 1.0 us */

/* Interrupt priority (ATOM naming per template; used by PWM period event) */
#define ISR_PRIORITY_ATOM              (20U)

/* LED pin (compound form: port, pin) */
#define LED                            &MODULE_P13, 0

/*
 * PWM TOUT pin symbols:
 * No validated TC3xx TOM1/Cluster_1 TOUTs provided in this context.
 * Leave as NULL_PTR placeholders for integration to replace with valid symbols, e.g.:
 *   &IfxGtm_TOM1_<ch>_TOUT<id>_P02_<n>_OUT
 */
#define PHASE_U_HS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTxx_P02_0_OUT */
#define PHASE_U_LS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTxx_P02_7_OUT */
#define PHASE_V_HS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTxx_P02_1_OUT */
#define PHASE_V_LS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTxx_P02_4_OUT */
#define PHASE_W_HS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTxx_P02_2_OUT */
#define PHASE_W_LS                     (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTxx_P02_5_OUT */

/* ========================= Module State ========================= */
typedef struct
{
    IfxGtm_Pwm               pwm;                                         /* Driver handle */
    IfxGtm_Pwm_Channel       channels[GTM_TOM3PH_NUM_CHANNELS];           /* Persistent channels array */
    float32                  dutyCycles[GTM_TOM3PH_NUM_CHANNELS];         /* Duty in percent */
    float32                  phases[GTM_TOM3PH_NUM_CHANNELS];             /* Phase in percent */
    IfxGtm_Pwm_DeadTime      deadTimes[GTM_TOM3PH_NUM_CHANNELS];          /* Dead-times (s) */
    IfxGtm_Tom_Timer         timer;                                       /* Shared TOM timer for update gating */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv; /* Zero-initialized */

/* ========================= ISR and Callbacks ========================= */
/* ISR toggles LED only; priority provided by ISR_PRIORITY_ATOM */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Empty period-event callback used by IfxGtm_Pwm interrupt configuration */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Local Helpers (none) ========================= */

/* ========================= Public API ========================= */
/*
 * Initialize the GTM-based 3-phase complementary PWM using the unified IfxGtm_Pwm driver
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structs as locals */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Tom_Timer_Config     timerConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for three logical channels (HS and complementary LS) */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;              /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;               /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: 0.5 us rising/falling dead-time, CMU Clock0 for DTM */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_S;

    /* 5) Channel configuration: logical channels 0..2, phase=0, duties=25/50/75 */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_INIT_DUTY_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR;  /* will set after interruptConfig is ready */

    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_INIT_DUTY_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_INIT_DUTY_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 6) Interrupt configuration for base channel period event */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    /* 7) Main PWM config */
    config.cluster              = IfxGtm_Cluster_1;                   /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;           /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;        /* Center-aligned */
    config.syncStart            = TRUE;                                /* Synchronous start */
    config.syncUpdateEnabled    = TRUE;                                /* Synchronous updates */
    config.numChannels          = (uint8)GTM_TOM3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_SWITCH_FREQ_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;          /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;   /* DTM clock from CMU CLK0 */

    /* 8) GTM enable guard + CMU clocks setup (inside guard only) */
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

    /* 9) Initialize PWM driver with persistent channels storage */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* Persist initial state for duties/phases/dead-times */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;  g_gtmTom3phInv.phases[0] = channelConfig[0].phase;  g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;  g_gtmTom3phInv.phases[1] = channelConfig[1].phase;  g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;  g_gtmTom3phInv.phases[2] = channelConfig[2].phase;  g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Initialize shared TOM timer for update gating */
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    /* Note: Timer base/timebase, channel selection, and clock are left at defaults here.
       Integration should select a TOM1 channel (e.g., 7) not colliding with PWM outputs
       and consistent with the PWM period source. */
    (void)IfxGtm_Tom_Timer_init(&g_gtmTom3phInv.timer, &timerConfig);

    /* 11) Configure LED GPIO as push-pull output (initial low) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update three phase duty cycles with wrap and commit via TOM timer update gating
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 -> reset to 0, then always add step */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }
    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;

    /* Issue synchronized update using shared TOM timer gating */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phInv.timer);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phInv.timer);
}
