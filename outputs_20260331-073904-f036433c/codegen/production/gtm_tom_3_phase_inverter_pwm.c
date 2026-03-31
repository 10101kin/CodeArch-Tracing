/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM on TC3xx.
 *
 * Notes:
 * - Follows iLLD IfxGtm_Pwm initialization pattern with GTM enable guard
 * - Center-aligned complementary PWM on TOM, cluster 1
 * - 3 channels (U, V, W). Complementary outputs with deadtime insertion
 * - Interrupt callback stub (period event) provided; ISR toggles LED on P13.0
 * - No watchdog handling here (must be in CpuX_Main.c only)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and constants ========================= */

/* Channel count */
#define TOM_INV_NUM_CHANNELS            (3u)

/* PWM switching frequency (Hz) */
#define TOM_INV_PWM_FREQUENCY_HZ        (20000.0f)

/* ISR priority for GTM ATOM (used by IFX_INTERRUPT and InterruptConfig) */
#define ISR_PRIORITY_ATOM               (20)

/* LED (port, pin) compound macro for ISR toggling */
#define LED                             &MODULE_P13, 0

/* Duty management (percent) */
#define PHASE_U_DUTY_INIT_PCT           (25.0f)
#define PHASE_V_DUTY_INIT_PCT           (50.0f)
#define PHASE_W_DUTY_INIT_PCT           (75.0f)
#define PHASE_DUTY_STEP_PCT             (10.0f)
#define PHASE_DUTY_MIN_PCT              (10.0f)
#define PHASE_DUTY_MAX_PCT              (90.0f)

/* Dead-time between complementary outputs (seconds) - user-confirmed */
#define TOM_INV_DEADTIME_SEC            (5.0e-07f)  /* 0.5 us */

/* TOUT pin mappings (TBD): Replace NULL_PTR with valid IfxGtm_TOM1_x_TOUTy_P02_z_OUT symbols
   once confirmed from IfxGtm_PinMap.h for the target board/package. */
#define PHASE_U_HS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_0_OUT */
#define PHASE_U_LS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_7_OUT */
#define PHASE_V_HS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_1_OUT */
#define PHASE_V_LS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_4_OUT */
#define PHASE_W_HS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_2_OUT */
#define PHASE_W_LS                      (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_5_OUT */

/* ========================= Module state ========================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                        /* PWM driver handle */
    IfxGtm_Pwm_Channel      channels[TOM_INV_NUM_CHANNELS];             /* Persistent channels array */
    float32                 dutyCycles[TOM_INV_NUM_CHANNELS];           /* Duty in percent */
    IfxGtm_Pwm_DeadTime     deadTimes[TOM_INV_NUM_CHANNELS];            /* Deadtime per channel */
    float32                 phases[TOM_INV_NUM_CHANNELS];               /* Phase in percent/deg equivalent */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv = { 0 };

/* ========================= ISR and callback (declared before init) ========================= */

/* ISR declaration with priority macro. Service provider (2nd arg) = CPU0 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/*
 * ISR: Toggle LED (P13.0). Keep minimal processing in ISR.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Empty period-event callback used by high-level PWM driver.
 * Must accept void *data and perform no action.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API implementations ========================= */

/*
 * initGtmTom3phInv
 *
 * Initializes GTM TOM center-aligned complementary PWM (3 channels) on Cluster 1.
 * - Configures complementary outputs and dead-times
 * - Sets 20 kHz frequency, synchronous start and updates
 * - Uses TOM clock source FXCLK0 and DTM clock source (system clock)
 * - Installs period-event callback in InterruptConfig for base channel (ch0)
 * - Leaves ATOM ISR for LED toggle (declared above), routing handled externally by project
 */
void initGtmTom3phInv(void)
{
    /* Local configuration objects */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_ChannelConfig     channelConfig[TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig      output[TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   interruptConfig;

    /* Initialize main PWM config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output configuration (complementary pairs) */
    /* Phase U (index 0) */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS; /* high-side */
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS; /* low-side  */
    output[0].polarity                = Ifx_ActiveState_high;            /* HS active-high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;             /* LS active-low  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (index 1) */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (index 2) */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM (dead-time) configuration for each channel */
    dtmConfig[0].deadTime.rising = TOM_INV_DEADTIME_SEC;
    dtmConfig[0].deadTime.falling = TOM_INV_DEADTIME_SEC;

    dtmConfig[1].deadTime.rising = TOM_INV_DEADTIME_SEC;
    dtmConfig[1].deadTime.falling = TOM_INV_DEADTIME_SEC;

    dtmConfig[2].deadTime.rising = TOM_INV_DEADTIME_SEC;
    dtmConfig[2].deadTime.falling = TOM_INV_DEADTIME_SEC;

    /* Interrupt configuration for base channel */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel configurations (logical indices 0..2) */
    /* Channel 0: Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT_PCT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;    /* base channel gets interrupt */

    /* Channel 1: Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT_PCT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2: Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT_PCT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                    /* Cluster 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;            /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;         /* Center-aligned */
    config.syncStart            = TRUE;                                 /* Sync start */
    config.syncUpdateEnabled    = TRUE;                                 /* Sync updates */
    config.numChannels          = (uint8)TOM_INV_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = TOM_INV_PWM_FREQUENCY_HZ;            /* 20 kHz */
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;          /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;  /* DTM clock source */

    /* GTM enable guard and CMU clock setup */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* Initialize PWM with persistent channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* Store initial state for later updates */
    g_gtmTom3phInv.dutyCycles[0] = PHASE_U_DUTY_INIT_PCT;
    g_gtmTom3phInv.dutyCycles[1] = PHASE_V_DUTY_INIT_PCT;
    g_gtmTom3phInv.dutyCycles[2] = PHASE_W_DUTY_INIT_PCT;

    g_gtmTom3phInv.phases[0] = 0.0f;
    g_gtmTom3phInv.phases[1] = 0.0f;
    g_gtmTom3phInv.phases[2] = 0.0f;

    g_gtmTom3phInv.deadTimes[0].rising  = TOM_INV_DEADTIME_SEC;
    g_gtmTom3phInv.deadTimes[0].falling = TOM_INV_DEADTIME_SEC;
    g_gtmTom3phInv.deadTimes[1].rising  = TOM_INV_DEADTIME_SEC;
    g_gtmTom3phInv.deadTimes[1].falling = TOM_INV_DEADTIME_SEC;
    g_gtmTom3phInv.deadTimes[2].rising  = TOM_INV_DEADTIME_SEC;
    g_gtmTom3phInv.deadTimes[2].falling = TOM_INV_DEADTIME_SEC;

    /* Configure LED pin (P13.0) as push-pull output for ISR toggling */
    IfxPort_setPinModeOutput(&MODULE_P13, 0, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * updateGtmTom3phInvDuty
 *
 * Steps duty cycles by a fixed 10% and wraps to 10% upon reaching/exceeding 90%.
 * Applies the change immediately and synchronously to all channels.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Update duties with wrap to min/max limits as specified */
    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP_PCT;
    if (g_gtmTom3phInv.dutyCycles[0] >= PHASE_DUTY_MAX_PCT)
    {
        g_gtmTom3phInv.dutyCycles[0] = PHASE_DUTY_MIN_PCT;
    }

    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP_PCT;
    if (g_gtmTom3phInv.dutyCycles[1] >= PHASE_DUTY_MAX_PCT)
    {
        g_gtmTom3phInv.dutyCycles[1] = PHASE_DUTY_MIN_PCT;
    }

    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP_PCT;
    if (g_gtmTom3phInv.dutyCycles[2] >= PHASE_DUTY_MAX_PCT)
    {
        g_gtmTom3phInv.dutyCycles[2] = PHASE_DUTY_MIN_PCT;
    }

    /* Apply immediate multi-channel update (percent units) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
