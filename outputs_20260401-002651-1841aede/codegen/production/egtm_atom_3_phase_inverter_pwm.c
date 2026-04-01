/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM ATOM0, Cluster 0: 3-phase inverter complementary PWM
 * - Center-aligned, synchronized start/update
 * - 20 kHz switching, 1 us dead-time (rising/falling)
 * - Initial duties U/V/W = 25% / 50% / 75%
 * - Period ISR on CPU0 (prio 20) toggles LED P03.9
 *
 * Notes:
 * - Uses IfxEgtm_Pwm unified driver per iLLD patterns
 * - Clocks enabled via EGTM CMU (FXCLK and CLK0)
 * - No watchdog handling here (per architecture rules)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Configuration macros (from requirements) ========================= */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY              (20000.0f)
#define ISR_PRIORITY_ATOM          (20)

#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/*
 * Pin routing macros
 * Use ONLY validated pin symbols. Where not available in the validated list, provide NULL_PTR
 * placeholders and note the intended symbol for integration.
 */
/* U-phase */
#define PHASE_U_HS                 (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT */
#define PHASE_U_LS                 (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
/* V-phase */
#define PHASE_V_HS                 (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT */
#define PHASE_V_LS                 (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT */
/* W-phase */
#define PHASE_W_HS                 (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT */
#define PHASE_W_LS                 (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT */

/* LED: P03.9 (toggle in ISR) */
#define LED                        &MODULE_P03, 9

/* ========================= Module state ========================= */
typedef struct
{
    IfxEgtm_Pwm            pwm;                                 /* Driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];           /* Persistent channels (owned by driver after init) */
    float32                dutyCycles[NUM_OF_CHANNELS];          /* Duty in percent */
    float32                phases[NUM_OF_CHANNELS];              /* Phase in percent-of-period (0..100 or 0..1 as per driver); here 0.0f */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];           /* DeadTime per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and callback (defined before init) ========================= */
/**
 * EGTM ATOM PWM Period ISR (CPU0, priority = ISR_PRIORITY_ATOM)
 * Minimal processing per best-practice: toggle LED and return.
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/**
 * Period-event callback used by the unified PWM driver
 * Body intentionally empty per design (ISR handles LED toggle)
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */
/**
 * Initialize a 3-channel center-aligned complementary PWM on EGTM ATOM0, Cluster 0
 * - Configures outputs (HS active-high, LS active-low), 1 us dead-time
 * - Sets up synchronized start/update and a period interrupt on CPU0
 * - Starts the PWM group synchronously
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all config structs as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load defaults into main config */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration: complementary PWM pairs, polarity and pad settings */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;  /* HS */
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;  /* LS (_N) */
    output[0].polarity              = Ifx_ActiveState_high;               /* HS active-high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;                /* LS active-low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;  /* HS */
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;  /* LS (_N) */
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;  /* HS */
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;  /* LS (_N) */
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: 1 us rising/falling dead-time for each channel */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: PWM period notification on CPU0, prio 20 */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction; /* callback (empty) */
    interruptConfig.dutyEvent   = NULL_PTR;                    /* not used */

    /* 6) Channel configuration: logical channel indices 0/1/2, phase 0.0 for all */
    /* Base channel (CH0) holds the interrupt */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;          /* Phase U */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;          /* Phase V */
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;              /* only base channel has interrupt */

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;          /* Phase W */
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main config fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;               /* ATOM clock source: CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM clock source: CMU CLK0 */
    config.syncUpdateEnabled  = TRUE;                                     /* Shadow-to-active at period */
    config.syncStart          = TRUE;                                     /* Start all channels together */

    /* 8) Enable-guard: enable EGTM and configure CMU clocks (FXCLK + CLK0) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Dynamically read module frequency; configure GCLK and CLK0 accordingly */
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            /* Per available API signatures: setGclkFrequency(numerator, denominator) */
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, (uint32)freq, (uint32)1);
            /* Configure CMU CLK0 using same frequency/count */
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, (uint32)freq);
            /* Enable FXCLK and CLK0 */
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize the PWM driver (applies routing, timing, dead-time, ISR, and starts outputs) */
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

    /* 11) Configure LED GPIO as output (push-pull) after PWM init; do not force level */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update all three channel duties by a fixed increment with wrap at 100%.
 * Applies the update atomically at shadow-to-active transfer using the driver's immediate API.
 */
void updateEgtmAtom3phInvDuty(void)
{
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply the duty update atomically to all channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
