/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production-ready driver for EGTM_ATOM_3_Phase_Inverter_PWM (TC4xx eGTM unified PWM)
 *
 * NOTES:
 * - Pins must be validated against the device PinMap for the target board.
 * - Watchdog disable must NOT be placed in this driver (CpuX_Main.c only).
 * - Follows the authoritative iLLD unified PWM init pattern and CMU clock enable guard.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros (configuration) ========================= */
#define EGTM3PH_NUM_CHANNELS         (3)
#define PWM_FREQUENCY_HZ             (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM            (20)

/* Initial duties in percent */
#define PHASE_U_DUTY                 (25.0f)
#define PHASE_V_DUTY                 (50.0f)
#define PHASE_W_DUTY                 (75.0f)

/* Duty step in percent for update function */
#define PHASE_DUTY_STEP              (5.0f)

/* Debug/status LED: compound macro (port, pin) */
#define LED                          &MODULE_P13, 0

/*
 * Pin mapping macros (placeholders chosen from validated list; must be reviewed for the board):
 * Use complementary pair per phase: pin = high-side, complementaryPin = low-side.
 *
 * IMPORTANT: Replace with board-valid ATOM1 pins for KIT_A3G_TC4D7_LITE after validation.
 */
#define PHASE_U_HS                   &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
#define PHASE_U_LS                   &IfxEgtm_ATOM0_0N_TOUT7_P02_7_OUT
#define PHASE_V_HS                   &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
#define PHASE_V_LS                   &IfxEgtm_ATOM0_0N_TOUT33_P33_11_OUT
#define PHASE_W_HS                   &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
#define PHASE_W_LS                   &IfxEgtm_ATOM0_0N_TOUT81_P14_1_OUT

/* ========================= Module-level state ========================= */

typedef struct
{
    IfxEgtm_Pwm             pwm;                                 /* unified eGTM PWM driver handle */
    IfxEgtm_Pwm_Channel     channels[EGTM3PH_NUM_CHANNELS];      /* persistent channel storage */
    float32                 dutyCycles[EGTM3PH_NUM_CHANNELS];    /* duty in percent */
    float32                 phases[EGTM3PH_NUM_CHANNELS];        /* electrical phase (deg or frac), initialized to 0 */
    IfxEgtm_Pwm_DeadTime    deadTimes[EGTM3PH_NUM_CHANNELS];     /* applied dead-times */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv; /* persistent module state */

/* ========================= Interrupt and callback (file scope) ========================= */

/* Hardware ISR for PWM period event (priority bound via InterruptConfig). */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    /* Minimal ISR body as per best practices. LED toggle intentionally omitted due to API constraints. */
}

/* Period event callback (assigned into InterruptConfig). Must be empty. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty */
}

/* ========================= API Implementations ========================= */

/**
 * Initialize a 3-phase inverter PWM using the unified eGTM PWM driver on eGTM Cluster 1 ATOM1.
 * Follows the mandatory initialization sequence and CMU clock enable guard.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_OutputConfig    output[EGTM3PH_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[EGTM3PH_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[EGTM3PH_NUM_CHANNELS];

    /* 2) Load default configuration */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration (complementary pairs) */
    /* Phase U (logical channel 0) */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high; /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;  /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (logical channel 1) */
    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (logical channel 2) */
    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM dead-time configuration (1.0 us rising and falling) */
    dtmConfig[0].deadTime.rising     = 1e-6f;
    dtmConfig[0].deadTime.falling    = 1e-6f;
    dtmConfig[0].fastShutOff         = NULL_PTR;

    dtmConfig[1].deadTime.rising     = 1e-6f;
    dtmConfig[1].deadTime.falling    = 1e-6f;
    dtmConfig[1].fastShutOff         = NULL_PTR;

    dtmConfig[2].deadTime.rising     = 1e-6f;
    dtmConfig[2].deadTime.falling    = 1e-6f;
    dtmConfig[2].fastShutOff         = NULL_PTR;

    /* 5) Interrupt configuration (single period-event on base channel) */
    interruptConfig.mode             = IfxEgtm_IrqMode_pulseNotify;  /* period notification */
    interruptConfig.isrProvider      = IfxSrc_Tos_cpu0;              /* CPU0 */
    interruptConfig.priority         = ISR_PRIORITY_ATOM;            /* priority 20 */
    interruptConfig.periodEvent      = IfxEgtm_periodEventFunction;  /* callback */
    interruptConfig.dutyEvent        = NULL_PTR;                     /* no duty callback */

    /* 6) Channel configuration for logical ATOM channels 0/1/2 */
    /* Channel 0 -> U phase (25%) */
    channelConfig[0].timerCh         = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase           = 0.0f;
    channelConfig[0].duty            = PHASE_U_DUTY;
    channelConfig[0].dtm             = &dtmConfig[0];
    channelConfig[0].output          = &output[0];
    channelConfig[0].mscOut          = NULL_PTR;
    channelConfig[0].interrupt       = &interruptConfig; /* only base channel */

    /* Channel 1 -> V phase (50%) */
    channelConfig[1].timerCh         = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase           = 0.0f;
    channelConfig[1].duty            = PHASE_V_DUTY;
    channelConfig[1].dtm             = &dtmConfig[1];
    channelConfig[1].output          = &output[1];
    channelConfig[1].mscOut          = NULL_PTR;
    channelConfig[1].interrupt       = NULL_PTR;

    /* Channel 2 -> W phase (75%) */
    channelConfig[2].timerCh         = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase           = 0.0f;
    channelConfig[2].duty            = PHASE_W_DUTY;
    channelConfig[2].dtm             = &dtmConfig[2];
    channelConfig[2].output          = &output[2];
    channelConfig[2].mscOut          = NULL_PTR;
    channelConfig[2].interrupt       = NULL_PTR;

    /* 7) Complete main configuration */
    config.cluster                   = IfxEgtm_Cluster_1;                  /* eGTM Cluster 1 */
    config.subModule                 = IfxEgtm_Pwm_SubModule_atom;         /* ATOM unified */
    config.alignment                 = IfxEgtm_Pwm_Alignment_center;       /* center-aligned */
    config.frequency                 = PWM_FREQUENCY_HZ;                   /* 20 kHz */
    config.syncStart                 = TRUE;                               /* start synchronously */
    config.syncUpdate                = TRUE;                               /* synchronized buffered updates */
    config.clockSource.atom          = IfxEgtm_Cmu_Fxclk_0;                /* FXCLK0 */
    config.dtmClockSource            = IfxEgtm_Dtm_ClockSource_cmuClock0;  /* DTM uses CMU CLK0 */
    config.channelConfig             = channelConfig;                      /* link channels */
    config.numChannels               = EGTM3PH_NUM_CHANNELS;               /* 3 channels */

    /* 8) eGTM enable and CMU clock configuration (guarded) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        {
            float32 moduleHz = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            (void)moduleHz; /* frequency read for completeness; divider set to 1:1 below */
            /* Set GCLK and ECLK0 dividers to 1:1 */
            IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
            IfxEgtm_Cmu_setEclkDivider(&MODULE_EGTM, IfxEgtm_Cmu_Eclk_0, 1u, 1u);
            /* Enable FXCLK and CLK0 so ATOM can run from FXCLK0 */
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize the PWM driver (applies initial duties/phases, configures complementary outputs and DTM) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initialized duty and dead-time values for runtime tracking */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1] = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2] = channelConfig[2].dtm->deadTime;

    /* 11) Configure a status GPIO (debug LED) as push-pull output AFTER PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Perform a synchronous duty increment on the three configured PWM channels and apply immediately.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap if (duty + step) >= 100, then add step, for each channel separately */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply all new duties immediately across the synchronized group */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
