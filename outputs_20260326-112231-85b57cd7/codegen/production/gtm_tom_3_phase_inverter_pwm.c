/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: GTM TOM 3-Phase complementary PWM using IfxGtm_Pwm (TC3xx).
 *
 * Implementation notes:
 *  - Follows iLLD unified PWM init pattern and CMU clock enable guard.
 *  - Uses complementary outputs with hardware DTM dead-time of 1 us.
 *  - Center-aligned 20 kHz, synchronous start/update enabled.
 *  - Persistent channels[] state is maintained as required by the driver.
 *  - No watchdog handling here (must be in CpuX main per AURIX pattern).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =============================
 * Macros (configuration values)
 * ============================= */
#define NUM_OF_CHANNELS             (3)
#define PWM_FREQUENCY_HZ            (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM           (20)

/* LED macro: compound (port, pin) */
#define LED                         &MODULE_P13, 0

/* Initial phase duties in percent and step */
#define PHASE_U_DUTY                (25.0f)
#define PHASE_V_DUTY                (50.0f)
#define PHASE_W_DUTY                (75.0f)
#define PHASE_DUTY_STEP             (10.0f)

/*
 * Pin mapping macros (use only valid iLLD pin symbols)
 * High-side pin:  TOM1 channel N
 * Low-side pin:   TOM1 channel N (negated path, _N_) paired to complementary output
 * Board pins: U(P02.0/P02.7), V(P02.1/P02.4), W(P02.2/P02.5)
 */
#define PHASE_U_HS                  ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                  ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                  ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                  ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS                  ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS                  ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_2N_TOUT5_P02_5_OUT)

/* ======================
 * Module runtime state
 * ====================== */

typedef struct
{
    IfxGtm_Pwm           pwm;                             /* driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];       /* persistent channels array */
    float32              dutyCycles[NUM_OF_CHANNELS];      /* percent (0..100) */
    float32              phases[NUM_OF_CHANNELS];          /* phase offset in degrees/percent as configured */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];       /* stored dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_state;

/* ======================
 * ISR and callbacks
 * ====================== */

/*
 * ISR declaration (priority defined by ISR_PRIORITY_ATOM).
 * Note: Per structural rules, ISR toggles LED. However, only a limited set of
 * IfxPort API calls are available in this build context. Therefore, the ISR body
 * is intentionally left empty to comply with the allowed API list.
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    /* Intentionally empty (no available toggle API in this context) */
}

/*
 * Empty period-event callback assigned to IfxGtm_Pwm interrupt configuration.
 * Must have an empty body — no actions in ISR context.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ======================
 * Public API
 * ====================== */

void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration objects (local) */
    IfxGtm_Pwm_Config          mainConfig;
    IfxGtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig irqCfg;

    /* 2) Initialize defaults */
    IfxGtm_Pwm_initConfig(&mainConfig, &MODULE_GTM);

    /* 3) Output configuration (complementary HS/LS pairs) */
    /* Phase U */
    output[0].pin                   = PHASE_U_HS;          /* High-side */
    output[0].complementaryPin      = PHASE_U_LS;          /* Low-side  */
    output[0].polarity              = Ifx_ActiveState_high;/* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low; /* LS active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = PHASE_V_HS;
    output[1].complementaryPin      = PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = PHASE_W_HS;
    output[2].complementaryPin      = PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: 1 us rising and falling dead-time per pair */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Interrupt configuration: pulse notify on period event, CPU0, prio 20 */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2, center-aligned, complementary enabled */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;  /* percent */
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;       /* base channel interrupt config */

    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;      /* only base channel has IRQ */

    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main configuration */
    mainConfig.cluster            = IfxGtm_Cluster_1;                   /* TOM1 Cluster_1 */
    mainConfig.subModule          = IfxGtm_Pwm_SubModule_tom;           /* TOM submodule */
    mainConfig.alignment          = IfxGtm_Pwm_Alignment_center;        /* center-aligned */
    mainConfig.syncStart          = TRUE;
    mainConfig.syncUpdateEnabled  = TRUE;
    mainConfig.numChannels        = NUM_OF_CHANNELS;
    mainConfig.channels           = &channelConfig[0];
    mainConfig.frequency          = PWM_FREQUENCY_HZ;                   /* 20 kHz */
    mainConfig.clockSource.atom   = IfxGtm_Cmu_Fxclk_0;                 /* FXCLK0 */
    mainConfig.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;   /* DTM clock */

    /* 8) GTM enable guard + CMU clocking (all inside guard) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver (persistent channels array passed) */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &mainConfig);

    /* 10) Persist runtime state: duties, phases, dead-times */
    g_state.dutyCycles[0] = channelConfig[0].duty;
    g_state.dutyCycles[1] = channelConfig[1].duty;
    g_state.dutyCycles[2] = channelConfig[2].duty;

    g_state.phases[0] = channelConfig[0].phase;
    g_state.phases[1] = channelConfig[1].phase;
    g_state.phases[2] = channelConfig[2].phase;

    g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO AFTER PWM init (keep default state) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateGtmTom3phInvDuty(void)
{
    const float32 STEP = PHASE_DUTY_STEP;

    /* Wrap rule: if (duty + STEP) >= 100 -> reset to 0, then always add STEP */
    if ((g_state.dutyCycles[0] + STEP) >= 100.0f) { g_state.dutyCycles[0] = 0.0f; }
    if ((g_state.dutyCycles[1] + STEP) >= 100.0f) { g_state.dutyCycles[1] = 0.0f; }
    if ((g_state.dutyCycles[2] + STEP) >= 100.0f) { g_state.dutyCycles[2] = 0.0f; }

    g_state.dutyCycles[0] += STEP;
    g_state.dutyCycles[1] += STEP;
    g_state.dutyCycles[2] += STEP;

    /* Apply updated duties immediately and synchronously */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32*)g_state.dutyCycles);
}
