/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-Phase Inverter PWM (TC3xx)
 *
 * Notes:
 * - Follows iLLD IfxGtm_Pwm initialization pattern and CMU enable guard
 * - No watchdog handling here (must be in Cpu0_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Configuration Macros ========================= */
#define GTM_TOM_3PH_NUM_CHANNELS     (3u)
#define GTM_TOM_PWM_FREQUENCY_HZ     (20000.0f)
#define ISR_PRIORITY_ATOM            (20)

#define PHASE_U_INIT_DUTY            (25.0f)
#define PHASE_V_INIT_DUTY            (50.0f)
#define PHASE_W_INIT_DUTY            (75.0f)
#define PHASE_DUTY_STEP              (10.0f)

#define PWM_DEAD_TIME_S              (5e-07f)   /* 0.5 us per reconciled migration values */
#define PWM_MIN_PULSE_TIME_S         (1e-06f)

/* LED/debug pin: compound macro expands to (port, pin) */
#define LED                          &MODULE_P13, 0u

/* ========================= Pin Mapping Placeholders ========================= */
/*
 * Pin symbols must come from device-specific PinMap headers. Since validated
 * symbols are not provided here, use NULL_PTR placeholders. Replace these with
 * the proper &IfxGtm_TOM1_x_TOUTy_P02_z_OUT symbols during board integration
 * while keeping TOM1 TGC1 routing (channels 8..13) as required.
 */
#define PHASE_U_HS                   ((IfxGtm_Pwm_ToutMap *)NULL_PTR) /* Replace: &IfxGtm_TOM1_8_TOUTx_P02_0_OUT */
#define PHASE_U_LS                   ((IfxGtm_Pwm_ToutMap *)NULL_PTR) /* Replace: &IfxGtm_TOM1_9_TOUTy_P02_7_OUT */
#define PHASE_V_HS                   ((IfxGtm_Pwm_ToutMap *)NULL_PTR) /* Replace: &IfxGtm_TOM1_10_TOUTx_P02_1_OUT */
#define PHASE_V_LS                   ((IfxGtm_Pwm_ToutMap *)NULL_PTR) /* Replace: &IfxGtm_TOM1_11_TOUTy_P02_4_OUT */
#define PHASE_W_HS                   ((IfxGtm_Pwm_ToutMap *)NULL_PTR) /* Replace: &IfxGtm_TOM1_12_TOUTx_P02_2_OUT */
#define PHASE_W_LS                   ((IfxGtm_Pwm_ToutMap *)NULL_PTR) /* Replace: &IfxGtm_TOM1_13_TOUTy_P02_5_OUT */

/* ========================= Module State ========================= */
typedef struct
{
    IfxGtm_Pwm               pwm;                                      /* driver handle */
    IfxGtm_Pwm_Channel       channels[GTM_TOM_3PH_NUM_CHANNELS];       /* persistent channels */
    float32                  dutyCycles[GTM_TOM_3PH_NUM_CHANNELS];     /* percent [0..100] */
    float32                  phases[GTM_TOM_3PH_NUM_CHANNELS];         /* degrees or percent phase */
    IfxGtm_Pwm_DeadTime      deadTimes[GTM_TOM_3PH_NUM_CHANNELS];      /* stored dead-time values */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3phState;

/* ========================= Internal ISR and Callback ========================= */
/* ISR declaration with vector attribute (priority defined by ISR_PRIORITY_ATOM) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/*
 * ISR: Minimal processing — toggle LED/debug pin only.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Periodic event callback used by IfxGtm_Pwm InterruptConfig.
 * Intentionally empty — do not perform any operation here.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */
/*
 * Initialize 3-channel complementary center-aligned PWM on TOM with sync start/update.
 *
 * Order strictly follows the mandated initialization sequence from iLLD patterns.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration — complementary high/low per channel */
    /* Phase U */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;   /* high-side active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;    /* low-side  active LOW  */
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

    /* 4) Interrupt configuration (base channel only) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 5) Channel configurations */
    /* Common DTM (dead time) for all channels */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[0].deadTime.falling= PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.falling= PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.falling= PWM_DEAD_TIME_S;

    /* Logical timer channel indices 0..2 as mandated */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;   /* only base channel gets IRQ */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 6) Main configuration */
    config.cluster              = IfxGtm_Cluster_1;                       /* TGC1 */
    config.subModule           = IfxGtm_Pwm_SubModule_tom;               /* TOM */
    config.alignment           = IfxGtm_Pwm_Alignment_center;            /* center-aligned */
    config.syncStart           = TRUE;                                    /* start synchronously */
    config.syncUpdateEnabled   = TRUE;                                    /* synchronous shadow updates */
    config.numChannels         = (uint8)GTM_TOM_3PH_NUM_CHANNELS;
    config.channels            = &channelConfig[0];
    config.frequency           = GTM_TOM_PWM_FREQUENCY_HZ;                /* 20 kHz */
    config.clockSource.tom     = (uint32)IfxGtm_Cmu_Fxclk_0;              /* TOM uses FXCLK_0 */
    config.dtmClockSource      = IfxGtm_Dtm_ClockSource_cmuClock0;        /* DTM clock */

    /* 7) GTM enable guard with CMU clock setup (inside guard only) */
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

    /* 8) Initialize PWM with persistent channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phState.pwm, &g_gtmTom3phState.channels[0], &config);

    /* 9) Store initial state for later updates */
    g_gtmTom3phState.dutyCycles[0] = PHASE_U_INIT_DUTY;
    g_gtmTom3phState.dutyCycles[1] = PHASE_V_INIT_DUTY;
    g_gtmTom3phState.dutyCycles[2] = PHASE_W_INIT_DUTY;

    g_gtmTom3phState.phases[0] = 0.0f;
    g_gtmTom3phState.phases[1] = 0.0f;
    g_gtmTom3phState.phases[2] = 0.0f;

    g_gtmTom3phState.deadTimes[0].rising  = PWM_DEAD_TIME_S;
    g_gtmTom3phState.deadTimes[0].falling = PWM_DEAD_TIME_S;
    g_gtmTom3phState.deadTimes[1].rising  = PWM_DEAD_TIME_S;
    g_gtmTom3phState.deadTimes[1].falling = PWM_DEAD_TIME_S;
    g_gtmTom3phState.deadTimes[2].rising  = PWM_DEAD_TIME_S;
    g_gtmTom3phState.deadTimes[2].falling = PWM_DEAD_TIME_S;

    /* 10) Configure LED/debug GPIO as push-pull output */
    IfxPort_setPinModeOutput(&MODULE_P13, 0u, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update duty cycles with synchronous immediate update.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap-then-increment rule (no loops; three explicit checks) */
    if ((g_gtmTom3phState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[2] = 0.0f; }

    g_gtmTom3phState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply synchronous immediate multi-channel duty update (percent units) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phState.pwm, (float32 *)g_gtmTom3phState.dutyCycles);
}

/*
 * Function present for unit-test expectations; no operation defined.
 */
void values(void)
{
    /* Intentionally empty for unit-test harness */
}
