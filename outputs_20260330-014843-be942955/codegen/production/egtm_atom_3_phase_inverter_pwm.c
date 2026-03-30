/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for TC4xx eGTM/ATOM 3-phase complementary center-aligned PWM using IfxEgtm_Pwm.
 * - Submodule: ATOM0 Cluster0
 * - Frequency: 20 kHz, center-aligned, synchronized start/updates
 * - Dead-time: 1 us rising/falling per channel via DTM (CMU.CLK0)
 * - Initial duties: U/V/W = 25/50/75 %
 * - Duty step: +10% with wrap-around (atomic multi-channel update)
 * - Period-event ISR toggles LED P03.9
 *
 * Notes:
 * - No watchdog API calls here (must be in CpuX_Main.c if required by application)
 * - No timing loops; schedule updateEgtmAtom3phInvDuty() from Cpu0_Main while(1) with 500 ms cadence
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD dependencies (selected) */
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ===================== Configuration Macros ===================== */
#define NUM_OF_CHANNELS             (3u)
#define PWM_FREQUENCY               (20000.0f)   /* Hz */
#define ISR_PRIORITY_ATOM           (20)

#define PHASE_U_DUTY                (25.0f)      /* percent */
#define PHASE_V_DUTY                (50.0f)      /* percent */
#define PHASE_W_DUTY                (75.0f)      /* percent */
#define PHASE_DUTY_STEP             (10.0f)      /* percent */

/* LED: P03.9 (compound form: port, pin) */
#define LED                         &MODULE_P03, 9

/* ===================== Pin Mapping Macros ===================== */
/* Use ONLY validated symbols when available; otherwise provide NULL_PTR placeholders for integrator replacement. */
/* User-requested pins: U_HS=P20.8(TOUT64), U_LS=P20.9(TOUT65); V_HS=P21.4(TOUT55), V_LS=P20.11(TOUT67); W_HS=P20.12(TOUT68), W_LS=P20.13(TOUT69) */
#define PHASE_U_HS  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT64_P20_8_OUT when available in target pinmap */
#define PHASE_U_LS  (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT55_P21_4_OUT when available in target pinmap */
#define PHASE_V_LS  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT67_P20_11_OUT when available in target pinmap */
#define PHASE_W_HS  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT68_P20_12_OUT when available in target pinmap */
#define PHASE_W_LS  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT69_P20_13_OUT when available in target pinmap */

/* ===================== Module State ===================== */
typedef struct
{
    IfxEgtm_Pwm            pwm;                               /* driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];         /* persistent channel storage */
    float32                dutyCycles[NUM_OF_CHANNELS];       /* percent [0..100] */
    float32                phases[NUM_OF_CHANNELS];           /* phase offsets in percent of period [0..100] */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];        /* stored per-channel dead-time */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ===================== ISR and Callback (must precede init) ===================== */
/* Period-event ISR: toggles LED (minimal ISR load) */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: empty body (driver routes ISR to this callback internally) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ===================== Public API ===================== */
/**
 * Initialize 3-channel complementary, center-aligned inverter PWM using eGTM ATOM.
 * - Configures output routing (HS/LS), dead-time via DTM, interrupt for period event
 * - Enables eGTM clocks (GCLK = module frequency, CMU.CLK0 = module frequency), FXCLK and CLK0
 * - Initializes driver with synchronized start and updates enabled
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration structures (locals) */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Initialize main config with driver defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration: complementary pairs, HS active-high / LS active-low, push-pull, automotive pad driver */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Per-channel DTM dead-time: 1 us rising/falling */
    dtmConfig[0].deadTime.rising = 1e-6f; dtmConfig[0].deadTime.falling = 1e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1e-6f; dtmConfig[1].deadTime.falling = 1e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1e-6f; dtmConfig[2].deadTime.falling = 1e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration: period event on CPU0, priority 20, pulse notify; duty event disabled */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: indices 0..2 for phases U/V/W with initial duties and zero phase */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;              /* base channel gets interrupt */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Complete main config */
    config.cluster            = IfxEgtm_Cluster_0;                       /* ATOM0 Cluster0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;              /* ATOM */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;            /* center-aligned */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;                            /* 20 kHz */
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;                /* ATOM clock source: CMU.CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM clock from CMU.CLK0 */
    config.syncUpdateEnabled  = TRUE;
    config.syncStart          = TRUE;

    /* 8) eGTM enable guard + CMU setup (MANDATORY pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);                                 /* GCLK = module clock */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);               /* CMU.CLK0 = module clock */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize high-level PWM driver */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store persistent initial state (duties, phases, dead-times) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO output (no explicit level set) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Step U/V/W duties by PHASE_DUTY_STEP with wrap-around and apply atomically.
 * This function does not implement any delay; call it from the main loop at 500 ms cadence.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap rule: if (duty + STEP) >= 100 -> set to 0, then unconditionally add STEP */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply atomically to all channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
