/**
 * @file egtm_atom_3_phase_inverter_pwm.c
 * @brief Production driver for eGTM ATOM 3-phase inverter PWM using unified IfxEgtm_Pwm (TC4xx)
 *
 * - Generates six complementary, center-aligned PWM outputs at 20 kHz
 * - Dead-time: rising=1e-6 s, falling=1e-6 s
 * - Initial duties: U=25%, V=50%, W=75%
 * - HS active-high, LS active-low
 * - Push-pull, cmosAutomotiveSpeed1
 * - syncStart and syncUpdate enabled
 * - Uses InterruptConfig with period-event callback and ISR toggling a debug LED
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and configuration constants ========================= */
#define NUM_OF_CHANNELS        (3u)
#define PWM_FREQUENCY          (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM      (20)

#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)

/* Duty ramp step in percent */
#define PHASE_DUTY_STEP        (5.0f)

/* Debug LED: compound macro (port, pin) */
#define LED &MODULE_P13, 0

/* ========================= Pin assignment macros (validated list only) ========================= */
/* User-requested U phase (validated): */
#define PHASE_U_HS             (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS             (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)

/* V, W requested pins are not present in the validated subset; use NULL_PTR placeholders. */
#define PHASE_V_HS             (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_y_TOUT2_P02_2_OUT when validated */
#define PHASE_V_LS             (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_yN_TOUT3_P02_3_OUT when validated */
#define PHASE_W_HS             (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_y_TOUT4_P02_4_OUT when validated */
#define PHASE_W_LS             (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_yN_TOUT5_P02_5_OUT when validated */

/* ========================= Module persistent state ========================= */
typedef struct
{
    IfxEgtm_Pwm            pwm;                              /* Driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];        /* Channel runtime data (must persist) */
    float32                dutyCycles[NUM_OF_CHANNELS];      /* Duty cycles in percent (0..100) */
    float32                phases[NUM_OF_CHANNELS];          /* Phase offsets in degrees/fraction (implementation-defined) */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];       /* Stored dead-time configuration */
} EgtmAtom3phInv_State;

/* Use IFX_STATIC per architectural requirement */
IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= Forward declarations (ISR and callback) ========================= */
/* ISR: toggles LED on PWM period interrupt. Priority defined by ISR_PRIORITY_ATOM. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/* Period-event callback: must have empty body. Assigned in InterruptConfig. */
void IfxEgtm_periodEventFunction(void *data);

/* ========================= Private functions ========================= */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty per design */
}

void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API implementations ========================= */
/**
 * @brief Initialize a 3-phase inverter PWM using eGTM ATOM with unified IfxEgtm_Pwm.
 *
 * Follows the mandatory iLLD initialization pattern and structural rules:
 *  - Declares local config structures and arrays
 *  - Loads defaults via IfxEgtm_Pwm_initConfig
 *  - Configures complementary outputs, dead-time, interrupts
 *  - Sets center alignment, syncStart, syncUpdate, 20 kHz frequency
 *  - Performs enable guard with CMU configuration inside
 *  - Calls IfxEgtm_Pwm_init using persistent state handles
 *  - Stores initial duties and dead-times into persistent state
 *  - Configures a debug LED as output; ISR toggles this LED on each period
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration structures locally */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration for three logical channels (U, V, W) */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high; /* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;  /* LS active LOW  */
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

    /* 4) Dead-time configuration for all channels (1e-6 s rising/falling) */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: pulse notify, CPU0, defined priority; only base channel uses it */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (ordinal logical indices 0..2) */
    /* Channel 0 -> Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY; /* percent */
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;      /* base channel only */

    /* Channel 1 -> Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY; /* percent */
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;     /* no separate interrupt */

    /* Channel 2 -> Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY; /* percent */
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;     /* no separate interrupt */

    /* 7) Main configuration fields */
    config.egtmSFR           = &MODULE_EGTM;
    config.cluster           = IfxEgtm_Cluster_0;                 /* ATOM0 on Cluster 0 */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;        /* Use ATOM */
    config.alignment         = IfxEgtm_Pwm_Alignment_center;      /* Center-aligned */
    config.numChannels       = (uint8)NUM_OF_CHANNELS;
    config.channels          = channelConfig;
    config.frequency         = PWM_FREQUENCY;                     /* Hz */
    config.clockSource.atom  = (uint32)IfxEgtm_Cmu_Fxclk_0;       /* Clock source selector (union .atom) */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock */
    config.syncUpdateEnabled = TRUE;                               /* Atomic update on period */
    config.syncStart         = TRUE;                               /* Start all together */

    /* 8) Enable guard: enable module and CMU inside guard only */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        /* Additionally configure 1:1 dividers as per migration requirement */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
        IfxEgtm_Cmu_setEclkDivider(&MODULE_EGTM, IfxEgtm_Cmu_Eclk_0, 1u, 1u);
    }

    /* 9) Initialize PWM with persistent handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store initial duties, phases, and dead-times into persistent state */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure debug LED GPIO as output (ISR toggles this) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Percent-based duty ramp for U, V, W. Wrap at 100 then increment, apply immediate update.
 *
 * No timing logic here; periodic scheduling is handled externally (e.g., Cpu0_Main).
 */
void updateEgtmAtom3phInvDuty(void)
{
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply all three duties atomically on next latch using Immediate API */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
