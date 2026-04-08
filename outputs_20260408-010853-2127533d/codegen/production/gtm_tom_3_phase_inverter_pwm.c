/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for TC3xx GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm.
 *
 * - Submodule: TOM (Cluster_1)
 * - Center-aligned, complementary outputs with dead-time
 * - Clock source: FXCLK0
 * - Frequency: 20 kHz
 *
 * Notes:
 * - Watchdog disable is NOT performed here (must be in CpuX_Main.c only).
 * - Follows iLLD initialization patterns and union clockSource.tom setup.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (numerical and pins) ========================= */
#define GTM_TOM_INV_NUM_CHANNELS          (3u)
#define GTM_TOM_INV_PWM_FREQUENCY_HZ      (20000.0f)      /* 20 kHz */
#define GTM_TOM_INV_ISR_PRIORITY_ATOM     (20)            /* CPU0 priority 20 */

#define PHASE_U_DUTY_INIT                 (25.0f)
#define PHASE_V_DUTY_INIT                 (50.0f)
#define PHASE_W_DUTY_INIT                 (75.0f)
#define PHASE_DUTY_STEP                   (10.0f)

#define PWM_DEAD_TIME_S                   (5.0e-07f)      /* 0.5 us per migration rule */
#define PWM_MIN_PULSE_TIME_S              (1.0e-06f)      /* 1 us (not a field in DTM config; documented only) */

/* LED: P13.0 (compound macro: port, pin) */
#define LED                               &MODULE_P13, 0

/* ========== Validated TOUT mappings (from user-requested assignments) ========== */
#define PHASE_U_HS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_5_TOUT2_P02_2_OUT)
#define PHASE_W_LS   ((IfxGtm_Pwm_ToutMap*)&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* ========================= Module State ========================= */
typedef struct
{
    IfxGtm_Pwm               pwm;                                        /* driver handle */
    IfxGtm_Pwm_Channel       channels[GTM_TOM_INV_NUM_CHANNELS];         /* persistent channels array */
    float32                  dutyCycles[GTM_TOM_INV_NUM_CHANNELS];       /* duty in percent */
    float32                  phases[GTM_TOM_INV_NUM_CHANNELS];           /* phase in percent (0..100) */
    IfxGtm_Pwm_DeadTime      deadTimes[GTM_TOM_INV_NUM_CHANNELS];        /* stored dead-times */
} GtmTom3phInv_State;

/* IFX_STATIC per structural rule */
IFX_STATIC GtmTom3phInv_State g_gtaState;

/* ========================= ISR and callback ========================= */
/* ISR declaration with priority macro per structural rules */
IFX_INTERRUPT(interruptGtmAtom, 0, GTM_TOM_INV_ISR_PRIORITY_ATOM);

/*
 * ISR: toggles LED only. No heavy processing.
 */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Period-event callback (empty) required by interrupt configuration.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */
/*
 * Initialize a 3-channel, center-aligned PWM on TOM using unified IfxGtm_Pwm with complementary outputs.
 * Algorithm strictly follows the SW detailed design and iLLD patterns.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_INV_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for 3 logical channels (U, V, W) */
    /* Complementary polarity convention: high-side active high, low-side active low */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: dead times and min pulse (documented) */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_S;

    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_S;

    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_S;

    /* 5) Interrupt configuration for base channel only */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;   /* periodic notification */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)GTM_TOM_INV_ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2 (U, V, W) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;  /* master base */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;           /* only on base channel */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Complete main configuration */
    config.cluster              = IfxGtm_Cluster_1;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.numChannels          = (uint8)GTM_TOM_INV_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_INV_PWM_FREQUENCY_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;   /* union field: TOM FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;
    config.syncUpdateEnabled    = TRUE;

    /* 8) Enable-guard and CMU configuration inside guard */
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

    /* 9) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gtaState.pwm, &g_gtaState.channels[0], &config);

    /* Store initial state: duties, phases, dead-times */
    g_gtaState.dutyCycles[0] = channelConfig[0].duty; g_gtaState.phases[0] = channelConfig[0].phase; g_gtaState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtaState.dutyCycles[1] = channelConfig[1].duty; g_gtaState.phases[1] = channelConfig[1].phase; g_gtaState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtaState.dutyCycles[2] = channelConfig[2].duty; g_gtaState.phases[2] = channelConfig[2].phase; g_gtaState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure debug LED GPIO for ISR toggle (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update three phase duties per wrap rule and apply immediately to the running PWM.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule (exact sequence without extra clamps) */
    if ((g_gtaState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtaState.dutyCycles[0] = 0.0f; }
    if ((g_gtaState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtaState.dutyCycles[1] = 0.0f; }
    if ((g_gtaState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtaState.dutyCycles[2] = 0.0f; }

    g_gtaState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtaState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtaState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply synchronously using immediate multi-channel update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtaState.pwm, (float32*)g_gtaState.dutyCycles);
}

/*
 * Unit-test hook: advance duty update sequence.
 * Implemented as a thin wrapper to the production updater.
 */
void duty(void)
{
    updateGtmTom3phInvDuty();
}
