/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for TC3xx GTM TOM 3-Phase Inverter using IfxGtm_Pwm
 * - Center-aligned 20 kHz
 * - Complementary HS/LS outputs with 1 us dead-time
 * - Synchronous start and update
 * - Unified high-level PWM driver (IfxGtm_Pwm)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD includes (source only) */
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/*=========================================================================
 * Configuration Macros (constants only)
 *========================================================================*/
#define NUM_OF_CHANNELS            (3U)
#define PWM_FREQUENCY_HZ           (20000.0f)
#define ISR_PRIORITY_ATOM          (20)

/* Initial duties in percent */
#define PHASE_U_DUTY_INIT_PCT      (25.0f)
#define PHASE_V_DUTY_INIT_PCT      (50.0f)
#define PHASE_W_DUTY_INIT_PCT      (75.0f)
#define PHASE_DUTY_STEP_PCT        (10.0f)

/* LED pin (compound macro: expands to two arguments for IfxPort_*) */
#define LED                        &MODULE_P13, 0

/*
 * PWM output pin macros
 * NOTE: Replace NULL_PTR with validated TOUT symbols for your board/package
 *       e.g., &IfxGtm_TOM1_x_TOUTy_P02_0_OUT from IfxGtm_PinMap_TC38x_516.h
 */
#define PHASE_U_HS   (NULL_PTR)
#define PHASE_U_LS   (NULL_PTR)
#define PHASE_V_HS   (NULL_PTR)
#define PHASE_V_LS   (NULL_PTR)
#define PHASE_W_HS   (NULL_PTR)
#define PHASE_W_LS   (NULL_PTR)

/*=========================================================================
 * Forward declarations: ISR and period-event callback (must precede init)
 *========================================================================*/
IFX_INTERRUPT(interruptGtmAtom0Ch0, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom0Ch0(void)
{
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/*=========================================================================
 * Module State (persistent; IFX_STATIC as required)
 *========================================================================*/
typedef struct
{
    IfxGtm_Pwm                 pwm;                                   /* driver handle */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];             /* persistent channels array */
    float32                    dutyCycles[NUM_OF_CHANNELS];           /* percent 0..100 */
    float32                    phases[NUM_OF_CHANNELS];               /* radians/deg app-defined; unused=0 */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];            /* per-phase dead-time */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gti;

/*=========================================================================
 * Public API
 *========================================================================*/
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structs locally */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   irqCfg;

    /* 2) Populate defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary HS/LS, active-high HS / active-low LS */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;   /* HS */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;    /* LS */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM configuration: 1.0 us rising/falling for each complementary pair */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* Interrupt configuration: period notification on base (first) channel */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* Per-channel configuration: logical channels 0..2, center-aligned duties */
    /* Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT_PCT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;  /* only base channel */

    /* Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT_PCT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT_PCT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Main configuration */
    config.gtmSFR              = &MODULE_GTM;
    config.cluster             = IfxGtm_Cluster_1;                 /* TOM1.TGC1 cluster */
    config.subModule           = IfxGtm_Pwm_SubModule_tom;         /* TOM submodule */
    config.alignment           = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart           = TRUE;                             /* start all synced */
    config.syncUpdateEnabled   = TRUE;                             /* shadow -> active at period */
    config.numChannels         = (uint8)NUM_OF_CHANNELS;
    config.channels            = &channelConfig[0];
    config.frequency           = PWM_FREQUENCY_HZ;
    config.clockSource         = IfxGtm_Pwm_ClockSource_cmuFxclk0; /* FXCLK0 */
    config.dtmClockSource      = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock source */

    /* 5) GTM enable + CMU clock configuration inside guard */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_selectClkInput(&MODULE_GTM, IfxGtm_Cmu_Clk_0, TRUE); /* TOM CLK0 from GCLK */
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gti.pwm, &g_gti.channels[0], &config);

    /* 7) Persist initial duties, phases, and dead-times in state */
    g_gti.dutyCycles[0] = PHASE_U_DUTY_INIT_PCT;
    g_gti.dutyCycles[1] = PHASE_V_DUTY_INIT_PCT;
    g_gti.dutyCycles[2] = PHASE_W_DUTY_INIT_PCT;

    g_gti.phases[0] = 0.0f;
    g_gti.phases[1] = 0.0f;
    g_gti.phases[2] = 0.0f;

    g_gti.deadTimes[0].rising  = dtmConfig[0].deadTime.rising;
    g_gti.deadTimes[0].falling = dtmConfig[0].deadTime.falling;
    g_gti.deadTimes[1].rising  = dtmConfig[1].deadTime.rising;
    g_gti.deadTimes[1].falling = dtmConfig[1].deadTime.falling;
    g_gti.deadTimes[2].rising  = dtmConfig[2].deadTime.rising;
    g_gti.deadTimes[2].falling = dtmConfig[2].deadTime.falling;

    /* 8) Synchronous start handled by config.syncStart = TRUE during init */

    /* 9) LED GPIO: push-pull output, initial level low */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(LED, IfxPort_State_low);
}

void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule in percent; explicit statements per phase (no loop) */
    if ((g_gti.dutyCycles[0] + PHASE_DUTY_STEP_PCT) >= 100.0f) { g_gti.dutyCycles[0] = 0.0f; }
    if ((g_gti.dutyCycles[1] + PHASE_DUTY_STEP_PCT) >= 100.0f) { g_gti.dutyCycles[1] = 0.0f; }
    if ((g_gti.dutyCycles[2] + PHASE_DUTY_STEP_PCT) >= 100.0f) { g_gti.dutyCycles[2] = 0.0f; }
    g_gti.dutyCycles[0] += PHASE_DUTY_STEP_PCT;
    g_gti.dutyCycles[1] += PHASE_DUTY_STEP_PCT;
    g_gti.dutyCycles[2] += PHASE_DUTY_STEP_PCT;

    /* Apply all three duties synchronously in the same cycle */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gti.pwm, (float32*)g_gti.dutyCycles);
}
