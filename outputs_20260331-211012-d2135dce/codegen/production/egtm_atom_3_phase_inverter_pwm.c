/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production-ready EGTM ATOM 3-Phase Inverter PWM driver for TC4xx
 * - Uses unified IfxEgtm_Pwm driver (ATOM submodule)
 * - 3 complementary, center-aligned channels (U,V,W)
 * - 20 kHz, 1 us rising/falling dead-time
 * - Sync start and sync update enabled
 * - Period-event ISR on channel 0 toggles LED
 *
 * Notes:
 * - Watchdog disable is NOT handled here (must be in CpuX_Main.c)
 * - No STM-based timing here; scheduling belongs to CpuX_Main.c
 */

/* Selected iLLD headers (transitively include required dependencies) */
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

#include "egtm_atom_3_phase_inverter_pwm.h"

/* ========================= Configuration Macros ========================= */
#define NUM_OF_CHANNELS        (3U)
#define PWM_FREQUENCY          (20000.0f)      /* 20 kHz */
#define ISR_PRIORITY_ATOM      (20)

#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)
#define PHASE_DUTY_STEP        (10.0f)

/* LED pin: P03.9 (compound macro: port, pin) */
#define LED                    &MODULE_P03, 9

/* User-requested ATOM0 Cluster0 TOUT mappings (complementary pairs) */
#define PHASE_U_HS             (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS             (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS             (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS             (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS             (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS             (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ========================= Module State ========================= */
typedef struct
{
    IfxEgtm_Pwm             pwm;                                   /* Driver handle */
    IfxEgtm_Pwm_Channel     channels[NUM_OF_CHANNELS];             /* Persistent channel data */
    float32                 dutyCycles[NUM_OF_CHANNELS];           /* Duty in percent [0..100] */
    float32                 phases[NUM_OF_CHANNELS];               /* Phase in degrees or fraction per driver definition */
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];            /* Dead-times per channel */
} EGTM_ATOM_3PhInv_State;

IFX_STATIC EGTM_ATOM_3PhInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback Prototypes ========================= */
/**
 * Period-event ISR: toggles LED. Priority/provider configured via InterruptConfig.
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/**
 * Period-event callback hook used by the unified driver. Intentionally empty.
 */
void IfxEgtm_periodEventFunction(void *data);

/* ========================= ISR and Callback Implementations ========================= */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty: event is serviced by hardware ISR */
}

void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API Implementations ========================= */
/**
 * Initialize a 3-channel complementary, center-aligned PWM using the unified eGTM PWM driver
 * with synchronous start/update and a period-event interrupt on the base channel.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all config structures as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Initialize the main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Build output[] for U, V, W phases */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;            /* HS active-high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;             /* LS active-low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Build dtmConfig[]: 1 us rising/falling dead-time on each channel */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Build interrupt configuration for base channel (period event) */
    interruptConfig.mode         = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider  = IfxSrc_Tos_cpu0;
    interruptConfig.priority     = ISR_PRIORITY_ATOM;
    interruptConfig.vmId         = IfxSrc_VmId_0;
    interruptConfig.periodEvent  = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent    = NULL_PTR;

    /* 6) Build channelConfig[0..2] */
    channelConfig[0].timerCh     = IfxEgtm_Pwm_SubModule_Ch_0;   /* Phase U */
    channelConfig[0].phase       = 0.0f;
    channelConfig[0].duty        = PHASE_U_DUTY;
    channelConfig[0].dtm         = &dtmConfig[0];
    channelConfig[0].output      = &output[0];
    channelConfig[0].mscOut      = NULL_PTR;
    channelConfig[0].interrupt   = &interruptConfig;             /* Period ISR on base channel */

    channelConfig[1].timerCh     = IfxEgtm_Pwm_SubModule_Ch_1;   /* Phase V */
    channelConfig[1].phase       = 0.0f;
    channelConfig[1].duty        = PHASE_V_DUTY;
    channelConfig[1].dtm         = &dtmConfig[1];
    channelConfig[1].output      = &output[1];
    channelConfig[1].mscOut      = NULL_PTR;
    channelConfig[1].interrupt   = NULL_PTR;                     /* No ISR on ch1 */

    channelConfig[2].timerCh     = IfxEgtm_Pwm_SubModule_Ch_2;   /* Phase W */
    channelConfig[2].phase       = 0.0f;
    channelConfig[2].duty        = PHASE_W_DUTY;
    channelConfig[2].dtm         = &dtmConfig[2];
    channelConfig[2].output      = &output[2];
    channelConfig[2].mscOut      = NULL_PTR;
    channelConfig[2].interrupt   = NULL_PTR;                     /* No ISR on ch2 */

    /* 7) Complete main config */
    config.cluster               = IfxEgtm_Cluster_0;
    config.subModule             = IfxEgtm_Pwm_SubModule_atom;
    config.alignment             = IfxEgtm_Pwm_Alignment_center;
    config.syncStart             = TRUE;
    config.syncUpdateEnabled     = TRUE;
    config.frequency             = PWM_FREQUENCY;
    config.clockSource.atom      = IfxEgtm_Cmu_Clk_0;            /* ATOM uses CMU CLK0 */
    config.dtmClockSource        = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.numChannels           = (uint8)NUM_OF_CHANNELS;
    config.channels              = &channelConfig[0];

    /* 8) Enable-guard and CMU setup INSIDE the guard */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize unified PWM driver with persistent handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Persist initial duties, phases and dead-times to module state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]     = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (no initial level change) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three logical PWM channels' duty cycles by a fixed step with wrap-around at 100%,
 * and apply immediately in one bulk call.
 */
void updateEgtmAtom3phInvDuty(void)
{
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
