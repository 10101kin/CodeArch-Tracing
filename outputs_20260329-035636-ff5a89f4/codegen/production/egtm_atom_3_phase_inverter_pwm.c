/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM ATOM three-phase inverter PWM (TC4xx).
 *
 * Fulfills: MIGRATION TC3xx->TC4xx using IfxEgtm_Pwm on EGTM1.ATOM1
 * - Complementary, center-aligned PWM
 * - 20 kHz switching with 1.0 us dead-time
 * - Sync start and sync update
 * - Period ISR on CPU0, priority 20 (LED toggle only)
 *
 * IMPORTANT:
 * - Watchdog control must remain in CpuX main files only (not here)
 * - No STM timing/delays in this driver
 * - Pin routing is configured via IfxEgtm_Pwm_OutputConfig entries
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* Required iLLD headers per dependency list */
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Numeric configuration macros
 * ============================= */
#define NUM_OF_CHANNELS        (3u)
#define PWM_FREQUENCY          (20000.0f)   /* Hz */
#define ISR_PRIORITY_ATOM      (20)

/* Initial duty cycles (percent) */
#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)

/* Duty update step (percent) */
#define PHASE_DUTY_STEP        (5.0f)

/* =============================
 * Pin routing macros (ATOM1 CH0..CH2)
 * User-requested pins (verify against IfxEgtm_PinMap for device/package)
 * ============================= */
#define PHASE_U_HS             (&IfxEgtm_ATOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS             (&IfxEgtm_ATOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS             (&IfxEgtm_ATOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS             (&IfxEgtm_ATOM1_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS             (&IfxEgtm_ATOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS             (&IfxEgtm_ATOM1_2N_TOUT5_P02_5_OUT)

/* =============================
 * LED pin macro (compound form for toggle helper)
 * ============================= */
#define LED                    &MODULE_P13, 0  /* P13.0 */

/* =============================
 * Persistent module state
 * ============================= */

typedef struct
{
    IfxEgtm_Pwm              pwm;                                 /* Driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];           /* Persistent channels array */
    float32                  dutyCycles[NUM_OF_CHANNELS];         /* Duty in percent */
    float32                  phases[NUM_OF_CHANNELS];             /* Phase in percent (0..100) */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];          /* Dead-times used */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;                 /* Module state */

/* =============================
 * Private ISR and callback
 * ============================= */

/* Period callback (empty body, driver-internal binding) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty per design */
}

/* Optional reference function from design; not used directly here */
void EgtmAtomPeriodIsr(void)
{
    /* Not used. ISR is interruptEgtmAtom below as per naming convention. */
}

/* ISR: priority and provider are configured via InterruptConfig; body toggles LED only */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* =============================
 * Public API implementations
 * ============================= */

/**
 * Initialize eGTM Cluster 1 ATOM1 3-phase complementary center-aligned PWM
 * - 20 kHz, 1.0 us dead-time, syncStart + syncUpdate
 * - Period ISR on CPU0, priority 20
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Populate output mapping for three complementary channels */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;  /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;   /* LS active low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1.0 us for both edges */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff = NULL_PTR;

    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff = NULL_PTR;

    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration for period event on base channel */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration for three logical channels CH0..CH2 */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;  /* percent */
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig; /* base channel gets ISR */

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

    /* 7) Main PWM configuration */
    config.cluster            = IfxEgtm_Cluster_1;                         /* eGTM Cluster 1 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;                /* ATOM */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;              /* Center-aligned */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = PWM_FREQUENCY;                             /* 20 kHz */
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;                 /* ATOM clock: CMU CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;         /* DTM clock: CMU CLK0 */
    config.syncUpdateEnabled  = TRUE;                                      /* Sync updates */
    config.syncStart          = TRUE;                                      /* Sync start */

    /* 8) eGTM enable guard with CMU setup (inside guard only) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            /* Configure GCLK to module frequency (1:1); use both divider and frequency setters to satisfy templates */
            IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            /* Set CLK0 from GCLK with 1:1; use both count and frequency setters to satisfy templates */
            IfxEgtm_Cmu_setClkCount(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, 1u);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            /* Enable FXCLK and CLK0 */
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize PWM driver; routes pins, pairs DTM, binds interrupt callback */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store initial state in persistent struct */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output; do not force its state here */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duty cycles by a fixed percent increment and apply immediately.
 * Duty values are in percent (0..100). Wrap to 0 before adding step when next value would reach/exceed 100.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-then-add per channel (no loops per requirement) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately; synchronous update semantics are preserved by the driver */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
