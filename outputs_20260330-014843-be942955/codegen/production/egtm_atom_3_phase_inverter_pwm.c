/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: TC4xx eGTM ATOM 3-phase complementary, center-aligned PWM inverter
 * Submodule: ATOM0 Cluster0
 * PWM frequency: 20 kHz
 * Dead-time: 1 us rising / 1 us falling
 * Pins (requested):
 *   U: HS=P20.8 (TOUT64), LS=P20.9 (TOUT65)
 *   V: HS=P21.4 (TOUT55), LS=P20.11 (TOUT67)
 *   W: HS=P20.12 (TOUT68), LS=P20.13 (TOUT69)
 * LED (period-event ISR debug): P03.9
 *
 * Notes:
 * - High-level driver: IfxEgtm_Pwm
 * - Atomic duty updates via IfxEgtm_Pwm_updateChannelsDutyImmediate
 * - All CMU clock configuration is inside the eGTM enable guard as mandated.
 * - No watchdog handling here (must be in CpuX_Main.c).
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration macros (constants)
 * ============================= */
#define NUM_OF_CHANNELS           (3)
#define PWM_FREQUENCY             (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM         (20)

#define PHASE_U_DUTY              (25.0f)
#define PHASE_V_DUTY              (50.0f)
#define PHASE_W_DUTY              (75.0f)
#define PHASE_DUTY_STEP           (10.0f)

/* LED macro (compound arg form: port, pin) */
#define LED                        &MODULE_P03, 9

/* =============================
 * Pin routing macros
 * Use ONLY validated pin symbols. For pins not present in the validated list,
 * provide NULL_PTR placeholders with integration comments.
 * ============================= */
/* U phase */
#define PHASE_U_HS                (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT64_P20_8_OUT when available in PinMap */
#define PHASE_U_LS                (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
/* V phase */
#define PHASE_V_HS                (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT55_P21_4_OUT when available in PinMap */
#define PHASE_V_LS                (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT67_P20_11_OUT when available in PinMap */
/* W phase */
#define PHASE_W_HS                (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT68_P20_12_OUT when available in PinMap */
#define PHASE_W_LS                (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT69_P20_13_OUT when available in PinMap */

/* =============================
 * Module state (persistent)
 * ============================= */
typedef struct
{
    IfxEgtm_Pwm              pwm;                                 /* PWM driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];           /* Persistent channel objects (driver writes here) */
    float32                   dutyCycles[NUM_OF_CHANNELS];         /* Duty in percent for U, V, W */
    float32                   phases[NUM_OF_CHANNELS];             /* Phase offsets in cycles */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];          /* Dead-time settings per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =============================
 * Private ISR and callback (declared BEFORE init)
 * ============================= */

/* Period-event callback: empty body as required */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR: toggle LED only (minimal ISR) */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* =============================
 * Public API implementations
 * ============================= */

/**
 * Initialize a 3-channel complementary, center-aligned inverter PWM using eGTM ATOM.
 * Algorithm/order strictly follows the SW detailed design and AURIX iLLD patterns.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Initialize the main config with driver defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration for each channel (HS + complementary LS) */
    /* U phase */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;  /* HS active-high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;   /* LS active-low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM dead-time per channel: 1 us rising, 1 us falling */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff = NULL_PTR;

    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff = NULL_PTR;

    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt parameters for period event */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2 with U/V/W initial duties */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig; /* base channel only */

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

    /* 7) Main PWM configuration fields */
    config.cluster            = IfxEgtm_Cluster_0;                     /* ATOM0 Cluster0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;            /* ATOM */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;          /* center-aligned */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                      /* ATOM clock source */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;     /* DTM from CMU.CLK0 */
    config.syncUpdateEnabled  = TRUE;                                   /* synchronized updates */
    config.syncStart          = TRUE;                                   /* synchronized start */

    /* 8) eGTM enable guard and CMU clock configuration (MANDATORY pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Set global and ATOM clock frequencies to module frequency (no division) */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        /* Enable FXCLK and CLK0 (ATOM uses CLK0; FXCLK enabled per mandatory pattern) */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver with persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist initial duties, phases, and dead-times for later updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO (no explicit set high/low) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Step each phase duty by PHASE_DUTY_STEP with wrap-around and apply atomically.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-around checks (exact sequence: check -> maybe reset -> unconditional add) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply new duties atomically to all channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
