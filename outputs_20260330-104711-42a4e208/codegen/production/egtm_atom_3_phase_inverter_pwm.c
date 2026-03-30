/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver implementation for EGTM ATOM 3-Phase Inverter PWM
 *
 * Notes:
 * - Uses IfxEgtm_Pwm high-level driver (TC4xx)
 * - Generates 3 complementary pairs (U, V, W) center-aligned at 20 kHz
 * - DTM dead-time: rising=1us, falling=1us
 * - Interrupt: period event, priority 20, CPU0, ISR toggles LED P03.9
 * - No watchdog handling here (must be in CpuX_Main.c only)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================================
 * Compile-time configuration macros
 * ============================================= */
#define NUM_OF_CHANNELS        (3u)
#define PWM_FREQUENCY          (20000.0f)   /* Hz */
#define ISR_PRIORITY_ATOM      (20)

/* Initial duties in percent (channel order: CH0=V, CH1=U, CH2=W) */
#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)

/* Duty update step (percent) */
#define PHASE_DUTY_STEP        (10.0f)

/* LED: P03.9 - used by ISR to indicate PWM period */
#define LED                    &MODULE_P03, 9

/* =============================================
 * Pin routing macros (TOUT mappings)
 *
 * PIN PRIORITY RULE: Use only validated symbols. For any user-requested pin
 * not present in the validated list, keep a NULL_PTR placeholder and replace
 * during integration with the correct &IfxEgtm_ATOMx_y_TOUTz_Pxx_y_OUT symbol.
 * ============================================= */
/* Phase U: HS=P20.8 (TOUT64), LS=P20.9 (TOUT65) */
#define PHASE_U_HS   (NULL_PTR)                                 /* Replace with &IfxEgtm_ATOMx_y_TOUT64_P20_8_OUT */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)       /* Validated */

/* Phase V: HS=P21.4 (TOUT55), LS=P20.11 (TOUT67) */
#define PHASE_V_HS   (NULL_PTR)                                 /* Replace with &IfxEgtm_ATOMx_y_TOUT55_P21_4_OUT */
#define PHASE_V_LS   (NULL_PTR)                                 /* Replace with &IfxEgtm_ATOMx_y_TOUT67_P20_11_OUT */

/* Phase W: HS=P20.12 (TOUT68), LS=P20.13 (TOUT69) */
#define PHASE_W_HS   (NULL_PTR)                                 /* Replace with &IfxEgtm_ATOMx_y_TOUT68_P20_12_OUT */
#define PHASE_W_LS   (NULL_PTR)                                 /* Replace with &IfxEgtm_ATOMx_y_TOUT69_P20_13_OUT */

/* =============================================
 * Module-scoped runtime state
 * ============================================= */
typedef struct
{
    IfxEgtm_Pwm            pwm;                                     /* Driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];               /* Persistent channel handle array */
    float32                dutyCycles[NUM_OF_CHANNELS];             /* Duty in percent per logical channel */
    float32                phases[NUM_OF_CHANNELS];                 /* Phase offset (fraction of period) */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];              /* Dead-time per channel (seconds) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =============================================
 * Forward declarations: ISR and callback
 * ============================================= */

/* Period-event ISR: minimal body to toggle LED */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Empty period-event callback required by high-level PWM driver */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================================
 * Public API implementations
 * ============================================= */

/**
 * Initialize a 3-channel complementary, center-aligned PWM for a three-phase inverter
 * on eGTM ATOM0, Cluster 0 using the unified high-level driver.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as local variables */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load driver defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure OutputConfig entries (pins, polarity, pad) */
    /* Channel 0 -> Phase V */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[0].polarity              = Ifx_ActiveState_high;     /* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;      /* LS active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 1 -> Phase U */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
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

    /* 4) Configure DTM dead-time for each channel (1us rising/falling) */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Configure the interrupt */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;  /* callback, empty body */
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Fill ChannelConfig[0..2] with logical channels and per-channel settings */
    /* CH0 -> Phase V */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_V_DUTY;  /* 50% */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* base channel carries interrupt */

    /* CH1 -> Phase U */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_U_DUTY;  /* 25% */
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;  /* 75% */
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Complete main Config fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;        /* ATOM submodule */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;      /* center-aligned */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;                     /* 20 kHz */
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;         /* ATOM uses CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock */
    config.syncUpdateEnabled  = TRUE;                              /* syncUpdate */
    config.syncStart          = TRUE;                              /* syncStart */

    /* 8) Enable guard: enable eGTM and required clocks if not enabled */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Run clocks at module frequency (dynamic) */
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            /* ATOM submodule requires enabling CLK0 */
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize PWM hardware with channel configuration */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist runtime state (duties, phases, dead-times) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (used in ISR) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Implement a periodic duty-cycle stepper for all three logical PWM channels (U, V, W).
 * For each channel: if (duty + STEP) >= 100, set duty=0, then add STEP. Finally, apply immediately.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-at-100 checks (exact sequence: check -> maybe reset -> unconditional add) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply synchronously to all complementary pairs (percent values) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
