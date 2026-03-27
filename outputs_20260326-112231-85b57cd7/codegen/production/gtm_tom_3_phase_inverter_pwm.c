/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for AURIX TC3xx GTM TOM 3-phase inverter using IfxGtm_Pwm
 * - Center-aligned, complementary HS/LS pairs with DTM dead-time
 * - Synchronous start and update
 * - Period callback registered (empty)
 * - LED P13.0 toggled by a GTM ATOM ISR stub (ISR only; timer setup external)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ============================ Macros ============================ */
#define NUM_OF_CHANNELS            (3)
#define PWM_FREQUENCY_HZ           (20000.0f)
#define ISR_PRIORITY_ATOM          (20)

/* Initial duties in percent */
#define PHASE_U_DUTY_PERCENT       (25.0f)
#define PHASE_V_DUTY_PERCENT       (50.0f)
#define PHASE_W_DUTY_PERCENT       (75.0f)
#define DUTY_STEP_PERCENT          (10.0f)

/* LED pin: compound macro (port, pin) */
#define LED                        &MODULE_P13, 0

/*
 * Validated/reference GTM TOM1 Cluster_1 TOUT pin symbols
 * Note: Complementary pairs are defined as HS/LS for unified PWM output[] mapping.
 * These mappings are provided from reference templates and must be verified on the target hardware.
 */
#define PHASE_U_HS                 &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS                 &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS                 &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS                 &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS                 &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS                 &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* ============================ Module State ============================ */
typedef struct
{
    IfxGtm_Pwm           pwm;                                 /* unified PWM driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];           /* persistent channel storage (owned by driver) */
    float32              dutyCycles[NUM_OF_CHANNELS];          /* duty in percent [0..100) */
    float32              phases[NUM_OF_CHANNELS];              /* phase in percent [0..100) */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];          /* dead-times per channel */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_state;

/* ============================ ISR and Callback ============================ */
/* ISR: toggles LED; timer setup is out of scope of this module. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Empty PWM period callback registered with IfxGtm_Pwm interrupt config */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================ Public API ============================ */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Load driver defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Global PWM settings: center-aligned, sync start/update, TOM1 cluster */
    config.cluster            = IfxGtm_Cluster_1;
    config.subModule          = IfxGtm_Pwm_SubModule_tom;
    config.alignment          = IfxGtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.numChannels        = NUM_OF_CHANNELS;
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource.atom   = IfxGtm_Cmu_Fxclk_0;                /* FXCLK0 for TOM */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM clock source */

    /* 4) Complementary outputs and pad settings */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;  /* U high-side */
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;  /* U low-side  */
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;  /* V high-side */
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;  /* V low-side  */
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;  /* W high-side */
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;  /* W low-side  */
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Hardware dead-time: 1 us rising/falling for all channels */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Base-channel interrupt configuration (period callback only) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;  /* empty callback */
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;                /* base channel ISR */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;               /* no ISR on this channel */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;               /* no ISR on this channel */

    /* Link channel array to main config */
    config.channels = &channelConfig[0];

    /* 7) GTM enable and CMU clock configuration (guarded) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize PWM with persistent state storage */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

    /* 9) Store initial duties and dead-times in module state (applied by init) */
    g_state.dutyCycles[0] = channelConfig[0].duty;
    g_state.dutyCycles[1] = channelConfig[1].duty;
    g_state.dutyCycles[2] = channelConfig[2].duty;

    g_state.phases[0] = channelConfig[0].phase;
    g_state.phases[1] = channelConfig[1].phase;
    g_state.phases[2] = channelConfig[2].phase;

    g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) LED GPIO as push-pull output, initial state left as reset default (no explicit low drive here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 11) ATOM0 CH0 2 Hz periodic interrupt configuration is out of scope for this module.
     * The ISR is provided (interruptGtmAtom). Timer/SRC setup must be done externally.
     */
}

void updateGtmTom3phInvDuty(void)
{
    /* Duty stepper: +10% with wrap rule (three explicit sequences, no loop) */
    if ((g_state.dutyCycles[0] + DUTY_STEP_PERCENT) >= 100.0f) { g_state.dutyCycles[0] = 0.0f; }
    if ((g_state.dutyCycles[1] + DUTY_STEP_PERCENT) >= 100.0f) { g_state.dutyCycles[1] = 0.0f; }
    if ((g_state.dutyCycles[2] + DUTY_STEP_PERCENT) >= 100.0f) { g_state.dutyCycles[2] = 0.0f; }

    g_state.dutyCycles[0] += DUTY_STEP_PERCENT;
    g_state.dutyCycles[1] += DUTY_STEP_PERCENT;
    g_state.dutyCycles[2] += DUTY_STEP_PERCENT;

    /* Apply as synchronized immediate update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32 *)g_state.dutyCycles);
}
