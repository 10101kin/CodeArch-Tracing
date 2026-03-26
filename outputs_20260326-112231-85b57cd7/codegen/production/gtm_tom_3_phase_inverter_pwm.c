/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-Phase complementary PWM using IfxGtm_Pwm (TC3xx)
 *
 * Notes:
 * - Follows iLLD IfxGtm_Pwm unified driver initialization pattern exactly (see authoritative references)
 * - No watchdog API used here (must be handled only in CpuX_Main.c)
 * - No STM-based timing in this driver (scheduling belongs to application layer)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ===================== Configuration Macros ===================== */
#define NUM_OF_CHANNELS       (3)
#define PWM_FREQUENCY_HZ      (20000.0f)
#define ISR_PRIORITY_ATOM     (20)

#define PHASE_U_DUTY          (25.0f)
#define PHASE_V_DUTY          (50.0f)
#define PHASE_W_DUTY          (75.0f)
#define DUTY_STEP             (10.0f)

/* LED macro (compound form) */
#define LED                   &MODULE_P13, 0

/*
 * Pin routing macros (use TOM1 Cluster_1 complementary convention with _N suffix for LS)
 * These symbols must exist in the iLLD PinMap for the target device/package.
 */
#define PHASE_U_HS            ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS            ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS            ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS            ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS            ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS            ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_2N_TOUT5_P02_5_OUT)

/* ===================== Module State ===================== */
typedef struct
{
    IfxGtm_Pwm           pwm;                              /* driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];        /* persistent channels storage (driver keeps pointer) */
    float32              dutyCycles[NUM_OF_CHANNELS];       /* percent [0..100) */
    float32              phases[NUM_OF_CHANNELS];           /* degrees or percent-of-period, app-defined (initialized to 0) */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];        /* dead-time settings (rising/falling seconds) */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_state;  /* persistent module state */

/* ===================== ISR and Callback (declared before init) ===================== */
/*
 * Hardware ISR: GTM submodule interrupt (example: ATOM/TOM routed service)
 * Minimal body per best practice: toggle a debug LED.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Period-event callback required by unified PWM driver interrupt configuration.
 * Intentionally does nothing.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ===================== Public API Implementations ===================== */
/**
 * Initialize GTM-based 3-phase complementary PWM using the unified IfxGtm_Pwm high-level driver.
 * Algorithm strictly follows the authoritative iLLD patterns and module design requirements.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all local configuration objects */
    IfxGtm_Pwm_Config          mainConfig;
    IfxGtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig irqCfg;

    /* 2) Initialize defaults */
    IfxGtm_Pwm_initConfig(&mainConfig, &MODULE_GTM);

    /* 3) Output configuration for three complementary HS/LS pairs */
    /* Phase U */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;      /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;       /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: 1 us rising and falling for each pair */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Base channel interrupt configuration (period notify via callback) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2, center-aligned, complementary enabled */
    /* Phase U (channel 0) */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;  /* logical index */
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;               /* percent */
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;                    /* base channel interrupt */

    /* Phase V (channel 1) */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;  /* logical index */
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;                   /* no interrupt */

    /* Phase W (channel 2) */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;  /* logical index */
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;                   /* no interrupt */

    /* 7) Main PWM configuration */
    mainConfig.cluster           = IfxGtm_Cluster_1;                         /* TOM1 Cluster_1 */
    mainConfig.subModule         = IfxGtm_Pwm_SubModule_tom;                 /* TOM submodule */
    mainConfig.alignment         = IfxGtm_Pwm_Alignment_center;              /* center-aligned */
    mainConfig.syncStart         = TRUE;                                     /* synchronous start */
    mainConfig.syncUpdateEnabled = TRUE;                                     /* synchronous updates */
    mainConfig.numChannels       = NUM_OF_CHANNELS;
    mainConfig.channels          = &channelConfig[0];
    mainConfig.frequency         = PWM_FREQUENCY_HZ;                         /* 20 kHz */
    mainConfig.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0;         /* DTM clock */
    mainConfig.clockSource.atom  = IfxGtm_Cmu_Fxclk_0;                       /* FXCLK0 for TOM */

    /* 8) GTM enable guard and CMU clock configuration (inside guard only) */
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

    /* 9) Initialize the PWM driver (driver stores pointer to channels[]; must be persistent) */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &mainConfig);

    /* 10) Persist runtime state: initial duties and dead-times */
    g_state.dutyCycles[0] = PHASE_U_DUTY;
    g_state.dutyCycles[1] = PHASE_V_DUTY;
    g_state.dutyCycles[2] = PHASE_W_DUTY;

    g_state.phases[0] = 0.0f;
    g_state.phases[1] = 0.0f;
    g_state.phases[2] = 0.0f;

    g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output AFTER PWM init (no explicit level drive) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duties in percent using a +10% step with wrap rule.
 * Applies the new duties immediately and synchronously across configured channels.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + STEP) >= 100 -> duty = 0; then always add STEP */
    if ((g_state.dutyCycles[0] + DUTY_STEP) >= 100.0f) { g_state.dutyCycles[0] = 0.0f; }
    if ((g_state.dutyCycles[1] + DUTY_STEP) >= 100.0f) { g_state.dutyCycles[1] = 0.0f; }
    if ((g_state.dutyCycles[2] + DUTY_STEP) >= 100.0f) { g_state.dutyCycles[2] = 0.0f; }

    g_state.dutyCycles[0] += DUTY_STEP;
    g_state.dutyCycles[1] += DUTY_STEP;
    g_state.dutyCycles[2] += DUTY_STEP;

    /* Apply duties immediately (percent input expected) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32 *)g_state.dutyCycles);
}
