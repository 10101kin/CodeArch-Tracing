/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production-ready EGTM ATOM (TC4xx) 3-phase complementary, center-aligned PWM driver.
 * Uses unified IfxEgtm_Pwm driver with synchronous start/update and a period-event ISR.
 *
 * Notes:
 * - No watchdog disable calls here (must be in CpuX_Main.c only).
 * - Follows iLLD init pattern: initConfig -> customize -> enable guard -> init.
 * - Uses only validated/public iLLD APIs provided.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and Configuration Constants ========================= */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY              (20000.0f)
#define ISR_PRIORITY_ATOM          (20)

#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* LED pin (P03.9) as compound macro for port+pin pair usage */
#define LED                         &MODULE_P03, 9

/* User-requested ATOM0 Cluster 0 complementary outputs (TOUT64..69 on P20.8..P20.13) */
#define PHASE_U_HS                  (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                  (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                  (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                  (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                  (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                  (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ========================= Module State ========================= */
typedef struct
{
    IfxEgtm_Pwm              pwm;                                   /* unified PWM driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];             /* persistent channels array */
    float32                  dutyCycles[NUM_OF_CHANNELS];           /* duty in percent [0..100] */
    float32                  phases[NUM_OF_CHANNELS];               /* phase in percent/ratio as per driver */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];            /* stored DTM dead-times */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback (must appear before init) ========================= */
/**
 * Interrupt Service Routine: toggles LED and returns.
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/**
 * Empty period-event callback for unified driver interrupt configuration.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API Implementations ========================= */
/**
 * Initialize a 3-channel complementary, center-aligned PWM on EGTM ATOM0 Cluster 0.
 * - Complementary outputs with 1us rising/falling dead-time
 * - Synchronous start and update enabled
 * - Period-event interrupt on base channel (Ch_0), provider CPU0, priority 20
 * - LED (P03.9) configured as push-pull output; initial state not set here
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all config structures as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Initialize the main config with defaults using the unified driver */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Build output[]: pins, complementary pins, polarity, mode, pad driver */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;  /* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;   /* LS active LOW  */
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

    /* 4) Build dtmConfig[]: 1us rising/falling dead-time for each channel */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Build interruptConfig: pulse-notify, CPU0 provider, priority 20, vmId=0, callback set only for period */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Build channelConfig[0..2] */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* period-event ISR on base channel */

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

    /* 7) Complete the main config */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;    /* ATOM uses CMU Clk_0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];

    /* 8) Enable-guard and CMU setup INSIDE the guard */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Call the unified Pwm init with persistent driver handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initial duties, phases, and dead-times into the persistent state */
    g_egtmAtom3phInv.dutyCycles[0]   = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1]   = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2]   = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0]       = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]       = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]       = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0]    = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]    = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]    = dtmConfig[2].deadTime;

    /* 11) Configure the LED GPIO as push-pull output; do not set level here */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update three PWM channel duties by a fixed step with wrap-around and apply immediately.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-around checks per channel (no loop as per spec) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    /* Unconditional increment by step */
    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately to all channels synchronously */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
