/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 * Production driver: EGTM Cluster 0 ATOM0 CH0..CH2 center-aligned complementary PWM at 20 kHz,
 * synchronized TMADC trigger via ATOM0 CH3, with debug GPIO toggle in ISR.
 *
 * Follows unified IfxEgtm_Pwm initialization pattern and TC4xx migration rules.
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Configuration macros (numeric constants) ========================= */
#define EGTM_NUM_CHANNELS                (3U)
#define EGTM_PWM_FREQUENCY_HZ            (20000.0f)
#define EGTM_PHASE_U_DUTY_PCT            (25.0f)
#define EGTM_PHASE_V_DUTY_PCT            (50.0f)
#define EGTM_PHASE_W_DUTY_PCT            (75.0f)
#define EGTM_PHASE_DUTY_STEP_PCT         (0.01f)
#define ISR_PRIORITY_ATOM                (25)

/* Dead-time configuration in seconds (1 us rising/falling) */
#define EGTM_DEADTIME_RISING_S           (1.0e-6f)
#define EGTM_DEADTIME_FALLING_S          (1.0e-6f)

/* ========================= Pin selection macros ========================= */
/*
 * Use only validated EGTM ATOM pin symbols. User-requested pins:
 *  U: P20.8 (HS) / P20.9 (LS), V: P20.10/11, W: P20.12/13, ADC trig: P33.0
 * From validated list, only P20.9 is available as ATOM0_0N. Others are not listed.
 * For unavailable pads, use NULL_PTR placeholders for integration-time assignment.
 */
#define PHASE_U_HS                       (NULL_PTR) /* Replace with the correct TOUT mapping for P20.8 when available */
#define PHASE_U_LS                       (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                       (NULL_PTR) /* Replace with the correct TOUT mapping for P20.10 when available */
#define PHASE_V_LS                       (NULL_PTR) /* Replace with the correct TOUT mapping for P20.11 when available */
#define PHASE_W_HS                       (NULL_PTR) /* Replace with the correct TOUT mapping for P20.12 when available */
#define PHASE_W_LS                       (NULL_PTR) /* Replace with the correct TOUT mapping for P20.13 when available */

/* Debug GPIO (toggled in ISR): P03.9 */
#define LED                              &MODULE_P03, 9  /* compound macro: (port, pin) */

/* ========================= Module persistent state ========================= */
typedef struct
{
    IfxEgtm_Pwm               pwm;                              /* PWM driver handle */
    IfxEgtm_Pwm_Channel       channels[EGTM_NUM_CHANNELS];      /* Persistent channel handles */
    float32                   dutyCycles[EGTM_NUM_CHANNELS];    /* Duty cycles in percent */
    float32                   phases[EGTM_NUM_CHANNELS];        /* Phase offsets in percent (0 for center mode) */
    IfxEgtm_Pwm_DeadTime      deadTimes[EGTM_NUM_CHANNELS];     /* Dead-time per channel (seconds) */
} EgtmAtom3ph_State;

IFX_STATIC EgtmAtom3ph_State g_egtmAtom3ph = {0};

/* ========================= ISR and callback (declared before init) ========================= */
/*
 * PWM ISR: priority must match InterruptConfig.priority. Body must toggle LED only.
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/*
 * Empty period-event callback required by unified PWM driver.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Private helpers (none) ========================= */

/* ========================= Public API implementation ========================= */
/*
 * Initialize 3-phase center-aligned complementary PWM on EGTM Cluster 0 ATOM0 with ADC trigger,
 * then bring TMADC online and configure a debug GPIO.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structs as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Initialize main PWM config with defaults and set core fields */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output configuration for three logical complementary channels */
    /* 3) Complementary pairing and polarity */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* LS active low */
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

    /* 4) DTM dead-time configuration (1 us rising/falling) */
    dtmConfig[0].deadTime.rising  = EGTM_DEADTIME_RISING_S;
    dtmConfig[0].deadTime.falling = EGTM_DEADTIME_FALLING_S;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = EGTM_DEADTIME_RISING_S;
    dtmConfig[1].deadTime.falling = EGTM_DEADTIME_FALLING_S;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = EGTM_DEADTIME_RISING_S;
    dtmConfig[2].deadTime.falling = EGTM_DEADTIME_FALLING_S;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration for base channel period event */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Per-channel configuration (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = EGTM_PHASE_U_DUTY_PCT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;   /* base channel gets interrupt */

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = EGTM_PHASE_V_DUTY_PCT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = EGTM_PHASE_W_DUTY_PCT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Main PWM configuration fields */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.cluster            = IfxEgtm_Cluster_0;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)EGTM_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;          /* ATOM clock source CMU CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_systemClock;/* DTM clock source */
    config.syncUpdateEnabled  = TRUE;
    config.syncStart          = TRUE;

    /* 7) EGTM enable guard with CMU setup */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize PWM driver (applies duties, alignment, outputs, dead-time, and configures ISR) */
    IfxEgtm_Pwm_init(&g_egtmAtom3ph.pwm, g_egtmAtom3ph.channels, &config);

    /* 9) Store initial duty cycles, phases, and dead-times into persistent state */
    g_egtmAtom3ph.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3ph.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3ph.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3ph.phases[0] = channelConfig[0].phase;
    g_egtmAtom3ph.phases[1] = channelConfig[1].phase;
    g_egtmAtom3ph.phases[2] = channelConfig[2].phase;

    g_egtmAtom3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure EGTM->ADC trigger: ATOM0 CH3 drives TMADC trigger via trigger API */
    {
        /* Note: Actual enumerators depend on device header; placeholders used per API */
        (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                        (IfxEgtm_TrigSource)0 /* e.g., ATOM0 source */,
                                        (IfxEgtm_TrigChannel)3 /* CH3 as trigger source */,
                                        (IfxEgtm_Cfg_AdcTriggerSignal)0 /* AdcTriggerSignal_0 */);
    }

    /* 11) Initialize and start TMADC: module + channels with event enable */
    {
        IfxAdc_Tmadc        tmadc;
        IfxAdc_Tmadc_Config tmadcCfg;
        IfxAdc_Tmadc_Ch     ch[5];
        IfxAdc_Tmadc_ChConfig chCfg;

        IfxAdc_Tmadc_initModuleConfig(&tmadcCfg, &MODULE_ADC);
        IfxAdc_Tmadc_initModule(&tmadc, &tmadcCfg);

        /* Initialize 5 channels: CH0..CH4 (one-shot, event on result) */
        for (uint8 i = 0U; i < 5U; ++i)
        {
            IfxAdc_Tmadc_initChannelConfig(&chCfg, &MODULE_ADC);
            /* Additional channel field settings (e.g., channel id, sampling time, trigger) would be applied here if available */
            IfxAdc_Tmadc_initChannel(&ch[i], &chCfg);
            IfxAdc_Tmadc_enableChannelEvent(&ch[i], (IfxAdc_TmadcServReq)0 /* SRV node 0 */, (IfxAdc_TmadcEventSel)0 /* result event */);
        }

        IfxAdc_Tmadc_runModule(&tmadc);
    }

    /* 12) Configure debug GPIO as output push-pull; ISR will toggle this pin */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Synchronous immediate duty update for three logical channels.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy requested duties directly into persistent state (percent) */
    g_egtmAtom3ph.dutyCycles[0] = requestDuty[0];
    g_egtmAtom3ph.dutyCycles[1] = requestDuty[1];
    g_egtmAtom3ph.dutyCycles[2] = requestDuty[2];

    /* Apply all three duties atomically using immediate update API */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3ph.pwm, (float32 *)g_egtmAtom3ph.dutyCycles);
}

/*
 * Incrementally adjust stored duty cycle values and apply synchronously.
 */
void updateEgtmAtomDuty(void)
{
    /* Apply DUTY WRAP RULE exactly: check, maybe reset, then always add step */
    if ((g_egtmAtom3ph.dutyCycles[0] + EGTM_PHASE_DUTY_STEP_PCT) >= 100.0f) { g_egtmAtom3ph.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3ph.dutyCycles[1] + EGTM_PHASE_DUTY_STEP_PCT) >= 100.0f) { g_egtmAtom3ph.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3ph.dutyCycles[2] + EGTM_PHASE_DUTY_STEP_PCT) >= 100.0f) { g_egtmAtom3ph.dutyCycles[2] = 0.0f; }

    g_egtmAtom3ph.dutyCycles[0] += EGTM_PHASE_DUTY_STEP_PCT;
    g_egtmAtom3ph.dutyCycles[1] += EGTM_PHASE_DUTY_STEP_PCT;
    g_egtmAtom3ph.dutyCycles[2] += EGTM_PHASE_DUTY_STEP_PCT;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3ph.pwm, (float32 *)g_egtmAtom3ph.dutyCycles);
}

/*
 * ADC result interrupt service routine: minimal processing (toggle debug GPIO).
 */
void resultISR(void)
{
    IfxPort_togglePin(LED);
}
