/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM_ATOM_3_Phase_Inverter_PWM (TC4xx, eGTM ATOM unified IfxEgtm_Pwm)
 * - Migration: TC3xx GTM TOM IfxGtm_Pwm -> TC4xx eGTM ATOM IfxEgtm_Pwm
 * - Cluster 1, Submodule ATOM1, channels 0/1/2
 * - Complementary outputs with DTM dead-time insertion (1.0 us rise/fall)
 * - Center-aligned 20 kHz PWM, FXCLK0 clock source
 * - Sync start and sync update enabled
 * - Period-event interrupt on channel 0 (CPU0, prio 20) with empty period callback
 *
 * Notes:
 * - Pins must be validated on the target board (KIT_A3G_TC4D7_LITE). Here, valid TC4xx eGTM ATOM
 *   TOUT symbols are used from the provided validated list. Update as needed per board schematic.
 * - Watchdog control must not be placed here (handled only in CpuX_Main.c per AURIX standard).
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD core types and selected drivers */
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Configuration Macros (from requirements) ========================= */
#define NUM_OF_CHANNELS                 (3)
#define PWM_FREQUENCY_HZ                (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM               (20)

/* Initial duties in percent (phase naming U/V/W) */
#define PHASE_U_DUTY                    (25.0f)
#define PHASE_V_DUTY                    (50.0f)
#define PHASE_W_DUTY                    (75.0f)

/* Step for duty update (percent) */
#define PHASE_DUTY_STEP                 (5.0f)

/* LED debug pin (compound macro: port, pin) */
#define LED                             &MODULE_P13, 0

/* ========================= Pin Mapping (validated symbols list provided) ==================== */
/*
 * NOTE: The following pin selections use only validated symbols from the provided list.
 * They must be reviewed and adapted to match ATOM1 routing on the target device/board.
 * Each complementary pair uses a high-side (pin) and low-side (complementaryPin).
 */
#define PHASE_U_HS                      &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
#define PHASE_U_LS                      &IfxEgtm_ATOM0_0N_TOUT7_P02_7_OUT
#define PHASE_V_HS                      &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
#define PHASE_V_LS                      &IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT
#define PHASE_W_HS                      &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
#define PHASE_W_LS                      &IfxEgtm_ATOM0_0N_TOUT33_P33_11_OUT

/* ========================= Module State ===================================================== */

typedef struct
{
    IfxEgtm_Pwm             pwm;                              /* unified eGTM PWM driver handle */
    IfxEgtm_Pwm_Channel     channels[NUM_OF_CHANNELS];        /* persistent channel storage */
    float32                 dutyCycles[NUM_OF_CHANNELS];      /* duty in percent */
    float32                 phases[NUM_OF_CHANNELS];          /* phase in percent/deg (unused here, reserved) */
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];       /* per-channel dead-time settings */
} EgtmAtom3phInv_State;

/* Persistent driver state (must use IFX_STATIC per coding rules) */
IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= Internal ISR and Callback ======================================== */

/* Period event callback (assigned in InterruptConfig) - MUST be empty body */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR declaration: priority must match ISR_PRIORITY_ATOM; body intentionally minimal */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    /* Intentionally left minimal. LED toggle, if desired, can be implemented when GPIO helpers are available. */
}

/* ========================= Public API ======================================================= */

/**
 * Initialize a 3-phase inverter PWM using the unified eGTM PWM driver on eGTM Cluster 1 ATOM1 with three logical
 * channels (0, 1, 2). Follows the mandatory initialization and CMU clock enable patterns.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure complementary outputs (active-high HS, active-low LS; push-pull; automotive pad driver) */
    output[0].pin                     = PHASE_V_HS;  /* Channel 0 -> V phase (per requirement example) */
    output[0].complementaryPin        = PHASE_V_LS;
    output[0].polarity                = Ifx_ActiveState_high;
    output[0].complementaryPolarity   = Ifx_ActiveState_low;
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                     = PHASE_U_HS;  /* Channel 1 -> U phase */
    output[1].complementaryPin        = PHASE_U_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                     = PHASE_W_HS;  /* Channel 2 -> W phase */
    output[2].complementaryPin        = PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Configure DTM dead-time: 1.0 us rising/falling on all channels */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: single period-event interrupt on base channel only */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify; /* period notification */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;             /* CPU0 */
    interruptConfig.priority    = ISR_PRIORITY_ATOM;           /* priority 20 */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction; /* empty callback */
    interruptConfig.dutyEvent   = NULL_PTR;                    /* no duty callback */

    /* 6) Channel configs: logical ordinal indices 0,1,2 with initial duties and zero phase */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;  /* V phase */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_V_DUTY;                /* 50% */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;            /* period event on base channel */

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;  /* U phase */
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_U_DUTY;                /* 25% */
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;                    /* no interrupt on ch1 */

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;  /* W phase */
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;                /* 75% */
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;                    /* no interrupt on ch2 */

    /* 7) Main config: eGTM Cluster 1, ATOM submodule, center alignment, sync start/update, FXCLK0 */
    config.cluster             = IfxEgtm_Cluster_1;
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;        /* ATOM */
    config.alignment           = IfxEgtm_Pwm_Alignment_center;      /* center aligned */
    config.syncStart           = TRUE;                              /* start coherently */
    config.syncUpdateEnabled   = TRUE;                              /* buffered sync updates */
    config.frequency           = PWM_FREQUENCY_HZ;                  /* 20 kHz */
    config.clockSource.atom    = IfxEgtm_Cmu_Fxclk_0;               /* ATOM clock source: FXCLK0 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock */
    config.channelConfig       = &channelConfig[0];                 /* attach channel list */
    config.numChannels         = NUM_OF_CHANNELS;                   /* 3 phases */

    /* 8) eGTM clock enable block (guarded) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        (void)IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM); /* read and keep for potential scaling if needed */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1, 1);     /* GCLK = module clock */
        IfxEgtm_Cmu_setEclkDivider(&MODULE_EGTM, IfxEgtm_Cmu_Eclk_0, 1, 1); /* ECLK0 = module clock */
        /* Enable FXCLK and ECLK0 so ATOM can run from FXCLK0 */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_ECLK0));
    }

    /* 9) Initialize the PWM peripheral (driver stores pointer to persistent channels) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initialized duty and dead-time in state for runtime tracking */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure a status GPIO (LED/debug) as push-pull output AFTER PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Perform a synchronous duty increment on the three configured PWM channels.
 * Wrap when (duty + PHASE_DUTY_STEP) >= 100, then add PHASE_DUTY_STEP unconditionally.
 * Apply new duties immediately across the synchronized group.
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
