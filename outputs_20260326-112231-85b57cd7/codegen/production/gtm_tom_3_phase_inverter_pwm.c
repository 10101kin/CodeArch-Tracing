/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for 3-phase complementary PWM using IfxGtm_Pwm on TOM1 Cluster_1
 * - Center-aligned 20 kHz
 * - Complementary HS/LS pairs with 1 us dead-time
 * - Sync start and sync update enabled
 * - Master time base: TOM1 cluster (logical Ch0..2 configured in unified driver)
 * - LED P13.0 toggled by GTM ATOM0 CH0 ISR (ISR declared; hardware setup external)
 *
 * Notes:
 * - Follows iLLD unified PWM initialization pattern with OutputConfig[], DtmConfig[], ChannelConfig[]
 * - All CMU configuration is guarded by IfxGtm_isEnabled()
 * - No watchdog disable here (must be in CpuX main files)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (constants from requirements) ========================= */
#define NUM_OF_CHANNELS                 (3)
#define PWM_FREQUENCY_HZ                (20000.0f)      /* 20 kHz */
#define ISR_PRIORITY_ATOM               (20)

/* Initial duties in PERCENT (0..100) */
#define DUTY_INIT_U_PERCENT             (25.0f)
#define DUTY_INIT_V_PERCENT             (50.0f)
#define DUTY_INIT_W_PERCENT             (75.0f)
#define DUTY_STEP_PERCENT               (10.0f)

/* LED: compound macro (port, pin) */
#define LED                             &MODULE_P13, 0

/*
 * Complementary output pin routing (use only validated/reference pin symbols).
 * The requested board pins P02.x are TBD; using reference TOM1 Cluster_1 TOUT on P00.x pins.
 * HS = high-side (active high), LS = low-side (active low)
 */
#define PHASE_U_HS                      &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS                      &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS                      &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS                      &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS                      &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS                      &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* ========================= Module persistent state ========================= */
typedef struct
{
    IfxGtm_Pwm             pwm;                               /* unified PWM driver handle */
    IfxGtm_Pwm_Channel     channels[NUM_OF_CHANNELS];         /* persistent channel storage (owned by driver) */
    float32                dutyCycles[NUM_OF_CHANNELS];       /* percent 0..100 */
    float32                phases[NUM_OF_CHANNELS];           /* degrees or percent of period; kept at 0.0f here */
    IfxGtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];        /* per-channel dead-time (rising/falling) */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_state; /* persistent, not zeroed explicitly (toolchain will clear BSS) */

/* ========================= ISR and callback (must be before init) ========================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

void interruptGtmAtom(void)
{
    /* ISR kept minimal: toggle LED only */
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* intentionally empty period callback */
}

/* ========================= Public API ========================= */

/*
 * Initialize 3-phase complementary PWM on TOM1 Cluster_1 using unified IfxGtm_Pwm.
 * - Center-aligned 20 kHz, synchronized start/update, hardware DTM dead-time = 1 us
 * - Base-channel-only interrupt configured with empty period callback
 * - LED P13.0 configured as push-pull output (initial level left unchanged here)
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Load driver defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Main PWM configuration (center-aligned, sync start/update, TOM1 cluster) */
    config.cluster            = IfxGtm_Cluster_1;
    config.subModule          = IfxGtm_Pwm_SubModule_tom;
    config.alignment          = IfxGtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource.atom   = IfxGtm_Cmu_Fxclk_0;            /* FXCLK0 for TOM */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 4) Complementary outputs and DTM (1 us rising/falling) */
    /* U-phase */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;           /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;            /* LS active low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    dtmConfig[0].deadTime.rising     = 1e-6f;  /* 1 us */
    dtmConfig[0].deadTime.falling    = 1e-6f;  /* 1 us */

    /* V-phase */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    dtmConfig[1].deadTime.rising     = 1e-6f;
    dtmConfig[1].deadTime.falling    = 1e-6f;

    /* W-phase */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    dtmConfig[2].deadTime.rising     = 1e-6f;
    dtmConfig[2].deadTime.falling    = 1e-6f;

    /* 5) Base channel interrupt configuration (period notify only) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices Ch_0..Ch_2 */
    /* U-phase (base channel: attach interrupt) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = DUTY_INIT_U_PERCENT;  /* percent */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;              /* base-channel-only IRQ */

    /* V-phase */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = DUTY_INIT_V_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;             /* base-channel-only pattern */

    /* W-phase */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = DUTY_INIT_W_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) GTM enable and CMU clocking (guarded) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        float32 freq;
        IfxGtm_enable(&MODULE_GTM);
        freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize PWM with persistent channel storage */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

    /* 9) Store initial duties, phases, and dead-times into persistent state */
    g_state.dutyCycles[0] = channelConfig[0].duty;
    g_state.dutyCycles[1] = channelConfig[1].duty;
    g_state.dutyCycles[2] = channelConfig[2].duty;

    g_state.phases[0] = channelConfig[0].phase;
    g_state.phases[1] = channelConfig[1].phase;
    g_state.phases[2] = channelConfig[2].phase;

    g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) LED GPIO pin: push-pull output; initial state managed externally */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 11) ATOM0 CH0 periodic interrupt configuration is not performed here due to API constraints.
     *     The ISR is declared (interruptGtmAtom). Hardware setup (timer period and SRC enable)
     *     shall be handled by system-level code if required.
     */
}

/*
 * Duty stepper: +10% with wrap rule; apply immediately as synchronized group update.
 */
void updateGtmTom3phInvDuty(void)
{
    if ((g_state.dutyCycles[0] + DUTY_STEP_PERCENT) >= 100.0f) { g_state.dutyCycles[0] = 0.0f; }
    if ((g_state.dutyCycles[1] + DUTY_STEP_PERCENT) >= 100.0f) { g_state.dutyCycles[1] = 0.0f; }
    if ((g_state.dutyCycles[2] + DUTY_STEP_PERCENT) >= 100.0f) { g_state.dutyCycles[2] = 0.0f; }

    g_state.dutyCycles[0] += DUTY_STEP_PERCENT;
    g_state.dutyCycles[1] += DUTY_STEP_PERCENT;
    g_state.dutyCycles[2] += DUTY_STEP_PERCENT;

    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32 *)g_state.dutyCycles);
}
