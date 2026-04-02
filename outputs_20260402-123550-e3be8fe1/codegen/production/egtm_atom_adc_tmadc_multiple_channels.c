/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 *
 * Production driver: EGTM ATOM 3-phase complementary PWM + TMADC trigger
 * Target: TC4xx (KIT_A3G_TC4D7_LITE)
 *
 * Notes:
 * - Uses unified IfxEgtm_Pwm driver (ATOM submodule) with center-aligned complementary outputs
 * - Synchronous ADC trigger via IfxEgtm_Trigger_trigToAdc from ATOM0 CH3
 * - TMADC module initialized and started; event enabled per-channel
 * - Debug GPIO (P03.9) is toggled in a minimal ISR function body
 * - No watchdog handling here (must be in CpuX_Main.c only)
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"

/* iLLD dependencies */
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Configuration Constants ========================= */

/* Channel count */
#define NUM_OF_CHANNELS            (3u)

/* PWM frequency (Hz) */
#define PWM_FREQUENCY              (20000.0f)

/* ISR priority for EGTM PWM period event (used by InterruptConfig and optional ISR macro) */
#define ISR_PRIORITY_ATOM          (25)

/* Initial duty cycles in percent */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)

/* Duty step (percent) for incremental update API */
#define PHASE_DUTY_STEP            (0.01f)

/* LED/debug GPIO compound macro (port, pin). User-requested P03.9 */
#define LED                        &MODULE_P03, 9

/* ========================= Pin Routing (validated pin symbols only) ========================= */
/*
 * Complementary pair mapping per logical channel:
 *  - output[i].pin               -> high-side  (active high)
 *  - output[i].complementaryPin -> low-side   (active low)
 *
 * Use only validated symbols. Where the exact TOUT symbol for the requested pad
 * is not listed in the validated set, use NULL_PTR and add integration comment.
 */

/* Phase U: requested HS=P20.8, LS=P20.9; validated list provides only P20.9 */
#define PHASE_U_HS   (NULL_PTR)                           /* TODO: attach correct TOUT for P20.8 in integration */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT) /* validated */

/* Phase V: requested HS=P20.10, LS=P20.11; not in validated list */
#define PHASE_V_HS   (NULL_PTR) /* TODO: attach correct TOUT for P20.10 in integration */
#define PHASE_V_LS   (NULL_PTR) /* TODO: attach correct TOUT for P20.11 in integration */

/* Phase W: requested HS=P20.12, LS=P20.13; not in validated list */
#define PHASE_W_HS   (NULL_PTR) /* TODO: attach correct TOUT for P20.12 in integration */
#define PHASE_W_LS   (NULL_PTR) /* TODO: attach correct TOUT for P20.13 in integration */

/* ========================= Module State ========================= */

typedef struct
{
    IfxEgtm_Pwm            pwm;                                   /* PWM driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];             /* Persistent channel handles */
    float32                dutyCycles[NUM_OF_CHANNELS];           /* Duty state in percent */
    float32                phases[NUM_OF_CHANNELS];               /* Phase offsets in percent (or 0 for center) */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];            /* Dead time configuration per channel */
} EgtmAtomApp_State;

/* IFX_STATIC per architectural guideline */
IFX_STATIC EgtmAtomApp_State g_egtmAtomState;

/* ========================= Internal ISR and Callback Declarations ========================= */

/* Optional PWM ISR (not explicitly used by this driver, kept minimal as guideline exemplar) */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback required by InterruptConfig (must exist; empty body) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API Implementations ========================= */

/*
 * Initialize a 3-phase center-aligned complementary PWM on EGTM Cluster 0 ATOM0
 * with three logical channels and a synchronized ADC trigger, then bring TMADC online
 * and configure a debug GPIO.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structs as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* TMADC locals */
    IfxAdc_Tmadc_Config          tmadcConfig;
    IfxAdc_Tmadc                 tmadc;
    IfxAdc_Tmadc_ChConfig        tmadcChCfg[5];
    IfxAdc_Tmadc_Ch              tmadcCh[5];

    /* 2) Initialize main PWM config with defaults using module handle */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure output[] for three logical channels with complementary pairing */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;      /* high-side active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;       /* low-side active LOW  */
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

    /* 4) Configure dtmConfig[] with 1us rising/falling dead-time and attach to channelConfig */
    dtmConfig[0].deadTime.rising = 1e-6f; dtmConfig[0].deadTime.falling = 1e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1e-6f; dtmConfig[1].deadTime.falling = 1e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1e-6f; dtmConfig[2].deadTime.falling = 1e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Configure interruptConfig for base-channel period event */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Fill channelConfig for three logical channels (Ch0..Ch2), center-aligned, with initial duties */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;        /* base channel gets interrupt */

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

    /* 7) Main config fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;              /* ATOM clock source */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_systemClock;    /* DTM clock source */
#if 1 /* sync features always enabled for this application */
    config.syncUpdateEnabled  = TRUE;
    config.syncStart          = TRUE;
#endif

    /* 7) Enable guard: enable EGTM and configure CMU if not already enabled */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 8) Unified PWM init (applies duties, alignment, outputs, dead-time, and wires interrupt) */
    IfxEgtm_Pwm_init(&g_egtmAtomState.pwm, &g_egtmAtomState.channels[0], &config);

    /* 9) Store initial state into persistent module storage */
    g_egtmAtomState.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtomState.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtomState.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtomState.phases[0] = channelConfig[0].phase;
    g_egtmAtomState.phases[1] = channelConfig[1].phase;
    g_egtmAtomState.phases[2] = channelConfig[2].phase;

    g_egtmAtomState.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtomState.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtomState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure EGTM->ADC trigger (ATOM0 CH3 -> TMADC trigger signal) */
    {
        /* Selected: Cluster0, ATOM0 source, Channel 3, AdcTriggerSignal_0 */
        /* Mode/edge selection is handled per SoC routing; API signature is fixed */
        boolean trigOk = IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                                   (IfxEgtm_TrigSource)0 /* IfxEgtm_TrigSource_atom0 */,
                                                   (IfxEgtm_TrigChannel)3 /* CH3 */,
                                                   (IfxEgtm_Cfg_AdcTriggerSignal)0 /* IfxEgtm_Cfg_AdcTriggerSignal_0 */);
        (void)trigOk; /* optional: handle routing failure in integration */
    }

    /* 11) Initialize and start TMADC (module + 5 channels, enable result event, run) */
    IfxAdc_Tmadc_initModuleConfig(&tmadcConfig, &MODULE_ADC);
    IfxAdc_Tmadc_initModule(&tmadc, &tmadcConfig);

    {
        uint8 i;
        for (i = 0u; i < 5u; ++i)
        {
            IfxAdc_Tmadc_initChannelConfig(&tmadcChCfg[i], &MODULE_ADC);
            IfxAdc_Tmadc_initChannel(&tmadcCh[i], &tmadcChCfg[i]);
            /* Enable channel event for result notification (servReq/eventSel placeholders) */
            IfxAdc_Tmadc_enableChannelEvent(&tmadcCh[i], (IfxAdc_TmadcServReq)0, (IfxAdc_TmadcEventSel)0);
        }
    }
    IfxAdc_Tmadc_runModule(&tmadc);

    /* 12) Configure debug GPIO as output push-pull; ISR will toggle this pin */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Synchronous immediate duty update for the three logical PWM channels.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy requested duties (percent) directly into module state */
    g_egtmAtomState.dutyCycles[0] = requestDuty[0];
    g_egtmAtomState.dutyCycles[1] = requestDuty[1];
    g_egtmAtomState.dutyCycles[2] = requestDuty[2];

    /* Apply all three duties atomically */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtomState.pwm, (float32 *)g_egtmAtomState.dutyCycles);
}

/*
 * Incrementally adjust the three stored duty cycle values and apply them synchronously.
 */
void updateEgtmAtomDuty(void)
{
    /* Wrap-then-add per-channel rule (no loops) */
    if ((g_egtmAtomState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtomState.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtomState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtomState.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtomState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtomState.dutyCycles[2] = 0.0f; }

    g_egtmAtomState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtomState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtomState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply atomically */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtomState.pwm, (float32 *)g_egtmAtomState.dutyCycles);
}

/*
 * ADC result interrupt service routine: toggle debug GPIO only.
 * No data processing here; defer longer work to main context.
 */
void resultISR(void)
{
    IfxPort_togglePin(LED);
}
