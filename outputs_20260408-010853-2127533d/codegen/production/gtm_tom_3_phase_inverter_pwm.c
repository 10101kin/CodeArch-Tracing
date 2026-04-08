/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ==========================================
 * Configuration macros (numeric constants)
 * ========================================== */
#define NUM_PWM_CHANNELS              (3u)
#define PWM_SWITCHING_FREQUENCY_HZ    (20000.0f)      /* 20 kHz */
#define ISR_PRIORITY_ATOM             (20)

/* User-requested TOUT mappings (validated for TC3xx) */
#define PHASE_U_HS   (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_5_TOUT2_P02_2_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* Initial duties and step (percent) */
#define PHASE_U_DUTY_PERCENT          (25.0f)
#define PHASE_V_DUTY_PERCENT          (50.0f)
#define PHASE_W_DUTY_PERCENT          (75.0f)
#define PHASE_DUTY_STEP_PERCENT       (10.0f)

/* Diagnostic LED (port, pin) for ISR toggle */
#define LED                           &MODULE_P13, 0

/* ==========================================
 * Module state
 * ========================================== */
typedef struct
{
    IfxGtm_Pwm               pwm;                                  /* unified PWM handle */
    IfxGtm_Pwm_Channel       channels[NUM_PWM_CHANNELS];           /* persistent channels array */
    float32                  dutyCycles[NUM_PWM_CHANNELS];          /* percent 0..100 */
    float32                  phases[NUM_PWM_CHANNELS];              /* phase in percent/normalized */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_PWM_CHANNELS];          /* per-channel dead time */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv; /* zero-initialized */

/* ==========================================
 * ISR and callback declarations (must appear before init)
 * ========================================== */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Stub: no action */
}

/* ==========================================
 * Initialization function
 * ========================================== */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_PWM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (complementary HS/LS, active-high HS, active-low LS) */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (seconds). Use 1e-6 for rising/falling. */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;

    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;

    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 5) Interrupt configuration: period notification on channel 0 only */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2, phases 0.0, initial duty */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main config: TOM submodule, Cluster_1, center-aligned, sync start/update, FXCLK0 */
    config.cluster              = IfxGtm_Cluster_1;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_PWM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_SWITCHING_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;           /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 8) Enable guard: enable GTM and clocks only if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 10) Store persistent runtime state */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure diagnostic LED as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ==========================================
 * Duty update function
 * ========================================== */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 => duty = 0; then always add step */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP_PERCENT) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP_PERCENT;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP_PERCENT;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP_PERCENT;

    /* Apply synchronized immediate duty update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
