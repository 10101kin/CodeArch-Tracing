/**************************************************************************
 * @file    egtm_atom_3_phase_inverter_pwm.c
 * @brief   eGTM ATOM 3-phase inverter PWM driver (TC4xx) using IfxEgtm_Pwm
 * @details Production-ready implementation following iLLD patterns and
 *          TC3xx -> TC4xx migration to eGTM and unified PWM API.
 *
 *          - Uses IfxEgtm_Pwm unified driver for complementary PWM
 *          - Center-aligned, 20 kHz, dead-time ~1us rising/falling
 *          - SyncStart and SyncUpdate enabled
 *          - Interrupt: pulse notify, provider CPU0, ISR toggles LED
 *
 *          Watchdog disable is NOT handled here (Cpu0_Main.c only by project policy).
 **************************************************************************/

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Configuration Macros ========================= */
#define NUM_OF_CHANNELS     (3u)
#define PWM_FREQUENCY       (20000.0f)       /* 20 kHz */
#define ISR_PRIORITY_ATOM   (20)

/* User-requested pins — use only validated symbols. For unavailable ones, keep NULL_PTR placeholders. */
#define PHASE_U_HS          &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
#define PHASE_U_LS          &IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT
#define PHASE_V_HS          (NULL_PTR) /* Replace with validated P02.2 symbol when available */
#define PHASE_V_LS          (NULL_PTR) /* Replace with validated P02.3 symbol when available */
#define PHASE_W_HS          (NULL_PTR) /* Replace with validated P02.4 symbol when available */
#define PHASE_W_LS          (NULL_PTR) /* Replace with validated P02.5 symbol when available */

#define PHASE_U_DUTY        (25.0f)
#define PHASE_V_DUTY        (50.0f)
#define PHASE_W_DUTY        (75.0f)

#define PHASE_DUTY_STEP     (5.0f)

/* LED on P13.0 (compound macro: port, pin) */
#define LED                 &MODULE_P13, 0

/* ============================= Module State ============================= */
typedef struct
{
    IfxEgtm_Pwm              pwm;                              /* Driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];        /* Persistent channels (driver stores pointers) */
    float32                  dutyCycles[NUM_OF_CHANNELS];      /* In percent 0..100 */
    float32                  phases[NUM_OF_CHANNELS];          /* Phase offsets (deg or normalized) */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];       /* Dead-time configuration mirror */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ============================ ISR and Callback ========================== */
/* ISR: Toggle LED only. Priority must match ISR_PRIORITY_ATOM. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback for unified PWM driver: empty by design. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================== Functions ============================== */
/**
 * Initialize a 3-phase inverter PWM using eGTM ATOM with the unified IfxEgtm_Pwm driver.
 * - Declares all configuration structures locally
 * - Loads defaults with IfxEgtm_Pwm_initConfig
 * - Configures complementary HS/LS outputs, dead-time, interrupts, and channels
 * - Enables eGTM clocks on demand (guarded)
 * - Initializes PWM and stores initial state
 * - Configures a debug LED as push-pull output
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;                               /* Main PWM config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];       /* Per-channel config */
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];           /* Dead-time config */
    IfxEgtm_Pwm_InterruptConfig  irqCfg;                               /* Interrupt config */
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];              /* Output pin config */

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration: complementary pairs, HS active high, LS active low */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* LS active LOW  */
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

    /* 4) Dead-time configuration (~1 us for rising and falling) */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: pulse notify, CPU0 provider, base channel only */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: timerCh logical indices 0..2, phases 0.0, initial duties */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;               /* base channel interrupt */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;             /* only CH0 has interrupt */

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main driver configuration */
    config.cluster            = IfxEgtm_Cluster_0;                  /* Match ATOM0 pins */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Fxclk_0;       /* FXCLK0 for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock */
    config.syncUpdateEnabled  = TRUE;
    config.syncStart          = TRUE;

    /* 8) eGTM enable guard + CMU configuration (inside guard only) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Read module frequency dynamically (no hardcoding) */
        float32 moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        (void)moduleFreq; /* Not directly used here; retained for diagnostics */

        /* Set GCLK and ECLK0 dividers to 1:1 and enable required clocks */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
        IfxEgtm_Cmu_setEclkDivider(&MODULE_EGTM, IfxEgtm_Cmu_Eclk_0, 1u, 1u);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent handle and channel storage */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initial state for later updates */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    g_egtmAtom3phInv.deadTimes[0].rising  = 1e-6f;
    g_egtmAtom3phInv.deadTimes[0].falling = 1e-6f;
    g_egtmAtom3phInv.deadTimes[1].rising  = 1e-6f;
    g_egtmAtom3phInv.deadTimes[1].falling = 1e-6f;
    g_egtmAtom3phInv.deadTimes[2].rising  = 1e-6f;
    g_egtmAtom3phInv.deadTimes[2].falling = 1e-6f;

    /* 11) Configure debug LED GPIO as output (push-pull) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Implement a percent-based duty ramp for U, V, W and apply via immediate update.
 * - For each duty[i], if duty[i] + PHASE_DUTY_STEP >= 100, wrap duty[i] to 0 first
 * - Then increment all duty values by PHASE_DUTY_STEP
 * - Apply all three with a single immediate update call
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
