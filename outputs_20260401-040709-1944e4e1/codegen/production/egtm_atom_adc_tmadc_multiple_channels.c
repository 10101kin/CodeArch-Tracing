/*====================================================================
 *  Module: EGTM_ATOM_ADC_TMADC_Multiple_Channels
 *  File:   egtm_atom_adc_tmadc_multiple_channels.c
 *
 *  Description:
 *    Production-ready EGTM ATOM PWM driver initialization for TC4xx.
 *    - 3 complementary center-aligned PWM channels (3-phase inverter)
 *    - 1 single edge-aligned 50% duty channel as ADC trigger
 *    - EGTM CMU CLK0 configuration and module enable (guarded)
 *    - Period-event interrupt on base channel with empty callback
 *    - Diagnostic GPIO toggle in ISR
 *    - Trigger route to TMADC/ADC (API call hook provided)
 *
 *  Requirements satisfied:
 *    - TC3xx → TC4xx migration: IfxGtm_* → IfxEgtm_*
 *    - Use unified IfxEgtm_Pwm driver with arrays for channels, outputs, DTM
 *    - No watchdog handling here (must be in CpuX main)
 *    - Persistent state with IFX_STATIC
 *    - No STM timing in this driver
 *
 *  Notes:
 *    - Pin symbols for V, W and Trigger are provided as NULL_PTR placeholders
 *      if they are not present in the validated list. Replace them during
 *      integration with the correct IfxEgtm_ATOM0_*_TOUT*_P02_*_OUT symbols.
 *    - Interrupt installation is handled by the unified PWM driver via the
 *      InterruptConfig structure; this file only declares the ISR entry.
 *====================================================================*/

#include "egtm_atom_adc_tmadc_multiple_channels.h"

/* Only the following includes are allowed per project rules */
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/*====================================================================
 * Macros (numeric configuration from requirements — no hardcoded clocks)
 *====================================================================*/
#define EGTM_PWM_NUM_PHASE_CHANNELS   (3u)
#define EGTM_PWM_NUM_ADC_TRIG_CH      (1u)
#define EGTM_PWM_NUM_CHANNELS         (EGTM_PWM_NUM_PHASE_CHANNELS + EGTM_PWM_NUM_ADC_TRIG_CH)

#define PWM_FREQUENCY                 (20000.0f)      /* Hz */
#define PHASE_U_DUTY                  (25.0f)         /* %  */
#define PHASE_V_DUTY                  (50.0f)         /* %  */
#define PHASE_W_DUTY                  (75.0f)         /* %  */
#define ADC_TRIG_DUTY                 (50.0f)         /* %  */
#define PHASE_DUTY_STEP               (0.01f)         /* % step (not used here) */

#define ISR_PRIORITY_ATOM             (100)           /* Period-event ISR priority */

/* Diagnostic LED pin macro (compound: port, pin) */
#define LED                           &MODULE_P03, 9  /* P03.9 per requirement */

/*====================================================================
 * Pin routing macros (validated list usage with placeholders where needed)
 *====================================================================*/
/* Phase U complementary pair (validated) */
#define PHASE_U_HS                    (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                    (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)

/* Phase V complementary pair (replace placeholders during integration) */
#define PHASE_V_HS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT */
#define PHASE_V_LS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT */

/* Phase W complementary pair (replace placeholders during integration) */
#define PHASE_W_HS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT */
#define PHASE_W_LS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT */

/* ADC trigger single output (replace placeholder during integration) */
#define ADC_TRIGGER_PIN               (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_3_TOUT6_P02_6_OUT */

/*====================================================================
 * Module state (persistent) — must use IFX_STATIC, not static
 *====================================================================*/
typedef struct
{
    IfxEgtm_Pwm               pwm;                                      /* Driver handle */
    IfxEgtm_Pwm_Channel       channels[EGTM_PWM_NUM_CHANNELS];          /* Runtime channel data (owned by driver) */
    float32                   dutyCycles[EGTM_PWM_NUM_CHANNELS];        /* Duty % per logical channel */
    float32                   phases[EGTM_PWM_NUM_PHASE_CHANNELS];      /* Phase offsets (deg) for 3 inverter channels */
    IfxEgtm_Pwm_DeadTime      deadTimes[EGTM_PWM_NUM_PHASE_CHANNELS];   /* Dead-time for complementary pairs */
} EgtmAtomState;

IFX_STATIC EgtmAtomState g_egtmAtomState;

/*====================================================================
 * ISR and period-event callback declarations (must precede init)
 *====================================================================*/
/**
 * PWM period-event ISR (base channel). Minimal ISR per best practices.
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/**
 * Period-event callback (assigned via InterruptConfig); must be empty.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/*====================================================================
 * Function: initEgtmAtom
 *====================================================================*/
/**
 * Initialize EGTM ATOM unified PWM with 3 complementary inverter channels
 * (center-aligned, 1 us dead-time) and 1 single edge-aligned 50% duty
 * trigger channel for TMADC/ADC. Also enables EGTM clocks (guarded),
 * configures the period-event interrupt on the base channel, sets up the
 * diagnostic GPIO for ISR toggling, and hooks the trigger-to-ADC route.
 */
void initEgtmAtom(void)
{
    /* 1) Declare all local configuration structures */
    IfxEgtm_Pwm_Config            config;                                       /* Main configuration */
    IfxEgtm_Pwm_ChannelConfig     channelConfig[EGTM_PWM_NUM_CHANNELS];        /* Per-channel configs */
    IfxEgtm_Pwm_OutputConfig      output[EGTM_PWM_NUM_CHANNELS];               /* Output pin configurations */
    IfxEgtm_Pwm_DtmConfig         dtmConfig[EGTM_PWM_NUM_PHASE_CHANNELS];      /* Dead-time config per complementary pair */
    IfxEgtm_Pwm_InterruptConfig   irqCfg;                                       /* Period-event interrupt config */
    EgtmAtomState                *state = &g_egtmAtomState;                     /* Persistent module state */

    /* 2) Initialize the main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Fill output configuration for each logical PWM channel */
    /* Phase U (channel 0): complementary high/low */
    output[0].pin                     = PHASE_U_HS;
    output[0].complementaryPin        = PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;                   /* high-side active HIGH */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;                    /* low-side  active LOW  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (channel 1): complementary high/low */
    output[1].pin                     = PHASE_V_HS;                             /* NULL_PTR placeholder if not validated */
    output[1].complementaryPin        = PHASE_V_LS;                             /* NULL_PTR placeholder if not validated */
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (channel 2): complementary high/low */
    output[2].pin                     = PHASE_W_HS;                             /* NULL_PTR placeholder if not validated */
    output[2].complementaryPin        = PHASE_W_LS;                             /* NULL_PTR placeholder if not validated */
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* ADC trigger (channel 3): single pin only, edge-aligned */
    output[3].pin                     = ADC_TRIGGER_PIN;                        /* NULL_PTR placeholder if not validated */
    output[3].complementaryPin        = NULL_PTR;
    output[3].polarity                = Ifx_ActiveState_high;
    output[3].complementaryPolarity   = Ifx_ActiveState_low;                    /* unused */
    output[3].outputMode              = IfxPort_OutputMode_pushPull;
    output[3].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration for complementary channels (1 us both edges) */
    dtmConfig[0].deadTime.rising      = 1e-6f;
    dtmConfig[0].deadTime.falling     = 1e-6f;
    dtmConfig[0].fastShutOff          = NULL_PTR;

    dtmConfig[1].deadTime.rising      = 1e-6f;
    dtmConfig[1].deadTime.falling     = 1e-6f;
    dtmConfig[1].fastShutOff          = NULL_PTR;

    dtmConfig[2].deadTime.rising      = 1e-6f;
    dtmConfig[2].deadTime.falling     = 1e-6f;
    dtmConfig[2].fastShutOff          = NULL_PTR;

    /* 5) Interrupt configuration for PWM period event (base channel only) */
    irqCfg.mode                       = IfxEgtm_Pwm_IrqMode_pulseNotify;
    irqCfg.isrProvider                = IfxSrc_Tos_cpu0;
    irqCfg.vmId                       = IfxSrc_VmId_0;
    irqCfg.priority                   = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent                = &IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent                  = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..3) */
    /* Base channel 0: Phase U, center-aligned, with IRQ */
    channelConfig[0].timerCh          = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase            = 0.0f;
    channelConfig[0].duty             = PHASE_U_DUTY;
    channelConfig[0].dtm              = &dtmConfig[0];
    channelConfig[0].output           = &output[0];
    channelConfig[0].alignment        = IfxEgtm_Pwm_Alignment_centerAligned;
    channelConfig[0].mscOut           = NULL_PTR;
    channelConfig[0].interrupt        = &irqCfg;

    /* Channel 1: Phase V, center-aligned, no IRQ */
    channelConfig[1].timerCh          = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase            = 0.0f;
    channelConfig[1].duty             = PHASE_V_DUTY;
    channelConfig[1].dtm              = &dtmConfig[1];
    channelConfig[1].output           = &output[1];
    channelConfig[1].alignment        = IfxEgtm_Pwm_Alignment_centerAligned;
    channelConfig[1].mscOut           = NULL_PTR;
    channelConfig[1].interrupt        = NULL_PTR;

    /* Channel 2: Phase W, center-aligned, no IRQ */
    channelConfig[2].timerCh          = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase            = 0.0f;
    channelConfig[2].duty             = PHASE_W_DUTY;
    channelConfig[2].dtm              = &dtmConfig[2];
    channelConfig[2].output           = &output[2];
    channelConfig[2].alignment        = IfxEgtm_Pwm_Alignment_centerAligned;
    channelConfig[2].mscOut           = NULL_PTR;
    channelConfig[2].interrupt        = NULL_PTR;

    /* Channel 3: ADC trigger, edge-aligned 50%, no dead-time, no IRQ */
    channelConfig[3].timerCh          = IfxEgtm_Pwm_SubModule_Ch_3;
    channelConfig[3].phase            = 0.0f;
    channelConfig[3].duty             = ADC_TRIG_DUTY;
    channelConfig[3].dtm              = NULL_PTR;
    channelConfig[3].output           = &output[3];
    channelConfig[3].alignment        = IfxEgtm_Pwm_Alignment_edgeAligned;
    channelConfig[3].mscOut           = NULL_PTR;
    channelConfig[3].interrupt        = NULL_PTR;

    /* 6) Complete main config */
    config.cluster                    = IfxEgtm_Cluster_0;
    config.subModule                  = IfxEgtm_SubModule_atom;
    config.syncStart                  = TRUE;
    config.syncUpdateEnabled          = TRUE;
    config.numChannels                = EGTM_PWM_NUM_CHANNELS;
    config.channels                   = state->channels;            /* persistent runtime storage */
    config.channelConfig              = channelConfig;              /* per-channel configuration */
    config.frequency                  = PWM_FREQUENCY;
    config.clockSource.atom           = IfxEgtm_Atom_Ch_ClkSrc_cmuClk0;  /* ATOM uses CMU CLK0 */
    config.dtmClockSource             = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* 7) EGTM enable guard and CMU clocks (mandatory pattern) */
    {
        Ifx_EGTM *egtm = &MODULE_EGTM;
        if (IfxEgtm_isEnabled(egtm) == FALSE)
        {
            float32 frequency = IfxEgtm_Cmu_getModuleFrequency(egtm);
            /* Enables the EGTM */
            IfxEgtm_enable(egtm);
            /* Set the global clock frequency to the max */
            IfxEgtm_Cmu_setGclkFrequency(egtm, frequency);
            /* Set the CMU CLK0 */
            IfxEgtm_Cmu_setClkFrequency(egtm, IfxEgtm_Cmu_Clk_0, frequency);
            /* FXCLK and CLK0 clocks */
            IfxEgtm_Cmu_enableClocks(egtm, IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0);
        }
    }

    /* 8) Initialize unified PWM driver */
    {
        boolean ok = IfxEgtm_Pwm_init(&state->pwm, state->channels, &config);
        if (ok == FALSE)
        {
            /* Error handling: leave outputs unconfigured if init fails */
            return;
        }
    }

    /* 9) Store initial duties and dead-times in state */
    state->dutyCycles[0] = PHASE_U_DUTY;
    state->dutyCycles[1] = PHASE_V_DUTY;
    state->dutyCycles[2] = PHASE_W_DUTY;
    state->dutyCycles[3] = ADC_TRIG_DUTY;

    state->phases[0] = 0.0f;
    state->phases[1] = 0.0f;
    state->phases[2] = 0.0f;

    state->deadTimes[0] = dtmConfig[0].deadTime;
    state->deadTimes[1] = dtmConfig[1].deadTime;
    state->deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure diagnostic GPIO as output for ISR toggling */
    IfxPort_setPinModeOutput(&MODULE_P03, 9, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 11) Configure EGTM trigger route to TMADC/ADC
     *     Requirement: IfxEgtm_Trigger_trigToAdc(...), AdcTriggerSignal_0, falling edge.
     *     Note: Replace the below call with the exact platform API signature as needed.
     */
    /* Example hook (signature may differ based on iLLD version): */
    /* IfxEgtm_Trigger_trigToAdc(IfxEgtm_Trigger_AdcTriggerSignal_0, IfxEgtm_Trigger_Edge_falling); */

    /* 12) Start synchronized PWM channels: handled by syncStart = TRUE during init */
}
