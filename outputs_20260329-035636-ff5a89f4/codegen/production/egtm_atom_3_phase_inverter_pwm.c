/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: EGTM ATOM 3-Phase Inverter PWM (TC4xx)
 * - eGTM Cluster 1, ATOM1, channels CH0..CH2
 * - Complementary, center-aligned PWM at 20 kHz
 * - Dead-time: 1.0 us (rising/falling)
 * - Synchronous start and update
 * - Period ISR on CPU0 with priority 20; ISR toggles a board LED
 *
 * Notes:
 * - Pin routing is performed via the IfxEgtm_Pwm_OutputConfig array entries
 * - No watchdog changes here (must be in CpuX_Main.c only)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ===== Configuration Macros ===== */
#define NUM_OF_CHANNELS              (3u)
#define PWM_FREQUENCY                (20000.0f)          /* 20 kHz */
#define ISR_PRIORITY_ATOM            (20)

/* Initial duty-cycles in percent (U,V,W) */
#define PHASE_U_DUTY                 (25.0f)
#define PHASE_V_DUTY                 (50.0f)
#define PHASE_W_DUTY                 (75.0f)

/* Duty update step (percent) */
#define PHASE_DUTY_STEP              (5.0f)

/* LED: compound macro (port, pin) for IfxPort_* APIs */
#define LED                          &MODULE_P13, 0

/* ===== TOUT Pin Mapping (User-requested pins) ===== */
/* High-side (HS) and Low-side (LS) complementary pins per phase */
#define PHASE_U_HS                   (&IfxEgtm_ATOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                   (&IfxEgtm_ATOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                   (&IfxEgtm_ATOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                   (&IfxEgtm_ATOM1_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS                   (&IfxEgtm_ATOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS                   (&IfxEgtm_ATOM1_2N_TOUT5_P02_5_OUT)

/* ===== Module State ===== */
typedef struct
{
    IfxEgtm_Pwm               pwm;                                   /* Driver handle */
    IfxEgtm_Pwm_Channel       channels[NUM_OF_CHANNELS];             /* Persistent channels buffer */
    float32                   dutyCycles[NUM_OF_CHANNELS];           /* Duty in percent */
    float32                   phases[NUM_OF_CHANNELS];               /* Phase in percent (0..100) */
    IfxEgtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];            /* Per-channel dead-time */
} EGTM_ATOM_3PHINV_State;

IFX_STATIC EGTM_ATOM_3PHINV_State g_egtmAtom3phInv;

/* ===== Private ISR and Callback ===== */
/* ISR declaration: must be at file scope, priority macro used below */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    /* Minimal ISR body per guidelines */
    IfxPort_togglePin(LED);
}

/* Period event callback: empty body (driver will invoke it) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty */
}

/* Optional private function placeholder from reference design (not used) */
void EgtmAtomPeriodIsr(void)
{
    /* Not used; ISR is interruptEgtmAtom() as per naming convention */
}

/* ===== Public API ===== */
/**
 * Initialize a 3-phase inverter PWM using EGTM1.ATOM1 CH0..CH2 with complementary outputs.
 * - Center-aligned PWM at 20 kHz, 1.0 us dead-time
 * - Synchronous start and update
 * - Period ISR on CPU0 (priority 20), ISR toggles LED
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load default configuration */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Populate output configuration for three logical channels (U,V,W) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* LS active low */
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

    /* 4) Dead-time configuration: 1.0 us for rising and falling edges */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: period event on base channel */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: three logical channels CH0..CH2, initial duties U/V/W */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig; /* base channel gets interrupt */

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

    /* 7) Main configuration */
    config.cluster            = IfxEgtm_Cluster_1;                 /* eGTM Cluster 1 */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;        /* ATOM submodule */
    config.alignment         = IfxEgtm_Pwm_Alignment_center;      /* Center-aligned */
    config.numChannels       = (uint8)NUM_OF_CHANNELS;
    config.channels          = channelConfig;
    config.frequency         = PWM_FREQUENCY;                     /* 20 kHz */
    config.clockSource.atom  = (uint32)IfxEgtm_Cmu_Clk_0;         /* ATOM clock source: CLK0 */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock source: CLK0 */
    config.syncUpdateEnabled = TRUE;                               /* Synchronous update */
    config.syncStart         = TRUE;                               /* Synchronous start */

    /* 8) Enable guard: enable module and configure CMU clocks (inside guard only) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Set GCLK to module frequency and CLK0 to same frequency */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        /* Also program 1:1 divider/count as requested by migration spec */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
        IfxEgtm_Cmu_setClkCount(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, 1u);
        /* Enable FXCLK and CLK0 */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM (routes pins, sets up DTM, binds interrupt callback) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist initial state (duties, phases, dead-times) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED as push-pull output (no forced state) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update three phase PWM duties by a fixed step (wrap to 0 before increment when reaching 100%).
 * Applies immediately to all channels with synchronous semantics preserved by the driver.
 */
void updateEgtmAtom3phInvDuty(void)
{
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
