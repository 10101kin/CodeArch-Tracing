/**
 * @file egtm_atom_3_phase_inverter_pwm.c
 * @brief eGTM ATOM 3-phase inverter PWM driver (TC4xx, migration TC3xx->TC4xx).
 *
 * Implementation notes:
 * - Uses unified IfxEgtm_Pwm high-level driver and ATOM submodule.
 * - Follows mandatory initialization, clock enable guard, and array-based pin/DTM configuration.
 * - No watchdog handling here (must be in CpuX_Main.c only).
 * - No STM timing or PinMap_set calls are used.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* Required iLLD headers (only the selected ones per module rules) */
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and constants ========================= */

/* Channel and timing configuration */
#define NUM_OF_CHANNELS        (3u)
#define PWM_FREQUENCY          (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM      (20)

/* Initial duty cycles in percent */
#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)

/* Duty ramp step in percent for update routine */
#define PHASE_DUTY_STEP        (5.0f)

/* User-requested pin assignments (use only validated symbols; placeholders where unavailable) */
#define PHASE_U_HS             (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS             (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)
#define PHASE_V_HS             (NULL_PTR) /* Replace with validated symbol, e.g., &IfxEgtm_ATOM0_x_TOUT2_P02_2_OUT */
#define PHASE_V_LS             (NULL_PTR) /* Replace with validated symbol, e.g., &IfxEgtm_ATOM0_xN_TOUT3_P02_3_OUT */
#define PHASE_W_HS             (NULL_PTR) /* Replace with validated symbol, e.g., &IfxEgtm_ATOM0_x_TOUT4_P02_4_OUT */
#define PHASE_W_LS             (NULL_PTR) /* Replace with validated symbol, e.g., &IfxEgtm_ATOM0_xN_TOUT5_P02_5_OUT */

/* Debug LED (compound macro: port, pin) */
#define LED                    &MODULE_P13, 0

/* ========================= Module state ========================= */

typedef struct
{
    IfxEgtm_Pwm            pwm;                               /* Driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];         /* Persistent channel data */
    float32                dutyCycles[NUM_OF_CHANNELS];       /* Duty in percent [0..100] */
    float32                phases[NUM_OF_CHANNELS];           /* Phase offset in percent [0..100] */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];        /* Deadtime values per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv = {0};

/* ========================= ISR and callback ========================= */

/* Period event callback (assigned in InterruptConfig). Body must be empty. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR: Toggle debug LED only. The high-level driver routes interrupts internally. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API ========================= */

/**
 * @brief Initialize a 3-phase inverter PWM using eGTM ATOM with the unified IfxEgtm_Pwm driver.
 *
 * Behavior summary:
 * - Declares local config structures and arrays (main, per-channel, DTM, interrupt, output).
 * - Loads defaults via IfxEgtm_Pwm_initConfig, then fills OutputConfig for HS/LS pairs with polarity
 *   HS: active_high, LS: active_low, push-pull, cmosAutomotiveSpeed1.
 * - Sets DTM dead-time rising/falling to 1e-6 s for each channel.
 * - Prepares InterruptConfig (pulse notify, CPU0, ISR_PRIORITY_ATOM) with periodEvent callback.
 * - Populates ChannelConfig[0..2] using SubModule_Ch_0..2, phase 0.0f, initial duties 25/50/75.
 *   Interrupt is assigned only on channel 0; others set to NULL_PTR.
 * - Completes main config: 3 channels, center-aligned, ATOM submodule, syncStart/syncUpdate enabled, 20 kHz.
 * - Performs eGTM enable guard: enable module, read module frequency, set GCLK/ECLK0 dividers to 1:1,
 *   enable required clocks (FXCLK|CLK0).
 * - Initializes PWM with persistent handle and channels array, then stores initial duties/dead-times to state.
 * - Configures a debug LED GPIO as push-pull output; ISR toggles this LED each PWM period.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration structures (local) */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration (HS/LS complementary pairs) */
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

    /* 4) DTM dead-time configuration: 1e-6 s rising/falling for all channels */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: pulse notify on CPU0; periodEvent callback; dutyEvent not used */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2, phase 0.0, duties 25/50/75, attach dtm/output */
    /* CH0 -> Phase U (base channel, carries interrupt) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;

    /* CH1 -> Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main configuration */
    config.cluster            = IfxEgtm_Cluster_0;                 /* Match ATOM0 pin selection */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = PWM_FREQUENCY;                     /* Hz */
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Fxclk_0;       /* FXCLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_systemClock;
    config.syncUpdateEnabled  = TRUE;
    config.syncStart          = TRUE;

    /* 8) eGTM enable guard + CMU clocks setup (inside guard) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        (void)IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM); /* Read dynamically as per guidelines */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);   /* GCLK 1:1 */
        IfxEgtm_Cmu_setEclkDivider(&MODULE_EGTM, IfxEgtm_Cmu_Eclk_0, 1u, 1u); /* ECLK0 1:1 */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver with persistent handle and channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store initial duties, phases, and dead-times in module state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure debug LED as push-pull output (ISR will toggle it) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Percent-based duty ramp update for U/V/W channels.
 *
 * Behavior summary:
 * - For each duty element: if (duty + step) >= 100, wrap to 0; then increment all by step.
 * - Apply new duties in one shot via IfxEgtm_Pwm_updateChannelsDutyImmediate using the state's array.
 * - No timing logic here; scheduling is managed by CpuX_Main.
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
