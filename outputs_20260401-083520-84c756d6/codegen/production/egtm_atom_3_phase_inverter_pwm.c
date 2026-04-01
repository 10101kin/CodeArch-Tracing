/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver for EGTM ATOM 3-Phase Inverter PWM (TC4xx)
 *
 * Notes:
 *  - Uses IfxEgtm_Pwm unified driver to configure 3 complementary, center-aligned channels on ATOM0 CH0..CH2.
 *  - Routes a separate ATOM0 CH3 trigger to ADC via IfxEgtm_Trigger_trigToAdc.
 *  - No watchdog APIs here (WTU/SCU WDT must be handled in CpuX_Main.c only).
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"

/* =============================
 * Driver configuration macros
 * ============================= */
#define EGTM_PWM_NUM_CHANNELS           (3u)
#define EGTM_PWM_FREQUENCY_HZ           (20000.0f)
#define PHASE_U_DUTY_PERCENT            (25.0f)
#define PHASE_V_DUTY_PERCENT            (50.0f)
#define PHASE_W_DUTY_PERCENT            (75.0f)
#define PHASE_DUTY_STEP_PERCENT         (0.01f)
#define ISR_PRIORITY_ATOM               (100)

/* LED for debug indication: compound macro expands to (port, pin) parameters */
#define LED                              &MODULE_P03, 9

/* =============================
 * Pin routing (validated symbols only)
 * =============================
 * Use ONLY validated symbols. If a requested pin is not present in the validated
 * list, keep it as NULL_PTR and replace during board integration.
 */
/* Phase U: P20.8 (HS), P20.9 (LS). Only P20.9 is validated below */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_0_TOUTxx_P20_8_OUT per board pin-map */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)

/* Phase V: P20.10 (HS), P20.11 (LS) --> not in validated list here */
#define PHASE_V_HS   (NULL_PTR) /* Replace with validated symbol for P20.10 */
#define PHASE_V_LS   (NULL_PTR) /* Replace with validated symbol for P20.11 */

/* Phase W: P20.12 (HS), P20.13 (LS) --> not in validated list here */
#define PHASE_W_HS   (NULL_PTR) /* Replace with validated symbol for P20.12 */
#define PHASE_W_LS   (NULL_PTR) /* Replace with validated symbol for P20.13 */

/* ADC trigger pin P33.0 is not required for routing via Trigger API; kept for reference */
#define ADC_TRIG_PIN (NULL_PTR) /* Route uses IfxEgtm_Trigger_trigToAdc, not a TOUT pin */

/* =============================
 * Module persistent state
 * ============================= */
typedef struct
{
    IfxEgtm_Pwm            pwm;                                      /* Driver handle */
    IfxEgtm_Pwm_Channel    channels[EGTM_PWM_NUM_CHANNELS];          /* Channel data retained by driver */
    float32                dutyCycles[EGTM_PWM_NUM_CHANNELS];        /* Duty in percent per logical channel */
    float32                phases[EGTM_PWM_NUM_CHANNELS];            /* Phase (not used, kept for completeness) */
    IfxEgtm_Pwm_DeadTime   deadTimes[EGTM_PWM_NUM_CHANNELS];         /* Configured dead-times */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =============================
 * ISR and period callback
 * ============================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Public API implementations
 * ============================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output (pin) configuration for each complementary pair */
    /* Complementary polarity convention: HS active HIGH, LS active LOW */
    output[0].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;  /* HS */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;   /* LS */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (in seconds); both edges = 1 us */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;
    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;
    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: period event on base channel only */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: 3 logical channels (U,V,W) on ATOM0 CH0..CH2 */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f; /* synchronous */
    channelConfig[0].duty      = PHASE_U_DUTY_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;  /* base channel gets the ISR */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f; /* synchronous */
    channelConfig[1].duty      = PHASE_V_DUTY_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f; /* synchronous */
    channelConfig[2].duty      = PHASE_W_DUTY_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;     /* center-aligned */
    config.numChannels        = (uint8)EGTM_PWM_NUM_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;            /* 20 kHz */
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                /* CMU Clock 0 for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;/* DTM clocks from CMU Clock 0 */
    config.syncUpdateEnabled  = TRUE;                             /* shadow update at period end */
    config.syncStart          = TRUE;                             /* start after init */

    /* 8) EGTM enable guard + CMU clocks (GCLK and CLK0 set to module frequency) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver (single unified init) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store initial duties and dead-times into persistent state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    /* 11) Route ADC trigger (ATOM0 CH3) to ADC trigger bus via EGTM Trigger API */
    {
        /* Edge-aligned channel setup for CH3 is not part of this unified 3-ch PWM configuration.
           This call configures routing of ATOM0 CH3 to ADC trigger mux (AdcTriggerSignal_0). */
        boolean routed = IfxEgtm_Trigger_trigToAdc(
            IfxEgtm_Cluster_0,
            IfxEgtm_TrigSource_atom0,
            IfxEgtm_TrigChannel_3,
            IfxEgtm_Cfg_AdcTriggerSignal_0);
        (void)routed; /* optional: handle routing status if needed */
    }

    /* 12) Configure LED GPIO as push-pull output for debug indication */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    if (requestDuty == NULL_PTR)
    {
        return;
    }

    /* Clamp requested duties to [0, 100] percent and store to state */
    float32 d0 = requestDuty[0];
    float32 d1 = requestDuty[1];
    float32 d2 = requestDuty[2];

    if (d0 < 0.0f) { d0 = 0.0f; } else if (d0 > 100.0f) { d0 = 100.0f; }
    if (d1 < 0.0f) { d1 = 0.0f; } else if (d1 > 100.0f) { d1 = 100.0f; }
    if (d2 < 0.0f) { d2 = 0.0f; } else if (d2 > 100.0f) { d2 = 100.0f; }

    g_egtmAtom3phInv.dutyCycles[0] = d0;
    g_egtmAtom3phInv.dutyCycles[1] = d1;
    g_egtmAtom3phInv.dutyCycles[2] = d2;

    /* Apply atomically to all three logical channels using the immediate API */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
