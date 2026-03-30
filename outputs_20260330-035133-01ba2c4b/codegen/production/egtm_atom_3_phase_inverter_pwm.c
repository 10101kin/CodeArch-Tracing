/*
 * EGTM ATOM 3-Phase Inverter PWM driver (TC4xx)
 *
 * Migration: TC3xx GTM TOM -> TC4xx eGTM ATOM using unified IfxEgtm_Pwm
 * - ATOM0, Cluster 0
 * - Center-aligned 20 kHz
 * - 3 complementary pairs with 1 us rising/falling dead-time
 * - Initial duties: U=25%, V=50%, W=75%
 * - Duty update step: 10%
 * - One period-event ISR (priority 20, CPU0) toggles LED P03.9
 *
 * Notes:
 * - No watchdog disable here (must be done only in CpuX_Main.c).
 * - No STM-based timing here (while-loop timing belongs to Cpu0_Main.c).
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros (configuration values) ========================= */
#define NUM_OF_CHANNELS          (3u)
#define PWM_FREQUENCY            (20000.0f)           /* 20 kHz */
#define ISR_PRIORITY_ATOM        (20)

#define PHASE_U_DUTY             (25.0f)
#define PHASE_V_DUTY             (50.0f)
#define PHASE_W_DUTY             (75.0f)
#define PHASE_DUTY_STEP          (10.0f)

/* LED P03.9 compound macro (used directly in IfxPort_* calls) */
#define LED                       &MODULE_P03, 9

/* ========================= Pin routing macros (validated list only) ========================= */
/*
 * Use ONLY symbols present in validated pin list. For any user-requested pin not present
 * in the provided validated list, keep it as NULL_PTR and replace during integration
 * with the correct IfxEgtm_*_TOUTxx_Pyy_z_OUT symbol from the device pin map.
 */
/* Phase U: HS requested P20.8 (TOUT64) not in validated list -> fallback to a validated ATOM0_0 HS pin */
#define PHASE_U_HS               (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)     /* Validated */
/* Phase U: LS P20.9 (TOUT65) present in validated list */
#define PHASE_U_LS               (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)   /* Validated */

/* Phase V: HS P21.4 (TOUT55) not present in validated list -> set NULL (replace in integration) */
#define PHASE_V_HS               (NULL_PTR) /* Replace with proper &IfxEgtm_ATOM0_1_TOUTxx_P21_4_OUT */
/* Phase V: LS P20.11 (TOUT67) not present in validated list -> set NULL (replace in integration) */
#define PHASE_V_LS               (NULL_PTR) /* Replace with proper &IfxEgtm_ATOM0_1N_TOUTxx_P20_11_OUT */

/* Phase W: HS P20.12 (TOUT68) not present in validated list -> set NULL (replace in integration) */
#define PHASE_W_HS               (NULL_PTR) /* Replace with proper &IfxEgtm_ATOM0_2_TOUTxx_P20_12_OUT */
/* Phase W: LS P20.13 (TOUT69) not present in validated list -> set NULL (replace in integration) */
#define PHASE_W_LS               (NULL_PTR) /* Replace with proper &IfxEgtm_ATOM0_2N_TOUTxx_P20_13_OUT */

/* ========================= Module state ========================= */
typedef struct
{
    IfxEgtm_Pwm             pwm;                                      /* Driver handle */
    IfxEgtm_Pwm_Channel     channels[NUM_OF_CHANNELS];                /* Persistent channels (driver stores pointer) */
    float32                 dutyCycles[NUM_OF_CHANNELS];              /* Duty in percent */
    float32                 phases[NUM_OF_CHANNELS];                  /* Phase [0..1) */
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];               /* Dead-times (s) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback ========================= */
/* ISR declaration (priority: ISR_PRIORITY_ATOM, provider: CPU0) */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/*
 * Period-event ISR: strictly minimal body as per guidelines.
 */
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Period-event callback assigned via InterruptConfig.
 * Must be non-static and have an empty body.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */
/**
 * Initialize a 3-phase complementary PWM on eGTM ATOM using unified IfxEgtm_Pwm driver.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as local variables */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Initialize the main Pwm config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure outputs: complementary pairs, polarity and pad configuration */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;  /* HS active-high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;   /* LS active-low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Configure DTM dead-time for each channel: 1 us rising, 1 us falling */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Configure period-event interrupt: pulse-notify, CPU0, priority macro, callback; only for base ch 0 */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Fill per-channel configurations */
    /* Channel 0 -> Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;  /* Base channel gets interrupt */

    /* Channel 1 -> Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Channel 2 -> Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Complete main config fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;            /* ATOM uses Clk_0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;    /* DTM clock 0 */
    config.syncUpdateEnabled  = TRUE;                                  /* Synchronized updates */
    config.syncStart          = TRUE;                                  /* Synchronized start */

    /* 8) eGTM enable guard + CMU configuration (MANDATORY pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver with persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist initial duties, phases, and dead-times into module state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (do not drive state here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update duty cycles by 10% step with wrap at 100%, then apply atomically.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap then increment (MANDATORY sequence) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediate multi-channel update atomically */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
