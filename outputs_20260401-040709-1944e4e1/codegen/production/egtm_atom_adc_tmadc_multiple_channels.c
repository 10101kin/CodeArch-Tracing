/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 *
 * Production driver for EGTM ATOM PWM on TC4xx: 3 complementary channels for inverter (U,V,W)
 * and one single-output 50% edge-aligned trigger channel intended for TMADC triggering.
 *
 * Notes:
 * - Uses unified IfxEgtm_Pwm high-level driver.
 * - Configures ATOM0 CH0..CH2 as complementary, center-aligned, with 1 us dead-time.
 * - Configures ATOM0 CH3 as single 50% duty trigger output.
 * - Enables EGTM clocks (GCLK, CLK0, FXCLK) per iLLD pattern before initializing PWM.
 * - Interrupt is configured only on the base channel (logical channel 0) with pulse notify mode.
 * - ISR toggles a diagnostic GPIO (LED) and the period callback is an empty hook.
 * - Watchdogs are NOT touched here (must be handled in CpuX_Main.c according to AURIX standard).
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (requirements -> constants) ========================= */
#define PWM_NUM_CHANNELS            (4)           /* 3 phases + 1 trigger */
#define PWM_FREQUENCY_HZ            (20000.0f)    /* 20 kHz */
#define PHASE_U_DUTY_INIT           (25.0f)       /* percent */
#define PHASE_V_DUTY_INIT           (50.0f)       /* percent */
#define PHASE_W_DUTY_INIT           (75.0f)       /* percent */
#define ADC_TRIG_DUTY_INIT          (50.0f)       /* percent */
#define ISR_PRIORITY_ATOM           (25)          /* PWM period-event ISR priority (provider CPU0) */

/* Diagnostic LED pin (toggle inside ISR) - compound form: (&MODULE_Pxx, pin) */
#define LED                         &MODULE_P03, 9  /* P03.9 as per requirement */

/* ========================= Validated EGTM ATOM pin symbols ========================= */
/*
 * Use ONLY validated pin symbols. For pins not present in the validated list, use NULL_PTR
 * placeholders to be replaced during platform integration with the correct IfxEgtm pin symbols.
 */
#define PHASE_U_HS                  (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)   /* P02.0 */
#define PHASE_U_LS                  (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)  /* P02.1 */

/* The following user-requested pins were not present in the provided validated list.
 * Leave them as NULL_PTR and replace with the appropriate validated symbols at integration time. */
#define PHASE_V_HS                  (NULL_PTR) /* Replace with validated symbol for ATOM0 CH1 high-side (e.g., &IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT) */
#define PHASE_V_LS                  (NULL_PTR) /* Replace with validated symbol for ATOM0 CH1 low-side  (e.g., &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT) */
#define PHASE_W_HS                  (NULL_PTR) /* Replace with validated symbol for ATOM0 CH2 high-side (e.g., &IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT) */
#define PHASE_W_LS                  (NULL_PTR) /* Replace with validated symbol for ATOM0 CH2 low-side  (e.g., &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT) */
#define ADC_TRIGGER_PIN             (NULL_PTR) /* Replace with validated symbol for ATOM0 CH3 out      (e.g., &IfxEgtm_ATOM0_3_TOUT6_P02_6_OUT) */

/* ========================= Module state ========================= */
typedef struct
{
    IfxEgtm_Pwm                pwm;                                  /* Driver handle */
    IfxEgtm_Pwm_Channel        channels[PWM_NUM_CHANNELS];           /* Channel state (persistent) */
    float32                    dutyCycles[PWM_NUM_CHANNELS];         /* Duty in percent (0..100) */
    float32                    phases[3];                             /* Phase shifts for U,V,W (seconds) */
    IfxEgtm_Pwm_DeadTime       deadTimes[3];                          /* Dead-times for complementary U,V,W */
} EgtmAtom_State;

IFX_STATIC EgtmAtom_State g_egtmAtomState;

/* ========================= ISR and callback ========================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= API implementation ========================= */
/**
 * Initialize EGTM ATOM unified PWM instance with 3 complementary channels (U,V,W) and 1 trigger channel.
 * - Channels[0..2]: complementary, center-aligned, 1us dead-time, duties: U=25%, V=50%, W=75%
 * - Channel[3]: single output, edge-aligned, 50% duty (ADC trigger source)
 * - Period-event interrupt only on base channel (channel 0)
 * - CMU clocks (GCLK, CLK0, FXCLK) enabled per iLLD pattern
 * - Diagnostic GPIO configured for ISR toggling
 */
void initEgtmAtom(void)
{
    /* 1) Declare local configuration containers */
    IfxEgtm_Pwm_Config           config;                              /* Main PWM config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[PWM_NUM_CHANNELS];     /* Per-channel config */
    IfxEgtm_Pwm_DtmConfig        dtmConfig[3];                        /* Dead-time for U,V,W */
    IfxEgtm_Pwm_InterruptConfig  irqCfg;                              /* Period-event interrupt (base ch) */
    IfxEgtm_Pwm_OutputConfig     output[PWM_NUM_CHANNELS];            /* Pin routing */

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration (U,V,W complementary + trigger single) */
    /* Phase U */
    output[0].pin                     = PHASE_U_HS;                   /* High-side */
    output[0].complementaryPin        = PHASE_U_LS;                   /* Low-side */
    output[0].polarity                = Ifx_ActiveState_high;         /* HS active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;          /* LS active low */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = PHASE_V_HS;                   /* High-side */
    output[1].complementaryPin        = PHASE_V_LS;                   /* Low-side */
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = PHASE_W_HS;                   /* High-side */
    output[2].complementaryPin        = PHASE_W_LS;                   /* Low-side */
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Trigger (single output) */
    output[3].pin                     = ADC_TRIGGER_PIN;              /* Single-pin output */
    output[3].complementaryPin        = NULL_PTR;                     /* No complementary */
    output[3].polarity                = Ifx_ActiveState_high;
    output[3].complementaryPolarity   = Ifx_ActiveState_low;          /* Irrelevant when no complementary pin */
    output[3].outputMode              = IfxPort_OutputMode_pushPull;
    output[3].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration for complementary phases (1 us rising/falling) */
    dtmConfig[0].deadTime.rising      = 1e-6f;                        /* 1 microsecond */
    dtmConfig[0].deadTime.falling     = 1e-6f;                        /* 1 microsecond */
    dtmConfig[0].fastShutOff          = NULL_PTR;

    dtmConfig[1].deadTime.rising      = 1e-6f;
    dtmConfig[1].deadTime.falling     = 1e-6f;
    dtmConfig[1].fastShutOff          = NULL_PTR;

    dtmConfig[2].deadTime.rising      = 1e-6f;
    dtmConfig[2].deadTime.falling     = 1e-6f;
    dtmConfig[2].fastShutOff          = NULL_PTR;

    /* 5) Interrupt configuration: period-event on base channel only */
    irqCfg.mode                       = IfxEgtm_Pwm_IrqMode_pulseNotify;
    irqCfg.isrProvider                = IfxSrc_Tos_cpu0;
    irqCfg.priority                   = ISR_PRIORITY_ATOM;
    irqCfg.vmId                       = IfxSrc_VmId_0;
    irqCfg.periodEvent                = IfxEgtm_periodEventFunction;   /* Non-static callback */
    irqCfg.dutyEvent                  = NULL_PTR;                      /* Not used */

    /* 6) Channel configuration (logical indices 0..3) */
    /* CH0: Phase U (base channel with IRQ) */
    channelConfig[0].timerCh          = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase            = 0.0f;
    channelConfig[0].duty             = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm              = &dtmConfig[0];
    channelConfig[0].output           = &output[0];
    channelConfig[0].mscOut           = NULL_PTR;                      /* Not using MSC */
    channelConfig[0].interrupt        = &irqCfg;                       /* IRQ on base channel */

    /* CH1: Phase V */
    channelConfig[1].timerCh          = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase            = 0.0f;
    channelConfig[1].duty             = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm              = &dtmConfig[1];
    channelConfig[1].output           = &output[1];
    channelConfig[1].mscOut           = NULL_PTR;
    channelConfig[1].interrupt        = NULL_PTR;                      /* No IRQ */

    /* CH2: Phase W */
    channelConfig[2].timerCh          = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase            = 0.0f;
    channelConfig[2].duty             = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm              = &dtmConfig[2];
    channelConfig[2].output           = &output[2];
    channelConfig[2].mscOut           = NULL_PTR;
    channelConfig[2].interrupt        = NULL_PTR;                      /* No IRQ */

    /* CH3: ADC trigger (single output, no dead-time) */
    channelConfig[3].timerCh          = IfxEgtm_Pwm_SubModule_Ch_3;
    channelConfig[3].phase            = 0.0f;
    channelConfig[3].duty             = ADC_TRIG_DUTY_INIT;
    channelConfig[3].dtm              = NULL_PTR;                      /* No dead-time */
    channelConfig[3].output           = &output[3];
    channelConfig[3].mscOut           = NULL_PTR;
    channelConfig[3].interrupt        = NULL_PTR;                      /* No IRQ */

    /* 6) Complete main PWM configuration */
    config.cluster                    = IfxEgtm_Cluster_0;
    config.subModule                  = IfxEgtm_SubModule_atom;        /* ATOM */
    config.syncStart                  = TRUE;                          /* Start all together */
    config.syncUpdateEnabled          = TRUE;                          /* Sync updates */
    config.numChannels                = PWM_NUM_CHANNELS;
    config.channels                   = channelConfig;                 /* Attach channel configs */
    config.frequency                  = PWM_FREQUENCY_HZ;              /* 20 kHz */
    config.alignment                  = IfxEgtm_Pwm_Alignment_centerAligned; /* Center-aligned by default */
    config.clockSource.atom           = IfxEgtm_Atom_Ch_ClkSrc_cmuClk0;      /* ATOM uses CMU CLK0 */
    config.dtmClockSource             = IfxEgtm_Dtm_ClockSource_cmuClock0;   /* DTM clock */

    /* 7) EGTM clock enable block (MANDATORY pattern) */
    {
        float32 frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Enables the EGTM */
        IfxEgtm_enable(&MODULE_EGTM);
        /* Set the global clock frequency to the max */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
        /* Set the CMU CLK0 */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, frequency);
        /* FXCLK: used by TOM and CLK0: used by ATOM */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* 8) Initialize unified PWM driver */
    IfxEgtm_Pwm_init(&g_egtmAtomState.pwm, &config);

    /* 9) Store initial duties and dead-times into module state */
    g_egtmAtomState.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtomState.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtomState.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtomState.dutyCycles[3] = channelConfig[3].duty;

    g_egtmAtomState.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtomState.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtomState.deadTimes[2]  = dtmConfig[2].deadTime;

    g_egtmAtomState.phases[0]     = channelConfig[0].phase;
    g_egtmAtomState.phases[1]     = channelConfig[1].phase;
    g_egtmAtomState.phases[2]     = channelConfig[2].phase;

    /* 10) Configure diagnostic GPIO for ISR toggling */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 11) Configure EGTM trigger route to TMADC/ADC
     * The routing API is platform-specific; integrate using IfxEgtm_Trigger_trigToAdc() to map
     * ATOM0 CH3 to AdcTriggerSignal_0 on falling edge, per project routing configuration.
     * Example (to be completed with validated enums in integration):
     *   IfxEgtm_Trigger_trigToAdc(&MODULE_EGTM, <AdcTriggerSignal_0>, <ATOM0_CH3>, <falling edge>);
     */

    /* 12) Start synchronized PWM channels: handled by syncStart=TRUE during init. */
}
