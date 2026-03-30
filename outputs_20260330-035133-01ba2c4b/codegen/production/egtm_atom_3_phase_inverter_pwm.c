/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver: eGTM ATOM 3-Phase complementary PWM using IfxEgtm_Pwm
 * Target: TC4D7 (KIT_A3G_TC4D7_LITE)
 * Notes:
 *  - Migrated from GTM TOM (TC3xx) to eGTM ATOM (TC4xx)
 *  - Unified driver IfxEgtm_Pwm is used as per iLLD patterns
 *  - Clocks enabled via CMU inside enable-guard
 *  - ISR toggles LED; period-event callback is empty
 *  - No watchdog handling here (must be in CpuX_Main.c)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Configuration Macros ========================= */
#define NUM_OF_CHANNELS        (3u)
#define PWM_FREQUENCY          (20000.0f)         /* 20 kHz */
#define ISR_PRIORITY_ATOM      (20)

/* Initial phase duties (percent) */
#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)

/* Duty update step (percent) */
#define PHASE_DUTY_STEP        (10.0f)

/* Dead-time (seconds) */
#define DEAD_TIME_RISING_S     (1.0e-6f)          /* 1 us */
#define DEAD_TIME_FALLING_S    (1.0e-6f)          /* 1 us */

/* LED on P03.9 (compound macro expands to two parameters: port, pin) */
#define LED                    &MODULE_P03, 9

/* ========================= Pin Macros (TOUT mappings) ========================= */
/*
 * Use ONLY validated pin symbols. Where the requested pin symbol is not listed
 * in the validated set, provide NULL_PTR as placeholder to allow integration to
 * supply the correct symbol later.
 */

/* Phase U: HS=P20.8 (TOUT64), LS=P20.9 (TOUT65) */
#define PHASE_U_HS   (NULL_PTR)                           /* Replace with &IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT when available */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT) /* Validated */

/* Phase V: HS=P21.4 (TOUT55), LS=P20.11 (TOUT67) */
#define PHASE_V_HS   (NULL_PTR)                           /* Replace with &IfxEgtm_ATOM0_1_TOUT55_P21_4_OUT when available */
#define PHASE_V_LS   (NULL_PTR)                           /* Replace with &IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT when available */

/* Phase W: HS=P20.12 (TOUT68), LS=P20.13 (TOUT69) */
#define PHASE_W_HS   (NULL_PTR)                           /* Replace with &IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT when available */
#define PHASE_W_LS   (NULL_PTR)                           /* Replace with &IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT when available */

/* ========================= Module State ========================= */

typedef struct
{
    IfxEgtm_Pwm              pwm;                                  /* Unified PWM driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];            /* Persistent channel storage (owned by driver) */
    float32                  dutyCycles[NUM_OF_CHANNELS];          /* Duty in percent */
    float32                  phases[NUM_OF_CHANNELS];              /* Phase in cycles (0..1), config-time only */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];           /* Stored DT settings */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback ========================= */
/* ISR declaration and implementation: toggle LED only */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback (assigned in InterruptConfig); body intentionally empty */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */
/**
 * Initialize a 3-phase complementary PWM on eGTM ATOM using IfxEgtm_Pwm.
 * Follows the iLLD init pattern: initConfig -> customize -> enable-guard clocks -> init.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure complementary outputs for three logical channels */
    /* Phase U (Channel 0) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* HS active-high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* LS active-low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (Channel 1) */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (Channel 2) */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Configure DTM dead-time for each channel (1 us rising/falling) */
    dtmConfig[0].deadTime.rising  = DEAD_TIME_RISING_S;
    dtmConfig[0].deadTime.falling = DEAD_TIME_FALLING_S;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = DEAD_TIME_RISING_S;
    dtmConfig[1].deadTime.falling = DEAD_TIME_FALLING_S;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = DEAD_TIME_RISING_S;
    dtmConfig[2].deadTime.falling = DEAD_TIME_FALLING_S;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Configure period-event interrupt (pulse-notify, CPU0, priority=ISR_PRIORITY_ATOM) */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction; /* non-static callback with (void*) */
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations (logical channel indices 0,1,2) */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;        /* percent */
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;            /* not used */
    channelConfig[0].interrupt = &irqCfg;             /* base channel gets interrupt */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;        /* percent */
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;            /* only CH0 has interrupt */

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;        /* percent */
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Complete main config fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;          /* ATOM submodule */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;        /* center-aligned */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = PWM_FREQUENCY;                        /* Hz */
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                    /* ATOM uses CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;    /* DTM clock 0 */
    config.syncUpdateEnabled  = TRUE;
    config.syncStart          = TRUE;

    /* 8) eGTM enable + CMU clock configuration inside guard */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Dynamic frequency: set GCLK pass-through and ATOM CLK0 */
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            /* Enable FXCLK (for TOM paths) and CLK0 (for ATOM) as per iLLD pattern */
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize unified PWM driver with persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist initial duty, phase and dead-time settings into module state */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (do not drive level here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update duty cycles of three logical PWM channels by +10% with wrap at 100%.
 * Applies all three duties atomically using the driver's Immediate API.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-then-add logic for each channel (no loop, per design rule) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply updated duties atomically (synchronized behavior) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
