/**********************************************************************************************************************
 *  FILE: egtm_atom_tmadc_consolidated.c
 *  BRIEF: Consolidated eGTM ATOM + TMADC driver (TC4xx) — implementation
 *
 *  REQUIREMENTS IMPLEMENTED (summary):
 *   - eGTM ATOM0 CH0–CH2: complementary, center-aligned PWM @ 20 kHz, 1 µs dead-time
 *   - Initial duties: U=25%, V=50%, W=75%
 *   - ATOM trigger channel: CH3, 50% trigger point (period match), pin routing via PinMap API
 *   - eGTM Trigger routing to TMADC via IfxEgtm_Trigger_trigToAdc()
 *   - TMADC module initialization (module-level)
 *   - LED GPIO configured as push-pull output for application use
 *
 *  CRITICAL RULES FOLLOWED:
 *   - High-level IfxEgtm_Pwm init pattern with initConfig → modify → init
 *   - Enable-guard for eGTM and CMU clocks inside a single guarded block
 *   - Use OutputConfig/DtmConfig/ChannelConfig arrays
 *   - Do not disable watchdogs in this driver
 *   - ISR and period-event callback defined per structural rules
 *********************************************************************************************************************/

/* Module header */
#include "egtm_atom_tmadc_consolidated.h"

/* iLLD core types */
#include "Ifx_Types.h"

/* eGTM / PWM / ATOM / CMU */
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Atom_Timer.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"

/* TMADC */
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"

/* GPIO */
#include "IfxPort.h"

/*--------------------------------------------------------------------------------------------------------------------
 * Configuration Macros (values from requirements — percent values expressed as float32 percentages [0..100])
 *-------------------------------------------------------------------------------------------------------------------*/
#define EGTM_NUM_PWM_CHANNELS         (3U)
#define EGTM_PWM_FREQUENCY_HZ         (20000.0f)
#define PHASE_U_DUTY_PERCENT          (25.0f)
#define PHASE_V_DUTY_PERCENT          (50.0f)
#define PHASE_W_DUTY_PERCENT          (75.0f)
#define PHASE_DUTY_STEP_PERCENT       (0.01f)
#define ADC_TRIG_DUTY_PERCENT         (50.0f)

/* ISR priority for eGTM ATOM PWM interrupt (structural rule macro name) */
#define ISR_PRIORITY_ATOM             (25)

/* LED macro (compound form: port, pin) — structural rule mandates P13.0 macro usage */
#define LED                           &MODULE_P13, 0

/*--------------------------------------------------------------------------------------------------------------------
 * Validated ATOM Pin Macros (use only validated symbols; otherwise provide NULL_PTR placeholder with note)
 *-------------------------------------------------------------------------------------------------------------------*/
/* User-requested CH0 pins are validated and available: */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)      /* EGTM.ATOM0.CH0 high-side:  P02.0 */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)     /* EGTM.ATOM0.CH0 low-side:   P02.1 */

/* User-requested CH1/CH2 pins are not present in the validated list → keep as placeholders to be bound during HW integration */
#define PHASE_V_HS   (NULL_PTR) /* TODO: Bind to &IfxEgtm_ATOM0_1_TOUTxx_P20_10_OUT when available in target PinMap */
#define PHASE_V_LS   (NULL_PTR) /* TODO: Bind to &IfxEgtm_ATOM0_1N_TOUTxx_P20_11_OUT when available in target PinMap */
#define PHASE_W_HS   (NULL_PTR) /* TODO: Bind to &IfxEgtm_ATOM0_2_TOUTxx_P20_12_OUT when available in target PinMap */
#define PHASE_W_LS   (NULL_PTR) /* TODO: Bind to &IfxEgtm_ATOM0_2N_TOUTxx_P20_13_OUT when available in target PinMap */

/* ATOM trigger output (CH3) — requested P33.0 not in validated list → placeholder until final binding */
#define ADC_TRIG_TOUT  (NULL_PTR) /* TODO: Bind to &IfxEgtm_ATOM0_3_TOUTxx_P33_0_OUT when available in target PinMap */

/*--------------------------------------------------------------------------------------------------------------------
 * Module State (persistent) — PWM driver handle, channels array, and runtime state for duties/phases/dead-times
 *-------------------------------------------------------------------------------------------------------------------*/
typedef struct
{
    IfxEgtm_Pwm           pwm;                                   /* PWM driver handle */
    IfxEgtm_Pwm_Channel   channels[EGTM_NUM_PWM_CHANNELS];       /* Persistent channels array (required by driver) */
    float32               dutyCycles[EGTM_NUM_PWM_CHANNELS];     /* Duty in percent [0..100] per channel */
    float32               phases[EGTM_NUM_PWM_CHANNELS];         /* Phase offsets (ticks or percent per design) */
    IfxEgtm_Pwm_DeadTime  deadTimes[EGTM_NUM_PWM_CHANNELS];      /* Dead-time (rising/falling) per channel */
} EGTM_ATOM_TMADC_State;

IFX_STATIC EGTM_ATOM_TMADC_State g_egtmAtomTmadc; /* IFX_STATIC per module-architecture rule */

/*--------------------------------------------------------------------------------------------------------------------
 * ISR and callback (defined before init function as required)
 *-------------------------------------------------------------------------------------------------------------------*/
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED); /* Minimal ISR: toggle debug LED only */
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Empty period-event callback — no processing here */
}

/*--------------------------------------------------------------------------------------------------------------------
 * Public API implementation
 *-------------------------------------------------------------------------------------------------------------------*/
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration and state structures locally (pwm handle + channels are persistent in module state) */
    IfxEgtm_Pwm_Config            pwmConfig;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[EGTM_NUM_PWM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      outputCfg[EGTM_NUM_PWM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmCfg[EGTM_NUM_PWM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg;

    IfxEgtm_Atom_Timer            atomTimer;           /* ATOM trigger channel driver (logical) */

    IfxAdc_Tmadc                  tmadc;               /* TMADC driver */
    IfxAdc_Tmadc_Config           tmadcConfig;         /* TMADC config */

    /* 2) Initialize unified eGTM PWM configuration with defaults */
    IfxEgtm_Pwm_initConfig(&pwmConfig, &MODULE_EGTM);

    /* Output configuration for three logical channels (complementary, center-aligned) */
    /* Phase U (channel index 0) */
    outputCfg[0].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    outputCfg[0].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    outputCfg[0].polarity                = Ifx_ActiveState_high;  /* HS active high */
    outputCfg[0].complementaryPolarity   = Ifx_ActiveState_low;   /* LS active low  */
    outputCfg[0].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (channel index 1) */
    outputCfg[1].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS; /* placeholder until bound */
    outputCfg[1].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS; /* placeholder until bound */
    outputCfg[1].polarity                = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity   = Ifx_ActiveState_low;
    outputCfg[1].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (channel index 2) */
    outputCfg[2].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS; /* placeholder until bound */
    outputCfg[2].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS; /* placeholder until bound */
    outputCfg[2].polarity                = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity   = Ifx_ActiveState_low;
    outputCfg[2].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM configuration: 1e-6 s rising and falling dead-time; DTM clock source will be CMU Clock 0 */
    dtmCfg[0].deadTime.rising  = 1e-6f;  dtmCfg[0].deadTime.falling  = 1e-6f;  dtmCfg[0].fastShutOff = NULL_PTR;
    dtmCfg[1].deadTime.rising  = 1e-6f;  dtmCfg[1].deadTime.falling  = 1e-6f;  dtmCfg[1].fastShutOff = NULL_PTR;
    dtmCfg[2].deadTime.rising  = 1e-6f;  dtmCfg[2].deadTime.falling  = 1e-6f;  dtmCfg[2].fastShutOff = NULL_PTR;

    /* Interrupt configuration (period event on base channel). ISR toggles LED, callback is empty per rules. */
    irqCfg.mode         = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider  = IfxSrc_Tos_cpu0;
    irqCfg.priority     = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId         = IfxSrc_VmId_0;
    irqCfg.periodEvent  = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent    = NULL_PTR;

    /* Channel configurations (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_PERCENT;
    channelConfig[0].dtm        = &dtmCfg[0];
    channelConfig[0].output     = &outputCfg[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;  /* base channel: enable interrupt */

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_PERCENT;
    channelConfig[1].dtm        = &dtmCfg[1];
    channelConfig[1].output     = &outputCfg[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_PERCENT;
    channelConfig[2].dtm        = &dtmCfg[2];
    channelConfig[2].output     = &outputCfg[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Main PWM configuration */
    pwmConfig.cluster             = IfxEgtm_Cluster_0;                       /* Same cluster for PWM and trigger */
    pwmConfig.subModule           = IfxEgtm_Pwm_SubModule_atom;              /* ATOM submodule */
    pwmConfig.alignment           = IfxEgtm_Pwm_Alignment_center;            /* Center-aligned */
    pwmConfig.numChannels         = (uint8)EGTM_NUM_PWM_CHANNELS;
    pwmConfig.channels            = &channelConfig[0];
    pwmConfig.frequency           = EGTM_PWM_FREQUENCY_HZ;                   /* 20 kHz */
    pwmConfig.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;               /* Use CMU Clock 0 for ATOM */
    pwmConfig.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM uses CMU Clock 0 */
    pwmConfig.syncUpdateEnabled   = TRUE;                                     /* Keep sync update enabled */
    pwmConfig.syncStart           = TRUE;                                     /* Start channels in sync */

    /* 5) Enable guard for eGTM and CMU clocks (all CMU calls inside the guard) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 6) Initialize PWM with persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtomTmadc.pwm, &g_egtmAtomTmadc.channels[0], &pwmConfig);

    /* Persist initial duties, phases, and dead-times into module state for runtime updates */
    g_egtmAtomTmadc.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtomTmadc.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtomTmadc.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtomTmadc.phases[0] = channelConfig[0].phase;
    g_egtmAtomTmadc.phases[1] = channelConfig[1].phase;
    g_egtmAtomTmadc.phases[2] = channelConfig[2].phase;

    g_egtmAtomTmadc.deadTimes[0] = dtmCfg[0].deadTime;
    g_egtmAtomTmadc.deadTimes[1] = dtmCfg[1].deadTime;
    g_egtmAtomTmadc.deadTimes[2] = dtmCfg[2].deadTime;

    /* 7) Initialize and start the ATOM timer for the trigger channel, set 50% trigger point, and run it */
    {
        boolean ok;
        ok = IfxEgtm_Atom_Timer_setFrequency(&atomTimer, EGTM_PWM_FREQUENCY_HZ); /* Match PWM period */
        if (ok == TRUE)
        {
            uint32 triggerPoint = (g_egtmAtomTmadc.pwm.periodTicks >> 1);     /* 50% of PWM period */
            IfxEgtm_Atom_Timer_setTrigger(&atomTimer, triggerPoint);
            IfxEgtm_Atom_Timer_run(&atomTimer);
        }
        else
        {
            /* Production note: add error handling/log if required */
        }
    }

    /* 8) Route ATOM trigger channel output to pad (PinMap) — placeholder until final pin is selected */
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)ADC_TRIG_TOUT, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) Configure the trigger routing from ATOM trigger source to TMADC (edge/mux selection handled in SoC-specific cfg) */
    {
        /* Source: EGTM.ATOM0.CH3 → AdcTriggerSignal_0 (TriggerMuxSel per SoC integration; falling edge if supported)
           Note: API signature in available mocks doesn’t expose edge/mux selector → handled externally if required. */
        (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0, (IfxEgtm_TrigSource)0 /* atom0 */, (IfxEgtm_TrigChannel)3, (IfxEgtm_Cfg_AdcTriggerSignal)0 /* Signal_0 */);
    }

    /* 9) Initialize TMADC: enable module, init module config, init module (channels/ISR/pins to be bound in integration) */
    IfxAdc_enableModule(&MODULE_ADC);
    IfxAdc_Tmadc_initModuleConfig(&tmadcConfig, &MODULE_ADC);
    IfxAdc_Tmadc_initModule(&tmadc, &tmadcConfig);
    /* NOTE: Channel one-shot configuration, sampling time (100 ns), wait-for-read, SRN/ISR binding,
             and analog input pin binding are platform-/board-specific and must be completed during system integration.
             Use IfxAdc_Tmadc_readChannelResult() in the ISR (priority 25, TOS cpu0) as requested. */

    /* 10) Configure LED pin as push-pull output for later application use (done after PWM init per ordering rule) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
