/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM ATOM 3-Phase Inverter PWM (TC4xx)
 *
 * Notes:
 * - Followed unified EGTM Pwm init pattern with local config structures
 * - EGTM CMU clock enable inside enable-guard
 * - ISR toggles LED only; period-event callback is empty
 * - No watchdog calls here (must be in CpuX_Main.c)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (user-confirmed values) ========================= */
#define EGTM_ATOM_NUM_CHANNELS     (3)
#define EGTM_PWM_FREQUENCY_HZ      (20000.0f)
#define ISR_PRIORITY_ATOM          (20)

#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* LED: P03.9 (active low) — compound macro (port, pin) */
#define LED                         &MODULE_P03, 9

/* ========================= Validated EGTM pin symbols mapping ========================= */
/*
 * Pin priority rule: use user-requested pins if validated symbols are available.
 * Only symbols listed in the validated set can be used. For unavailable pins, provide NULL_PTR
 * placeholders with integration comment. Do NOT invent symbol names.
 */

/* U phase: HS=P20.8 (TOUT64) [not in validated list] -> placeholder, LS=P20.9 (TOUT65) [validated] */
#define PHASE_U_HS                  (NULL_PTR) /* Replace during integration with validated symbol for P20.8 TOUT64 */
#define PHASE_U_LS                  (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)

/* V phase: HS=P21.4 (TOUT55), LS=P20.11 (TOUT67) — not present in validated list -> placeholders */
#define PHASE_V_HS                  (NULL_PTR) /* Replace during integration with validated symbol for P21.4 TOUT55 */
#define PHASE_V_LS                  (NULL_PTR) /* Replace during integration with validated symbol for P20.11 TOUT67 */

/* W phase: HS=P20.12 (TOUT68), LS=P20.13 (TOUT69) — not present in validated list -> placeholders */
#define PHASE_W_HS                  (NULL_PTR) /* Replace during integration with validated symbol for P20.12 TOUT68 */
#define PHASE_W_LS                  (NULL_PTR) /* Replace during integration with validated symbol for P20.13 TOUT69 */

/* ========================= Module state ========================= */
/** Driver state for 3-phase EGTM ATOM PWM */
typedef struct
{
    IfxEgtm_Pwm                 pwm;                                        /* unified EGTM PWM handle */
    IfxEgtm_Pwm_Channel         channels[EGTM_ATOM_NUM_CHANNELS];           /* persistent channel data storage */
    float32                     dutyCycles[EGTM_ATOM_NUM_CHANNELS];         /* duty in percent (0..100) */
    float32                     phases[EGTM_ATOM_NUM_CHANNELS];             /* phase in degrees or fraction per driver def */
    IfxEgtm_Pwm_DeadTime        deadTimes[EGTM_ATOM_NUM_CHANNELS];          /* stored dead-time parameters */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= Interrupt + callback (declared before init) ========================= */
/** Period-event callback — empty by design; routing handled by driver */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Hardware ISR: priority and TOS match config; body toggles LED only */
IFX_INTERRUPT(EgtmAtomIsr, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* ========================= Initialization ========================= */
/**
 * Initialize EGTM ATOM0 Cluster 0 for 3-phase complementary, center-aligned PWM at 20 kHz
 * with 1 us dead-time on both edges. SyncStart and SyncUpdate enabled. Fxclk_0 used as source.
 * Interrupt configured on base channel (channel 0) with pulse-notify mode; ISR toggles LED.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures (locals) */
    IfxEgtm_Pwm_Config            config;                                      /* main PWM config */
    IfxEgtm_Pwm_ChannelConfig     channelConfig[EGTM_ATOM_NUM_CHANNELS];       /* per-channel config */
    IfxEgtm_Pwm_DtmConfig         dtmConfig[EGTM_ATOM_NUM_CHANNELS];           /* per-channel DTM config */
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;                             /* interrupt (base channel only) */
    IfxEgtm_Pwm_OutputConfig      output[EGTM_ATOM_NUM_CHANNELS];              /* per-channel outputs */

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration: complementary pairs, HS active-high, LS active-low */
    /* U phase (channel 0 mapping) */
    output[0].pin                   = PHASE_U_HS;
    output[0].complementaryPin      = PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;                    /* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;                     /* LS active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase (channel 1 mapping) */
    output[1].pin                   = PHASE_V_HS;
    output[1].complementaryPin      = PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase (channel 2 mapping) */
    output[2].pin                   = PHASE_W_HS;
    output[2].complementaryPin      = PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us on rising and falling edges */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: base channel period event */
    interruptConfig.mode        = IfxEgtm_Pwm_IrqMode_pulseNotify;              /* pulse-notify on period */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;                              /* CPU0 */
    interruptConfig.priority    = ISR_PRIORITY_ATOM;                            /* 20 */
    interruptConfig.vmId        = IfxSrc_VmId_0;                                /* VM 0 */
    interruptConfig.periodEvent = &IfxEgtm_periodEventFunction;                 /* callback */
    interruptConfig.dutyEvent   = NULL_PTR;                                     /* not used */

    /* 6) Channel configuration: contiguous logical indices starting from SubModule_Ch_0 */
    /* Channel 0 -> U phase */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;                              /* base channel only */

    /* Channel 1 -> V phase */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;                                     /* no IRQ on this channel */

    /* Channel 2 -> W phase */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;                                     /* no IRQ on this channel */

    /* 7) Complete main config */
    config.cluster             = IfxEgtm_Cluster_0;                             /* target cluster 0 */
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;                    /* use ATOM */
    config.alignment           = IfxEgtm_Pwm_Alignment_centerAligned;           /* center-aligned */
    config.syncStart           = TRUE;                                          /* start all together */
    config.frequency           = EGTM_PWM_FREQUENCY_HZ;                         /* 20 kHz */
    config.channels            = channelConfig;                                 /* channel setup */
    config.numChannels         = EGTM_ATOM_NUM_CHANNELS;                        /* 3 channels */
    config.clockSource.atom    = IfxEgtm_Atom_Ch_ClkSrc_cmuFxclk0;              /* ATOM from Fxclk_0 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;             /* DTM clock source */
    config.syncUpdateEnabled   = TRUE;                                          /* update at period end */

    /* Provide persistent channel storage from module state (driver stores pointers) */
    config.channelData         = g_egtmAtom3phInv.channels;

    /* 8) EGTM enable guard + CMU clock setup */
    if (FALSE == IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);                                           /* enable EGTM */
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);              /* dynamic frequency */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);                 /* GCLK = module clock */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, moduleFreq);/* program CLK0 */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0)); /* enable FXCLK + CLK0 */
    }

    /* 9) Initialize unified EGTM PWM */
    if (FALSE == IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &config))
    {
        /* Initialization failed — leave without further action. Application may handle error. */
        return;
    }

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

    /* 11) Configure LED GPIO as push-pull output (do not drive level here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
