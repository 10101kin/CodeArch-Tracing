/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver for EGTM ATOM 3-Phase Inverter PWM (TC4xx)
 *
 * Notes:
 *  - Uses IfxEgtm_Pwm unified driver (ATOM submodule)
 *  - 3 complementary, center-aligned PWM pairs on ATOM0 CH0..CH2 with 1 us dead-time
 *  - 1 edge-aligned trigger generator on ATOM0 CH3 routed to ADC via IfxEgtm_Trigger_trigToAdc
 *  - No watchdog disable in this driver (WTU APIs must be placed in CpuX_Main.c only)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================
 * Configuration Macros
 * ======================== */
#define EGTM_PWM_NUM_CHANNELS           (3U)
#define EGTM_PWM_FREQUENCY_HZ           (20000.0f)
#define PHASE_U_DUTY_INIT_PERCENT       (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT       (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT       (75.0f)
#define PHASE_DUTY_STEP_PERCENT         (0.01f)
#define ADC_TRIG_DUTY_PERCENT           (50.0f)

/* Debug LED: compound form (port, pin) */
#define LED                              &MODULE_P13, 0

/* ISR priority for ATOM period event */
#define ISR_PRIORITY_ATOM               (25)

/* ========================
 * Pin Routing Macros
 * Use only validated symbols. If not available, keep as NULL_PTR placeholder.
 * ======================== */
/* Phase U: P20.8 (HS), P20.9 (LS) */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_0_TOUTxx_P20_8_OUT in integration */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)

/* Phase V: P20.10 (HS), P20.11 (LS) */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUTxx_P20_10_OUT in integration */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUTxx_P20_11_OUT in integration */

/* Phase W: P20.12 (HS), P20.13 (LS) */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUTxx_P20_12_OUT in integration */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUTxx_P20_13_OUT in integration */

/* ADC Trigger: P33.0 */
#define ADC_TRIG_PIN (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_3_TOUTxx_P33_0_OUT in integration */

/* ========================
 * Module persistent state
 * ======================== */
typedef struct
{
    IfxEgtm_Pwm            pwm;                                   /* unified driver handle */
    IfxEgtm_Pwm_Channel    channels[EGTM_PWM_NUM_CHANNELS];       /* persistent channel data */
    float32                dutyCycles[EGTM_PWM_NUM_CHANNELS];     /* duty in percent */
    float32                phases[EGTM_PWM_NUM_CHANNELS];         /* phase in seconds (if used) */
    IfxEgtm_Pwm_DeadTime   deadTimes[EGTM_PWM_NUM_CHANNELS];      /* stored dead-times */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================
 * Internal callback and ISR
 * ======================== */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* ========================
 * Public functions
 * ======================== */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      output[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg;

    /* 2) Initialize main configuration with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration (complementary pairs) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high; /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;  /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us rising and falling (use literal 1e-6f) */
    dtmConfig[0].deadTime.rising    = 1.0e-6f;
    dtmConfig[0].deadTime.falling   = 1.0e-6f;
    dtmConfig[0].fastShutOff        = NULL_PTR;

    dtmConfig[1].deadTime.rising    = 1.0e-6f;
    dtmConfig[1].deadTime.falling   = 1.0e-6f;
    dtmConfig[1].fastShutOff        = NULL_PTR;

    dtmConfig[2].deadTime.rising    = 1.0e-6f;
    dtmConfig[2].deadTime.falling   = 1.0e-6f;
    dtmConfig[2].fastShutOff        = NULL_PTR;

    /* 5) Interrupt configuration: period event on base channel only */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2 map to ATOM CH0..CH2) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg; /* base channel has interrupt */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main configuration fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)EGTM_PWM_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;           /* CMU Clock 0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;   /* DTM clock consistent with CMU0 */
    config.syncUpdateEnabled  = TRUE;                                 /* shadow to active at period end */
    config.syncStart          = TRUE;                                 /* start after init */

    /* 8) EGTM enable guard with CMU clock setup */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize unified PWM (persistent handle and channels) */
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

    /* 11) Configure trigger generator on ATOM0 CH3 (edge-aligned 50% duty) and route to ADC */
    {
        IfxEgtm_Pwm_Config        trigCfg;
        IfxEgtm_Pwm_ChannelConfig trigChCfg[1];
        IfxEgtm_Pwm_OutputConfig  trigOut[1];
        IfxEgtm_Pwm               trigPwm;
        IfxEgtm_Pwm_Channel       trigCh[1];

        IfxEgtm_Pwm_initConfig(&trigCfg, &MODULE_EGTM);

        trigOut[0].pin                   = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_PIN;
        trigOut[0].complementaryPolarity = Ifx_ActiveState_low;  /* unused */
        trigOut[0].polarity              = Ifx_ActiveState_high; /* active high */
        trigOut[0].outputMode            = IfxPort_OutputMode_pushPull;
        trigOut[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        trigOut[0].complementaryPin      = NULL_PTR;

        trigChCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;    /* ATOM0 CH3 */
        trigChCfg[0].phase     = 0.0f;
        trigChCfg[0].duty      = ADC_TRIG_DUTY_PERCENT;         /* 50% */
        trigChCfg[0].dtm       = NULL_PTR;                      /* no DTM for trigger */
        trigChCfg[0].output    = &trigOut[0];
        trigChCfg[0].mscOut    = NULL_PTR;
        trigChCfg[0].interrupt = NULL_PTR;

        trigCfg.cluster           = IfxEgtm_Cluster_0;
        trigCfg.subModule         = IfxEgtm_Pwm_SubModule_atom;
        trigCfg.alignment         = IfxEgtm_Pwm_Alignment_edge; /* edge-aligned */
        trigCfg.numChannels       = 1U;
        trigCfg.channels          = &trigChCfg[0];
        trigCfg.frequency         = EGTM_PWM_FREQUENCY_HZ;      /* same 20 kHz */
        trigCfg.clockSource.atom  = (uint32)IfxEgtm_Cmu_Clk_0;
        trigCfg.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0;
        trigCfg.syncUpdateEnabled = TRUE;
        trigCfg.syncStart         = TRUE;

        IfxEgtm_Pwm_init(&trigPwm, &trigCh[0], &trigCfg);

        /* Route ATOM0 CH3 to ADC trigger bus (AdcTriggerSignal_0) */
        (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                        IfxEgtm_TrigSource_atom0,
                                        IfxEgtm_TrigChannel_3,
                                        IfxEgtm_Cfg_AdcTriggerSignal_0);
    }

    /* 12) Configure LED GPIO as push-pull output for debug indication */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    if (requestDuty == NULL_PTR)
    {
        return; /* no update if input is invalid */
    }

    /* Clamp and copy in percent [0..100] */
    float32 du = requestDuty[0];
    float32 dv = requestDuty[1];
    float32 dw = requestDuty[2];

    if (du < 0.0f) { du = 0.0f; } else if (du > 100.0f) { du = 100.0f; }
    if (dv < 0.0f) { dv = 0.0f; } else if (dv > 100.0f) { dv = 100.0f; }
    if (dw < 0.0f) { dw = 0.0f; } else if (dw > 100.0f) { dw = 100.0f; }

    g_egtmAtom3phInv.dutyCycles[0] = du;
    g_egtmAtom3phInv.dutyCycles[1] = dv;
    g_egtmAtom3phInv.dutyCycles[2] = dw;

    /* Apply atomically via shadow registers; update takes effect at next period */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
