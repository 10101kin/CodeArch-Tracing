/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver: EGTM ATOM 3-Phase complementary, center-aligned PWM using unified IfxEgtm_Pwm
 *
 * Notes:
 * - This module does not disable any watchdogs (Cpu/Safety). Follow AURIX standard: only CpuX_Main.c may do so.
 * - Interrupt is installed via the unified PWM driver configuration. The ISR body is minimal (LED toggle only).
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Configuration Macros ========================= */
#define NUM_CHANNELS           (3u)
#define PWM_FREQUENCY          (20000.0f)
#define ISR_PRIORITY_ATOM      (20)

#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)
#define PHASE_DUTY_STEP        (10.0f)

/* LED on P03.9 (compound macro expands to two arguments: &MODULE_Pxx, pin) */
#define LED                    &MODULE_P03, 9u

/*
 * Pin routing: Use validated pin symbols only. For pins not present in the validated list,
 * set to NULL and replace during board integration.
 */
#define IFX_TOUT_NULL          ((IfxEgtm_Pwm_ToutMap *)0)

/* U phase: HS=P20.8 (TOUT64) [not in validated list] -> NULL; LS=P20.9 (TOUT65) [validated] */
#define PHASE_U_HS             IFX_TOUT_NULL
#define PHASE_U_LS             (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)

/* V phase: HS=P21.4 (TOUT55) [not in validated list]; LS=P20.11 (TOUT67) [not in validated list] */
#define PHASE_V_HS             IFX_TOUT_NULL
#define PHASE_V_LS             IFX_TOUT_NULL

/* W phase: HS=P20.12 (TOUT68) [not in validated list]; LS=P20.13 (TOUT69) [not in validated list] */
#define PHASE_W_HS             IFX_TOUT_NULL
#define PHASE_W_LS             IFX_TOUT_NULL

/* ========================= Module State ========================= */

typedef struct
{
    IfxEgtm_Pwm              pwm;                               /* driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_CHANNELS];            /* persistent channel state (driver writes here) */
    float32                  dutyCycles[NUM_CHANNELS];          /* duty in percent [0..100] per logical channel */
    float32                  phases[NUM_CHANNELS];              /* phase offset (seconds or fraction as per driver config) */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_CHANNELS];           /* per-channel dead time settings */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback ========================= */

/* ISR: minimal processing — toggle LED and return */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback assigned in InterruptConfig — empty body by design */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */

/**
 * Initialize a 3-phase complementary, center-aligned PWM on eGTM ATOM using the unified driver.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;
    IfxEgtm_Pwm_OutputConfig      output[NUM_CHANNELS];

    /* 2) Populate main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration for each logical channel (HS/LS complementary) */
    /* Channel 0 -> Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;  /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;   /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 1 -> Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 2 -> Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us rising / 1 us falling */
    dtmConfig[0].deadTime.rising    = 1.0e-6f;
    dtmConfig[0].deadTime.falling   = 1.0e-6f;
    dtmConfig[0].fastShutOff        = NULL_PTR;

    dtmConfig[1].deadTime.rising    = 1.0e-6f;
    dtmConfig[1].deadTime.falling   = 1.0e-6f;
    dtmConfig[1].fastShutOff        = NULL_PTR;

    dtmConfig[2].deadTime.rising    = 1.0e-6f;
    dtmConfig[2].deadTime.falling   = 1.0e-6f;
    dtmConfig[2].fastShutOff        = NULL_PTR;

    /* 5) Interrupt configuration */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = (IfxEgtm_Pwm_callBack)IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical channels 0..2) */
    /* CH0 - Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;      /* interrupt on base channel only */

    /* CH1 - Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;              /* no interrupt on this channel */

    /* CH2 - Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;              /* no interrupt on this channel */

    /* 7) Complete the main configuration */
    config.cluster              = IfxEgtm_Cluster_0;                     /* Cluster 0 */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;            /* ATOM submodule */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;          /* center-aligned */
    config.syncStart            = TRUE;                                   /* start all in sync */
    config.syncUpdateEnabled    = TRUE;                                   /* synchronous updates */
    config.numChannels          = (uint8)NUM_CHANNELS;
    config.channels             = channelConfig;
    config.frequency            = PWM_FREQUENCY;                          /* 20 kHz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                      /* ATOM time base = CMU CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_systemClock;    /* DTM clock source */

    /* 8) Enable-guard and CMU setup (inside guard only) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize the PWM driver with persistent handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist initial state into module state structure */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO pin as push-pull output (no state change here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Advance three channel duties by a fixed step and apply immediately (atomic multi-channel update).
 */
void updateEgtmAtom3phInvDuty(void)
{
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply updated duties immediately, atomically, maintaining complementary + center-aligned outputs */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
