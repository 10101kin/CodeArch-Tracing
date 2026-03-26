/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production-ready GTM TOM 3-Phase Inverter PWM driver for AURIX TC3xx.
 * Uses unified IfxGtm_Pwm high-level driver on TOM1 Cluster_1, center-aligned,
 * complementary HS/LS pairs with 1 us deadtime, sync start/update enabled.
 *
 * Mandatory patterns and constraints:
 * - Follow iLLD config-init-init sequence exactly.
 * - Use OutputConfig arrays and DTM config arrays.
 * - GTM enable guard + CMU clock configuration inside the guard block.
 * - Persistent channels[] array in module state.
 * - No watchdog code in this driver.
 * - No STM timing logic here.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ======================== Configuration Macros ======================== */
#define NUM_OF_CHANNELS                 (3)
#define PWM_FREQUENCY_HZ                (20000.0f)   /* 20 kHz */

#define PHASE_U_DUTY_INIT_PERCENT       (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT       (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT       (75.0f)
#define PHASE_DUTY_STEP_PERCENT         (10.0f)

#define ISR_PRIORITY_ATOM               (20)

/* LED macro: compound form (port, pin) */
#define LED                             &MODULE_P13, 0

/* ======================== Pin Routing Macros (Validated Convention) ======================== */
/*
 * Complementary pair routing on TOM1 Cluster_1, using P02.x pins per target requirements.
 * High-side = active HIGH on TOUTx; Low-side (complementary) = active LOW on TOUTy with _N_ suffix.
 * Note: Use only pin symbols provided by iLLD PinMap (convention shown below is valid for TC3xx TOM1/P02).
 */
#define PHASE_U_HS                      (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                      (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)

#define PHASE_V_HS                      (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                      (&IfxGtm_TOM1_1N_TOUT4_P02_4_OUT)

#define PHASE_W_HS                      (&IfxGtm_TOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS                      (&IfxGtm_TOM1_2N_TOUT5_P02_5_OUT)

/* ======================== Module State ======================== */
typedef struct
{
    IfxGtm_Pwm           pwm;                              /* driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];        /* persistent channel storage */
    float32              dutyCycles[NUM_OF_CHANNELS];      /* percent [0..100) */
    float32              phases[NUM_OF_CHANNELS];          /* degrees or percent phase as used by driver (initialized to 0) */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];       /* stored dead times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_state;                     /* module-level persistent state */

/* ======================== ISR and Callback ======================== */
/*
 * ISR: GTM submodule interrupt handler to toggle LED at 2 Hz (source configured externally).
 * Priority is defined by ISR_PRIORITY_ATOM macro. Keep ISR minimal.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Period callback for unified PWM driver interrupt configuration.
 * Intentionally empty. Must be visible to linker (not static).
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ======================== Public API Implementations ======================== */
/**
 * Initialize GTM-based 3-phase complementary PWM using unified IfxGtm_Pwm driver.
 * - TOM1 Cluster_1, center-aligned, complementary HS/LS pairs with 1 us deadtime.
 * - Sync start and sync update enabled.
 * - Initial duties: U=25%, V=50%, W=75%.
 * - Period-event callback registered on channel 0; LED GPIO configured after PWM init.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all local configuration objects */
    IfxGtm_Pwm_Config          config;
    IfxGtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig irqCfg;

    /* 2) Initialize defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: 3 complementary HS/LS pairs, push-pull, pad driver set. */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;     /* HS active HIGH */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;      /* LS active LOW  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: 1 microsecond rising and falling deadtime for each pair */
    dtmConfig[0].deadTime.rising = 1.0e-6f; dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f; dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f; dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 5) Interrupt configuration: period callback on channel 0 only */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2, complementary outputs enabled via output[] */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;                /* Base channel interrupt */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;               /* no ISR on this channel */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;               /* no ISR on this channel */

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                      /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;              /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;           /* center-aligned */
    config.syncStart            = TRUE;                                   /* sync start */
    config.syncUpdateEnabled    = TRUE;                                   /* sync update */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                       /* 20 kHz */
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;                     /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;       /* DTM clock */

    /* 8) GTM enable guard + CMU clock setup (inside guard) */
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

    /* 9) Initialize the PWM (driver stores pointer to persistent g_state.channels[]) */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

    /* 10) Persist initial runtime state */
    g_state.dutyCycles[0] = PHASE_U_DUTY_INIT_PERCENT;
    g_state.dutyCycles[1] = PHASE_V_DUTY_INIT_PERCENT;
    g_state.dutyCycles[2] = PHASE_W_DUTY_INIT_PERCENT;

    g_state.phases[0] = 0.0f;
    g_state.phases[1] = 0.0f;
    g_state.phases[2] = 0.0f;

    g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output AFTER PWM init (no explicit low set here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update three phase duties in +10% steps with wrap rule, then apply immediately.
 * Wrap rule per phase: if (duty + 10) >= 100 then duty = 0; then add 10.
 * Values remain in [0, 100) and are applied synchronously to all channels.
 */
void updateGtmTom3phInvDuty(void)
{
    const float32 STEP = PHASE_DUTY_STEP_PERCENT;

    if ((g_state.dutyCycles[0] + STEP) >= 100.0f) { g_state.dutyCycles[0] = 0.0f; }
    if ((g_state.dutyCycles[1] + STEP) >= 100.0f) { g_state.dutyCycles[1] = 0.0f; }
    if ((g_state.dutyCycles[2] + STEP) >= 100.0f) { g_state.dutyCycles[2] = 0.0f; }

    g_state.dutyCycles[0] += STEP;
    g_state.dutyCycles[1] += STEP;
    g_state.dutyCycles[2] += STEP;

    /* Apply immediately and synchronously */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32 *)g_state.dutyCycles);
}
