/*
 * GTM TOM 3-Phase Inverter PWM driver (TC3xx)
 * - Unified IfxGtm_Pwm on TOM backend
 * - 3 complementary channels, center aligned, sync start/update
 * - 20 kHz, 1 us rising/falling deadtime
 * - Pins: P02.0/P02.7 (U), P02.1/P02.4 (V), P02.2/P02.5 (W) [placeholders]
 * - Period-event callback (no-op) and ATOM ISR toggling P13.0
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =========================== Macros/Constants =========================== */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY_HZ           (20000.0f)
#define ISR_PRIORITY_ATOM          (20)

/* User-requested pin assignments (validated TOUT symbols not provided in template).
 * Replace NULL_PTR with actual TOUT map symbols during integration, e.g.:
 *   &IfxGtm_TOM1_x_TOUTy_P02_0_OUT, etc.
 */
#define PHASE_U_HS                 (NULL_PTR) /* P02.0: replace with &IfxGtm_TOM1_x_TOUTy_P02_0_OUT */
#define PHASE_U_LS                 (NULL_PTR) /* P02.7: replace with &IfxGtm_TOM1_x_TOUTy_P02_7_OUT */
#define PHASE_V_HS                 (NULL_PTR) /* P02.1: replace with &IfxGtm_TOM1_x_TOUTy_P02_1_OUT */
#define PHASE_V_LS                 (NULL_PTR) /* P02.4: replace with &IfxGtm_TOM1_x_TOUTy_P02_4_OUT */
#define PHASE_W_HS                 (NULL_PTR) /* P02.2: replace with &IfxGtm_TOM1_x_TOUTy_P02_2_OUT */
#define PHASE_W_LS                 (NULL_PTR) /* P02.5: replace with &IfxGtm_TOM1_x_TOUTy_P02_5_OUT */

/* Initial duty cycles in percent */
#define PHASE_U_DUTY_INIT          (25.0f)
#define PHASE_V_DUTY_INIT          (50.0f)
#define PHASE_W_DUTY_INIT          (75.0f)

/* Duty update step in percent */
#define PHASE_DUTY_STEP            (10.0f)

/* Debug LED (P13.0) compound macro: expands to (port, pin) */
#define LED                        &MODULE_P13, 0u

/* =========================== Module State ============================== */
typedef struct
{
    IfxGtm_Pwm             pwm;                                   /* PWM driver handle */
    IfxGtm_Pwm_Channel     channels[NUM_OF_CHANNELS];             /* Persistent channels array */
    float32                dutyCycles[NUM_OF_CHANNELS];            /* Duty in percent (0..100) */
    float32                phases[NUM_OF_CHANNELS];                /* Phase offsets in percent */
    IfxGtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];            /* Dead-times per pair (s) */
} GtmTom3PhInv_State;

IFX_STATIC GtmTom3PhInv_State g_gtmTom3PhInvState;

/* =========================== ISR and Callbacks ========================= */
/* ATOM ISR stub: toggle LED. Priority must match InterruptConfig.priority. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback: no-op. Assigned to InterruptConfig.periodEvent. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =========================== Public API =============================== */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Initialize main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (complementary pairs) */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high; /* HS: active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;  /* LS: active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us rising and falling for each pair */
    dtmConfig[0].deadTime.rising = 1e-6f; dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f; dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f; dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Interrupt configuration: period event only on base channel (CH0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: CH0=U, CH1=V, CH2=W; center-aligned duties in percent */
    /* CH0 - Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;   /* base channel gets interrupt */

    /* CH1 - Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;           /* no interrupt on CH1 */

    /* CH2 - Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;           /* no interrupt on CH2 */

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                    /* TOM1 cluster 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;            /* TOM backend */
    config.alignment            = IfxGtm_Pwm_Alignment_center;         /* center-aligned */
    config.syncStart            = TRUE;                                 /* synchronized start */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                     /* 20 kHz */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;    /* DTM from CMU Clock0 */
    config.syncUpdateEnabled    = TRUE;                                 /* synchronized shadow updates */
    config.clockSource.atom     = (uint32)IfxGtm_Cmu_Fxclk_0;           /* use FXCLK0 */

    /* 8) Enable guard: enable GTM + CMU FXCLK if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        (void)IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM); /* dynamic read per guidelines */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 9) Initialize PWM using persistent channels array from module state */
    IfxGtm_Pwm_init(&g_gtmTom3PhInvState.pwm, &g_gtmTom3PhInvState.channels[0], &config);

    /* 10) Store initial duties, phases, and dead-times into module state */
    g_gtmTom3PhInvState.dutyCycles[0] = channelConfig[0].duty; g_gtmTom3PhInvState.phases[0] = channelConfig[0].phase; g_gtmTom3PhInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3PhInvState.dutyCycles[1] = channelConfig[1].duty; g_gtmTom3PhInvState.phases[1] = channelConfig[1].phase; g_gtmTom3PhInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3PhInvState.dutyCycles[2] = channelConfig[2].duty; g_gtmTom3PhInvState.phases[2] = channelConfig[2].phase; g_gtmTom3PhInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure debug LED (P13.0) as output; no explicit level set here */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateGtmTom3phInvDuty(void)
{
    /* Wrap-to-0-then-add-step behavior, explicitly per channel (no loops) */
    if ((g_gtmTom3PhInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhInvState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3PhInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhInvState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3PhInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhInvState.dutyCycles[2] = 0.0f; }

    g_gtmTom3PhInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3PhInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3PhInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately and atomically via synchronized shadow updates */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3PhInvState.pwm, (float32 *)g_gtmTom3PhInvState.dutyCycles);
}
