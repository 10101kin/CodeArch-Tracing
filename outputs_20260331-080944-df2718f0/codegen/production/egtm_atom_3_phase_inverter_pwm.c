/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production EGTM ATOM 3-Phase complementary, center-aligned PWM driver (TC4xx)
 * - Migrated from GTM to eGTM (TC3xx -> TC4xx)
 * - Uses IfxEgtm_Pwm unified high-level driver
 * - Submodule: ATOM (Cluster 0)
 *
 * Notes:
 * - Watchdog disable must be done only in CpuX_Main.c (not here)
 * - Interrupt is routed internally by the driver using InterruptConfig
 * - This module does not perform any time-based scheduling (no STM waits)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (numeric constants from requirements) ========================= */
#define EGTM_ATOM_NUM_CHANNELS   (3u)
#define PWM_FREQUENCY            (20000.0f)     /* Hz */
#define ISR_PRIORITY_ATOM        (20)
#define PHASE_U_DUTY             (25.0f)        /* percent */
#define PHASE_V_DUTY             (50.0f)        /* percent */
#define PHASE_W_DUTY             (75.0f)        /* percent */
#define PHASE_DUTY_STEP          (10.0f)        /* percent */

/* LED macro: expand to two arguments for IfxPort_* APIs */
#define LED                      &MODULE_P03, 9  /* P03.9, active-low on KIT_A3G_TC4D7_LITE */

/* ========================= Pin selection ========================= */
/*
 * Use ONLY validated pin symbols from the provided PinMap lists.
 * The user-requested pins (P20.8/TOUT64, P20.9/TOUT65, P21.4/TOUT55, P20.11/TOUT67, P20.12/TOUT68, P20.13/TOUT69)
 * are not present in the validated snippet list above. To avoid inventing symbols, leave them as NULL_PTR here.
 * Integrators should replace NULL_PTR with the correct IfxEgtm_ATOMx_*_TOUT*_Pxx_y_OUT symbols from their BSP.
 */
#define PHASE_U_HS               (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT64_P20_8_OUT for U high-side */
#define PHASE_U_LS               (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT65_P20_9_OUT for U low-side */
#define PHASE_V_HS               (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT55_P21_4_OUT for V high-side */
#define PHASE_V_LS               (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT67_P20_11_OUT for V low-side */
#define PHASE_W_HS               (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT68_P20_12_OUT for W high-side */
#define PHASE_W_LS               (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT69_P20_13_OUT for W low-side */

/* ========================= Module state ========================= */
typedef struct
{
    IfxEgtm_Pwm                 pwm;                                        /* Driver handle */
    IfxEgtm_Pwm_Channel         channels[EGTM_ATOM_NUM_CHANNELS];           /* Persistent channel data (owned by driver) */
    float32                     dutyCycles[EGTM_ATOM_NUM_CHANNELS];         /* Percent [0..100] */
    float32                     phases[EGTM_ATOM_NUM_CHANNELS];             /* Phase offset [0..1.0] */
    IfxEgtm_Pwm_DeadTime        deadTimes[EGTM_ATOM_NUM_CHANNELS];          /* Dead-time per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback ========================= */
/**
 * EGTM ATOM period-event ISR (base channel). The driver routes to this ISR using InterruptConfig.
 * Body must be minimal; toggle the debug LED only.
 */
IFX_INTERRUPT(EgtmAtomIsr, 0, ISR_PRIORITY_ATOM);
void EgtmAtomIsr(void)
{
    IfxPort_togglePin(LED);
}

/**
 * Period event callback used by the IfxEgtm_Pwm driver. Must be empty by design.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Initialization ========================= */
/**
 * Initialize EGTM ATOM0 Cluster 0 3-phase complementary center-aligned PWM at 20 kHz.
 * - 3 channels (U/V/W), complementary low-side with 1 us dead-time (rising/falling)
 * - Initial duties: U=25%, V=50%, W=75%
 * - syncStart and syncUpdate enabled
 * - Clock domain: FXCLK0 for ATOM, DTM clock from CMU Clock0
 * - Period-event interrupt on base channel (channel 0), prio=20, CPU0 TOS, vmId 0
 * - LED P03.9 configured as push-pull output (not driven here)
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as locals */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[EGTM_ATOM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[EGTM_ATOM_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;
    IfxEgtm_Pwm_OutputConfig      output[EGTM_ATOM_NUM_CHANNELS];

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration: high-side + complementary low-side, polarity and pad driver */
    /* Phase U */
    output[0].pin                     = PHASE_U_HS;
    output[0].complementaryPin        = PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;          /* HS active-high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;           /* LS active-low  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase V */
    output[1].pin                     = PHASE_V_HS;
    output[1].complementaryPin        = PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase W */
    output[2].pin                     = PHASE_W_HS;
    output[2].complementaryPin        = PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us rising and falling for each channel */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;
    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;
    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration for base channel (channel 0) */
    interruptConfig.mode        = IfxEgtm_Pwm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = &IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: contiguous logical indices starting at SubModule_Ch_0 */
    /* Base channel 0 (interrupt attached) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;
    /* Channel 1 */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;
    /* Channel 2 */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Complete main config */
    config.cluster             = IfxEgtm_Cluster_0;                 /* Target cluster index 0 */
    config.subModule           = IfxEgtm_SubModule_atom;            /* ATOM submodule */
    config.alignment           = IfxEgtm_Pwm_Alignment_center;      /* Center-aligned */
    config.syncStart           = TRUE;                              /* Sync start for all channels */
    config.syncUpdateEnabled   = TRUE;                              /* Transfer at period end */
    config.frequency           = PWM_FREQUENCY;                     /* 20 kHz */
    config.numChannels         = EGTM_ATOM_NUM_CHANNELS;
    config.channels            = channelConfig;                     /* Channel configurations */
    config.clockSource.atom    = IfxEgtm_Atom_Ch_ClkSrc_cmuFxclk0;  /* ATOM from FXCLK0 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock source */

    /* 8) Enable guard: enable EGTM and configure CMU clocks if not already enabled */
    if (IfxEgtm_isEnabled(&MODULE_EGTM) == FALSE)
    {
        IfxEgtm_enable(&MODULE_EGTM);
        IfxEgtm_Cmu_enable(&MODULE_EGTM);

        float32 modFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Run GCLK at module frequency (unity divider) */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, modFreq);
        /* Program CLK0 as functional clock domain */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, modFreq);
        /* Enable clocks: FXCLK and CLK0 (enable all if mask details are abstracted) */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, 0xFFFFFFFFu);
    }

    /* 9) Initialize PWM (single call) with persistent channels array in the state */
    /* The driver internally binds to g_egtmAtom3phInv.channels from the handle */
    {
        boolean ok = IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &config);
        if (ok == TRUE)
        {
            /* 10) Store initial duty, phase, and dead-time into persistent module state */
            g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
            g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
            g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

            g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
            g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
            g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

            g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
            g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
            g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;
        }
        else
        {
            /* Initialization failed: leave state arrays unmodified; no further action here */
        }
    }

    /* 11) Configure LED GPIO as push-pull output (do not drive level here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
