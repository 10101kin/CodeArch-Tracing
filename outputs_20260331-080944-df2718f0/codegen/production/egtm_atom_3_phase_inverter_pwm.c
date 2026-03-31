/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production EGTM ATOM 3-phase complementary, center-aligned PWM (TC4xx)
 * - Submodule: ATOM0, Cluster 0
 * - Frequency: 20 kHz
 * - Dead-time: 1 us (rising/falling)
 * - Initial duties: U=25%, V=50%, W=75%
 * - syncStart and syncUpdate enabled
 * - Interrupt on base channel (CH0), ISR priority 20, TOS CPU0, VMID 0
 *
 * Notes:
 * - Watchdog disable must be done only in CpuX_Main.c (per AURIX standard). No watchdog code here.
 * - Pins are configured via IfxEgtm_Pwm output configuration.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (numeric configuration) ========================= */
#define NUM_OF_CHANNELS     (3u)
#define PWM_FREQUENCY       (20000.0f)
#define ISR_PRIORITY_ATOM   (20)

#define PHASE_U_DUTY        (25.0f)
#define PHASE_V_DUTY        (50.0f)
#define PHASE_W_DUTY        (75.0f)
#define PHASE_DUTY_STEP     (10.0f)
#define WAIT_TIME           (500.0f)

/* ========================= LED (port, pin) ========================= */
/* Active-low LED on P03.9; toggle only in ISR. */
#define LED &MODULE_P03, 9

/* ========================= Pin macros (validated pin symbols only) ========================= */
/*
 * User-requested pins:
 *  U_HS: P20.8 (TOUT64)     U_LS: P20.9 (TOUT65)
 *  V_HS: P21.4 (TOUT55)     V_LS: P20.11 (TOUT67)
 *  W_HS: P20.12 (TOUT68)    W_LS: P20.13 (TOUT69)
 *
 * Important: Use ONLY validated symbols. If a requested pin is not present in the validated list,
 * define the macro as NULL_PTR and replace during board integration with the correct IfxEgtm pin symbol.
 */
#define PHASE_U_HS   (NULL_PTR) /* P20.8 / TOUT64: not in validated list here; replace with valid IfxEgtm pin symbol */
#define PHASE_U_LS   (NULL_PTR) /* P20.9 / TOUT65: replace with &IfxEgtm_ATOMx_yN_TOUT65_P20_9_OUT matching your routing */
#define PHASE_V_HS   (NULL_PTR) /* P21.4 / TOUT55: not in validated list here; replace with valid IfxEgtm pin symbol */
#define PHASE_V_LS   (NULL_PTR) /* P20.11 / TOUT67: not in validated list here; replace with valid IfxEgtm pin symbol */
#define PHASE_W_HS   (NULL_PTR) /* P20.12 / TOUT68: not in validated list here; replace with valid IfxEgtm pin symbol */
#define PHASE_W_LS   (NULL_PTR) /* P20.13 / TOUT69: not in validated list here; replace with valid IfxEgtm pin symbol */

/* ========================= Module state ========================= */
typedef struct
{
    IfxEgtm_Pwm              pwm;                                 /* Driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];           /* Persistent channel objects */
    float32                  dutyCycles[NUM_OF_CHANNELS];         /* Duty cycles in percent */
    float32                  phases[NUM_OF_CHANNELS];             /* Phase offsets in [0..1), unused=0 */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];          /* Dead-time per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and callback (declared before init) ========================= */
/* Period-event callback: empty body as required. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Hardware ISR: toggle LED only. Priority must match ISR_PRIORITY_ATOM. */
IFX_INTERRUPT(EgtmAtomIsr, 0, ISR_PRIORITY_ATOM);
void EgtmAtomIsr(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Initialization ========================= */
/**
 * Initialize EGTM ATOM 3-phase complementary PWM with center-aligned mode.
 * - Configures 3 logical channels with complementary outputs and 1 us dead-time.
 * - Base channel (CH0) period event interrupt is routed to EgtmAtomIsr with priority 20 on CPU0.
 * - syncStart and syncUpdate are enabled.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures (locals) */
    IfxEgtm_Pwm_Config            config;                                        /* Main PWM configuration */
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];                /* Per-channel configuration */
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];                    /* Dead-time configuration */
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;                               /* Interrupt configuration (for base channel) */
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];                       /* Output (pin) configuration */

    /* 2) Initialize main PWM config with defaults for EGTM */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration: complementary pairs, HS active-high, LS active-low */
    output[0].pin                    = PHASE_U_HS;                               /* High-side U */
    output[0].complementaryPin       = PHASE_U_LS;                               /* Low-side  U */
    output[0].polarity               = Ifx_ActiveState_high;                     /* HS active-high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;                      /* LS active-low */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = PHASE_V_HS;                               /* High-side V */
    output[1].complementaryPin       = PHASE_V_LS;                               /* Low-side  V */
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = PHASE_W_HS;                               /* High-side W */
    output[2].complementaryPin       = PHASE_W_LS;                               /* Low-side  W */
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us for rising and falling edges */
    dtmConfig[0].deadTime.rising     = 1e-6f;
    dtmConfig[0].deadTime.falling    = 1e-6f;
    dtmConfig[0].fastShutOff         = NULL_PTR;

    dtmConfig[1].deadTime.rising     = 1e-6f;
    dtmConfig[1].deadTime.falling    = 1e-6f;
    dtmConfig[1].fastShutOff         = NULL_PTR;

    dtmConfig[2].deadTime.rising     = 1e-6f;
    dtmConfig[2].deadTime.falling    = 1e-6f;
    dtmConfig[2].fastShutOff         = NULL_PTR;

    /* 5) Interrupt configuration: base channel period-event, pulse-notify, CPU0, priority 20, VMID 0 */
    interruptConfig.mode             = IfxEgtm_Pwm_IrqMode_pulseNotify;          /* Pulse-notify mode */
    interruptConfig.isrProvider      = IfxSrc_Tos_cpu0;                          /* Type-of-service: CPU0 */
    interruptConfig.priority         = ISR_PRIORITY_ATOM;                        /* Priority: 20 */
    interruptConfig.vmId             = IfxSrc_VmId_0;                            /* Virtual machine ID */
    interruptConfig.periodEvent      = &IfxEgtm_periodEventFunction;             /* Period event callback */
    interruptConfig.dutyEvent        = NULL_PTR;                                 /* No duty callback */

    /* 6) Channel configuration: CH0..CH2, phase=0, duties 25/50/75 %, attach DTM and outputs */
    channelConfig[0].timerCh         = IfxEgtm_Pwm_SubModule_Ch_0;               /* Base channel */
    channelConfig[0].phase           = 0.0f;
    channelConfig[0].duty            = PHASE_U_DUTY;
    channelConfig[0].dtm             = &dtmConfig[0];
    channelConfig[0].output          = &output[0];
    channelConfig[0].mscOut          = NULL_PTR;                                 /* Not used */
    channelConfig[0].interrupt       = &interruptConfig;                         /* Interrupt on base channel only */

    channelConfig[1].timerCh         = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase           = 0.0f;
    channelConfig[1].duty            = PHASE_V_DUTY;
    channelConfig[1].dtm             = &dtmConfig[1];
    channelConfig[1].output          = &output[1];
    channelConfig[1].mscOut          = NULL_PTR;
    channelConfig[1].interrupt       = NULL_PTR;

    channelConfig[2].timerCh         = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase           = 0.0f;
    channelConfig[2].duty            = PHASE_W_DUTY;
    channelConfig[2].dtm             = &dtmConfig[2];
    channelConfig[2].output          = &output[2];
    channelConfig[2].mscOut          = NULL_PTR;
    channelConfig[2].interrupt       = NULL_PTR;

    /* 7) Main configuration fields */
    config.cluster                   = IfxEgtm_Cluster_0;                        /* Cluster 0 */
    config.subModule                 = IfxEgtm_SubModule_atom;                   /* ATOM submodule */
    config.alignment                 = IfxEgtm_Pwm_Alignment_centerAligned;      /* Center-aligned */
    config.syncStart                 = TRUE;                                     /* Start all channels together */
    config.frequency                 = PWM_FREQUENCY;                            /* 20 kHz */
    config.channels                  = channelConfig;                            /* Attach channel configs */
    config.numChannels               = NUM_OF_CHANNELS;                          /* 3 channels */
    config.clockSource.atom          = IfxEgtm_Atom_Ch_ClkSrc_cmuFxclk0;         /* ATOM clock from FXCLK0 */
    config.dtmClockSource            = IfxEgtm_Dtm_ClockSource_cmuClock0;        /* DTM clock source */
    config.syncUpdateEnabled         = TRUE;                                     /* Update at period end */

    /* 8) EGTM enable/clock configuration (inside enable guard) */
    if (IfxEgtm_isEnabled(&MODULE_EGTM) == FALSE)
    {
        float32 frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_enable(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);                   /* Unity divider: GCLK = module clock */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, frequency); /* Program CLK0 */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver (single call) */
    {
        boolean ok = IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &config);
        if (ok == FALSE)
        {
            /* Initialization failed; do not proceed further in this driver. */
            return;
        }
    }

    /* 10) Store initial duties and dead-times into persistent state (no immediate update here) */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (do not drive level here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}
