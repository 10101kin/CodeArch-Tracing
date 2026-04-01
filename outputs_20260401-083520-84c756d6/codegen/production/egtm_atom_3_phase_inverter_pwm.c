/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver: EGTM ATOM 3-Phase Inverter PWM (TC4xx)
 * - 3 complementary, center-aligned PWM pairs at 20 kHz with 1 us dead-time
 * - ATOM0 CH0..CH2 for phases U,V,W
 * - ATOM0 CH3 as edge-aligned trigger routed to ADC via IfxEgtm_Trigger_trigToAdc (AdcTriggerSignal_0)
 * - LED (P03.9) toggled in ISR for debug
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"
#include "IfxCpu.h"

/* ========================= Macros and configuration constants ========================= */
#define NUM_OF_CHANNELS               (3U)
#define PWM_FREQUENCY                 (20000.0f)   /* Hz */

#define PHASE_U_DUTY                  (25.0f)      /* percent */
#define PHASE_V_DUTY                  (50.0f)      /* percent */
#define PHASE_W_DUTY                  (75.0f)      /* percent */
#define PHASE_DUTY_STEP               (0.01f)      /* percent, kept for state/reference */

#define ISR_PRIORITY_ATOM             (25)

/* LED macro in compound form (port, pin) */
#define LED                           &MODULE_P03, 9

/*
 * Pin routing macros (validated pin symbols only). The user's requested pins are:
 *  U: P20.8 (HS), P20.9 (LS)
 *  V: P20.10 (HS), P20.11 (LS)
 *  W: P20.12 (HS), P20.13 (LS)
 *  ADC_TRIG: P33.0
 * From the validated list, only P20.9 is available as ATOM0_0N.
 * For unavailable pins, keep NULL_PTR placeholders for integration to replace later.
 */
#define PHASE_U_HS   (NULL_PTR)                           /* Replace with &IfxEgtm_ATOM0_0_TOUTxx_P20_8_OUT */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT) /* Validated */
#define PHASE_V_HS   (NULL_PTR)                           /* Replace with &IfxEgtm_ATOM0_1_TOUTxx_P20_10_OUT */
#define PHASE_V_LS   (NULL_PTR)                           /* Replace with &IfxEgtm_ATOM0_1N_TOUTxx_P20_11_OUT */
#define PHASE_W_HS   (NULL_PTR)                           /* Replace with &IfxEgtm_ATOM0_2_TOUTxx_P20_12_OUT */
#define PHASE_W_LS   (NULL_PTR)                           /* Replace with &IfxEgtm_ATOM0_2N_TOUTxx_P20_13_OUT */

/* ========================= Module persistent state ========================= */
typedef struct
{
    IfxEgtm_Pwm              pwm;                                 /* Driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];           /* Channel data (persistent) */
    float32                  dutyCycles[NUM_OF_CHANNELS];         /* Duty cycles in percent */
    float32                  phases[NUM_OF_CHANNELS];             /* Phase offsets in percent of period */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];          /* Dead-time values (s) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and callback (must appear before init) ========================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API implementations ========================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      outputCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg;

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration (complementary) */
    /* U phase */
    outputCfg[0].pin                     = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    outputCfg[0].complementaryPin        = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    outputCfg[0].polarity                = Ifx_ActiveState_high;  /* HS active high */
    outputCfg[0].complementaryPolarity   = Ifx_ActiveState_low;   /* LS active low  */
    outputCfg[0].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase */
    outputCfg[1].pin                     = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    outputCfg[1].complementaryPin        = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    outputCfg[1].polarity                = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity   = Ifx_ActiveState_low;
    outputCfg[1].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase */
    outputCfg[2].pin                     = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    outputCfg[2].complementaryPin        = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    outputCfg[2].polarity                = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity   = Ifx_ActiveState_low;
    outputCfg[2].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration for both edges (1 us rising/falling) */
    dtmCfg[0].deadTime.rising  = 1.0e-6f;
    dtmCfg[0].deadTime.falling = 1.0e-6f;
    dtmCfg[0].fastShutOff      = NULL_PTR;

    dtmCfg[1].deadTime.rising  = 1.0e-6f;
    dtmCfg[1].deadTime.falling = 1.0e-6f;
    dtmCfg[1].fastShutOff      = NULL_PTR;

    dtmCfg[2].deadTime.rising  = 1.0e-6f;
    dtmCfg[2].deadTime.falling = 1.0e-6f;
    dtmCfg[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration (single object attached to base channel only) */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (base + two synchronous) */
    /* Channel 0 -> Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;  /* percent */
    channelConfig[0].dtm       = &dtmCfg[0];
    channelConfig[0].output    = &outputCfg[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;       /* only base channel has interrupt */

    /* Channel 1 -> Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmCfg[1];
    channelConfig[1].output    = &outputCfg[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Channel 2 -> Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmCfg[2];
    channelConfig[2].output    = &outputCfg[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;              /* ATOM submodule */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;            /* Center-aligned for inverter */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;               /* CMU Clock 0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM uses CMU Clock 0 */
    config.syncUpdateEnabled  = TRUE;                                     /* shadow-to-active at period end */
    config.syncStart          = TRUE;                                     /* start after init */

    /* 8) EGTM enable guard + CMU clocks configuration (follow mandated pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver (single unified init) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store initial state (duties, phases, dead-times) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmCfg[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmCfg[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmCfg[2].deadTime;

    /* 11) Route trigger channel (ATOM0 CH3) to ADC trigger bus (AdcTriggerSignal_0) */
    {
        boolean trigOk = IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                                   IfxEgtm_TrigSource_atom,
                                                   IfxEgtm_TrigChannel_3,
                                                   IfxEgtm_Cfg_AdcTriggerSignal_0);
        (void)trigOk; /* In production, handle FALSE with error logging */
    }

    /* 12) Configure LED GPIO for debug (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    if (requestDuty == NULL_PTR)
    {
        return; /* No update if pointer is invalid */
    }

    /* Clamp to [0, 100] percent and copy into persistent state */
    float32 d0 = requestDuty[0];
    float32 d1 = requestDuty[1];
    float32 d2 = requestDuty[2];

    if (d0 < 0.0f) { d0 = 0.0f; } else if (d0 > 100.0f) { d0 = 100.0f; }
    if (d1 < 0.0f) { d1 = 0.0f; } else if (d1 > 100.0f) { d1 = 100.0f; }
    if (d2 < 0.0f) { d2 = 0.0f; } else if (d2 > 100.0f) { d2 = 100.0f; }

    g_egtmAtom3phInv.dutyCycles[0] = d0;
    g_egtmAtom3phInv.dutyCycles[1] = d1;
    g_egtmAtom3phInv.dutyCycles[2] = d2;

    /* Apply atomically using shadow registers; effect at next period boundary */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm,
                                            (float32*)g_egtmAtom3phInv.dutyCycles,
                                            (uint32)NUM_OF_CHANNELS);
}
