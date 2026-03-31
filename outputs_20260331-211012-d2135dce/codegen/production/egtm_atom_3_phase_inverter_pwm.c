/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver for TC4xx eGTM ATOM 3-phase complementary PWM using IfxEgtm_Pwm
 * - 3 logical channels (U,V,W), complementary outputs, center-aligned, sync start/update
 * - 20 kHz switching, 1 us rising/falling dead-time
 * - Period event interrupt on base channel (channel 0) toggles LED P03.9
 *
 * Notes:
 * - No watchdog handling here (must be in CpuX_Main.c only)
 * - Pins are provided via OutputConfig array; no separate pin init function
 * - eGTM enable + CMU clock setup done with mandatory enable-guard pattern
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define NUM_OF_CHANNELS       (3U)
#define PWM_FREQUENCY         (20000.0f)
#define ISR_PRIORITY_ATOM     (20)
#define PHASE_U_DUTY          (25.0f)
#define PHASE_V_DUTY          (50.0f)
#define PHASE_W_DUTY          (75.0f)
#define PHASE_DUTY_STEP       (10.0f)

/* LED P03.9 compound macro (port, pin) */
#define LED                   (&MODULE_P03), 9

/*
 * Pin routing macros (TOUT mappings)
 * Use NULL_PTR placeholders if validated symbols are not available in this integration context.
 * Replace NULL_PTR with concrete symbols from IfxEgtm_PinMap when integrating on target board:
 *   Phase U: HS -> &IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT,  LS -> &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT
 *   Phase V: HS -> &IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT, LS -> &IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT
 *   Phase W: HS -> &IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT, LS -> &IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT
 */
#define PHASE_U_HS            (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT */
#define PHASE_U_LS            (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT */
#define PHASE_V_HS            (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT */
#define PHASE_V_LS            (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT */
#define PHASE_W_HS            (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT */
#define PHASE_W_LS            (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT */

/* =============================
 * Module State
 * ============================= */

typedef struct
{
    IfxEgtm_Pwm              pwm;                              /* Driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];        /* Persistent channel objects */
    float32                  dutyCycles[NUM_OF_CHANNELS];       /* Duty in percent */
    float32                  phases[NUM_OF_CHANNELS];           /* Phase in degrees (edge-aligned use) */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];        /* Stored dead-times */
} EGTM_Atom3phInv_State;

IFX_STATIC EGTM_Atom3phInv_State g_egtmAtom3phInv;

/* =============================
 * ISR and Callback Declarations (must appear before init)
 * ============================= */

/* Period-event ISR: toggle LED and return */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Empty period-event callback (required by InterruptConfig) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Public API Implementations
 * ============================= */

/**
 * Initialize a 3-channel complementary, center-aligned ATOM PWM with sync start/update and base-channel period ISR.
 * Sequence strictly follows unified IfxEgtm_Pwm init pattern and CMU enable-guard pattern.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;                              /* main PWM config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];      /* per-channel config */
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];          /* per-channel DTM config */
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;                     /* single interrupt config (base channel) */
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];             /* per-channel pin config */

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration (complementary pairs) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;                 /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;                  /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: 1 us rising and falling dead-time for each channel */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration (base channel only) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Per-channel configuration: logical indices Ch_0, Ch_1, Ch_2 */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* base channel period ISR */

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Complete main PWM configuration */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                 /* ATOM uses CMU Clk_0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock source */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];

    /* 8) Enable-guard and CMU setup (inside the not-enabled branch only) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver with persistent handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initial duties, phases, and dead-times into persistent state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (no level set here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update duty cycles of all three logical channels by a fixed step with wrap-around, then apply immediately.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap then add, per channel, as specified */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply new duties synchronously across all channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
