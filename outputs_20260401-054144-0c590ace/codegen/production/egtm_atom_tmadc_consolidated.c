/*
 * egtm_atom_tmadc_consolidated.c
 *
 * Consolidated TC4xx eGTM ATOM + TMADC production driver (migration from TC3xx GTM TOM + EVADC)
 *
 * Notes:
 * - Follows iLLD unified PWM init pattern and enable-guard for eGTM CMU clocks
 * - Uses ATOM submodule for 3-phase complementary center-aligned PWM with DTM dead-time
 * - Configures a separate ATOM timer channel as ADC trigger source and routes it to TMADC
 * - Installs PWM ISR (interruptEgtmAtom) that only toggles LED as per ISR guideline
 * - TMADC module is enabled and initialized with default config; channel-level setup and ISR hookup
 *   placeholders are left for integration (API not available in current mock set)
 * - Watchdog disable is NOT present here (must be in CpuX_Main.c only)
 */

#include "egtm_atom_tmadc_consolidated.h"

/* iLLD base & driver headers */
#include "Ifx_Types.h"
#include "Compilers.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Atom_Timer.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (numeric constants) ========================= */
#define NUM_OF_CHANNELS                (3u)
#define PWM_FREQUENCY                  (20000.0f)    /* Hz */
#define PHASE_U_DUTY                   (25.0f)       /* percent */
#define PHASE_V_DUTY                   (50.0f)       /* percent */
#define PHASE_W_DUTY                   (75.0f)       /* percent */
#define PHASE_DUTY_STEP                (0.01f)       /* percent step (not used here; for runtime updates) */
#define ADC_TRIG_DUTY                  (50.0f)       /* percent (logical; ATOM trigger set to 50%) */

/* ISR priority for PWM (ATOM). Keep distinct from ADC result ISR priority (25). */
#define ISR_PRIORITY_ATOM              (100)

/* LED macro (compound port+pin) — P03.9 as requested */
#define LED                            &MODULE_P03, 9

/* ========================= Pin macros (validated symbols only) ========================= */
/* Phase U (ATOM0.CH0) — validated */
#define PHASE_U_HS                     (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                     (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)

/* Phase V (ATOM0.CH1) — user requested P20.10/11; placeholders until validated symbols confirmed */
#define PHASE_V_HS                     (NULL_PTR)    /* TODO: bind to valid &IfxEgtm_ATOM0_1_TOUTxx_P20_10_OUT when available */
#define PHASE_V_LS                     (NULL_PTR)    /* TODO: bind to valid &IfxEgtm_ATOM0_1N_TOUTxx_P20_11_OUT when available */

/* Phase W (ATOM0.CH2) — user requested P20.12/13; placeholders until validated symbols confirmed */
#define PHASE_W_HS                     (NULL_PTR)    /* TODO: bind to valid &IfxEgtm_ATOM0_2_TOUTxx_P20_12_OUT when available */
#define PHASE_W_LS                     (NULL_PTR)    /* TODO: bind to valid &IfxEgtm_ATOM0_2N_TOUTxx_P20_13_OUT when available */

/* Trigger output (ATOM0.CH3) — user requested P33.0; placeholder until validated symbol confirmed */
#define ATOM_TRIG_TOUT                 (NULL_PTR)    /* TODO: bind to valid &IfxEgtm_ATOM0_3_TOUTxx_P33_0_OUT when available */

/* ========================= Module state ========================= */
typedef struct
{
    IfxEgtm_Pwm            pwm;                               /* unified eGTM PWM driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];         /* persistent channel objects (owned by driver) */
    float32                dutyCycles[NUM_OF_CHANNELS];       /* persisted duty in percent for runtime updates */
    float32                phases[NUM_OF_CHANNELS];           /* phase offsets (0 by default) */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];        /* persisted dead-time values */
} EgtmAtomTmadc_State;

IFX_STATIC EgtmAtomTmadc_State g_state;  /* persistent module-level state */

/* ========================= ISR and callbacks ========================= */
/* PWM ISR (ATOM) — body must only toggle LED as per guideline */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period event callback (empty body by rule) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Optional TMADC result ISR stub (priority 25, cpu0) — body left empty in this module */
IFX_INTERRUPT(resultISR, 0, 25)
{
    /* Integration note: read TMADC0.CH0–CH4 and clear source here when IfxAdc_Tmadc_readChannelResult() is available */
}

/* ========================= API implementation ========================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all config structs locally (state arrays persist in g_state) */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    IfxEgtm_Atom_Timer           triggerTimer;          /* logical ATOM timer for ADC trigger */

    IfxAdc_Tmadc_Config          tmadcConfig;
    IfxAdc_Tmadc                 tmadc;

    /* 2) Initialize unified eGTM PWM configuration */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output configuration: complementary, center-aligned, with DTM; polarity convention per guideline */
    /* Phase U (CH0) */
    output[0].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;   /* high-side active HIGH */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;    /* low-side  active LOW  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (CH1) */
    output[1].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (CH2) */
    output[2].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: 1 us rising/falling, DTM clock CMU Clock 0 */
    dtmConfig[0].deadTime.rising      = 1e-6f;
    dtmConfig[0].deadTime.falling     = 1e-6f;
    dtmConfig[0].fastShutOff          = NULL_PTR;

    dtmConfig[1].deadTime.rising      = 1e-6f;
    dtmConfig[1].deadTime.falling     = 1e-6f;
    dtmConfig[1].fastShutOff          = NULL_PTR;

    dtmConfig[2].deadTime.rising      = 1e-6f;
    dtmConfig[2].deadTime.falling     = 1e-6f;
    dtmConfig[2].fastShutOff          = NULL_PTR;

    /* Interrupt configuration for PWM base channel */
    interruptConfig.mode              = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider       = IfxSrc_Tos_cpu0;
    interruptConfig.priority          = ISR_PRIORITY_ATOM;
    interruptConfig.vmId              = IfxSrc_VmId_0;
    interruptConfig.periodEvent       = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent         = NULL_PTR;

    /* Channel configurations (logical indices 0..2) */
    /* CH0 → Phase U */
    channelConfig[0].timerCh          = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase            = 0.0f;
    channelConfig[0].duty             = PHASE_U_DUTY;
    channelConfig[0].dtm              = &dtmConfig[0];
    channelConfig[0].output           = &output[0];
    channelConfig[0].mscOut           = NULL_PTR;
    channelConfig[0].interrupt        = &interruptConfig;  /* base channel provides interrupt */

    /* CH1 → Phase V */
    channelConfig[1].timerCh          = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase            = 0.0f;
    channelConfig[1].duty             = PHASE_V_DUTY;
    channelConfig[1].dtm              = &dtmConfig[1];
    channelConfig[1].output           = &output[1];
    channelConfig[1].mscOut           = NULL_PTR;
    channelConfig[1].interrupt        = NULL_PTR;

    /* CH2 → Phase W */
    channelConfig[2].timerCh          = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase            = 0.0f;
    channelConfig[2].duty             = PHASE_W_DUTY;
    channelConfig[2].dtm              = &dtmConfig[2];
    channelConfig[2].output           = &output[2];
    channelConfig[2].mscOut           = NULL_PTR;
    channelConfig[2].interrupt        = NULL_PTR;

    /* Main PWM configuration */
    config.cluster                    = IfxEgtm_Cluster_0;
    config.subModule                  = IfxEgtm_Pwm_SubModule_atom;
    config.alignment                  = IfxEgtm_Pwm_Alignment_center;
    config.numChannels                = (uint8)NUM_OF_CHANNELS;
    config.channels                   = channelConfig;
    config.frequency                  = PWM_FREQUENCY;
    config.clockSource.atom           = (uint32)IfxEgtm_Cmu_Clk_0;            /* CMU Clock 0 for ATOM */
    config.dtmClockSource             = IfxEgtm_Dtm_ClockSource_cmuClock0;    /* DTM uses CMU Clock 0 */
    config.syncUpdateEnabled          = TRUE;
    config.syncStart                  = TRUE;

    /* 3) Initialize ATOM timer (separate logical trigger channel) —
       Using available API to set frequency and 50% trigger point */
    {
        (void)IfxEgtm_Atom_Timer_setFrequency(&triggerTimer, PWM_FREQUENCY); /* match PWM period */
        /* Set 50% trigger point (abstract units; integration to adjust to ticks if required) */
        IfxEgtm_Atom_Timer_setTrigger(&triggerTimer, (uint32)50);
    }

    /* 4) Configure routing from ATOM trigger source to ADC trigger signal */
    {
        (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                        (IfxEgtm_TrigSource)0,          /* EGTM.ATOM source (implementation-specific) */
                                        (IfxEgtm_TrigChannel)3,         /* logical trigger channel index */
                                        (IfxEgtm_Cfg_AdcTriggerSignal)0 /* AdcTriggerSignal_0 */);
    }

    /* 5) Enable-guard for eGTM and CMU clocks — EXACT pattern */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent channels array */
    IfxEgtm_Pwm_init(&g_state.pwm, g_state.channels, &config);

    /* 7) Start ATOM trigger timer (after PWM init) */
    IfxEgtm_Atom_Timer_run(&triggerTimer);

    /* 8) Configure trigger channel's output pad via generic PinMap API (if mapping provided) */
    if (ATOM_TRIG_TOUT != NULL_PTR)
    {
        IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)ATOM_TRIG_TOUT,
                                   IfxPort_OutputMode_pushPull,
                                   IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }

    /* 9) Initialize TMADC module: enable + module init with default config
          Channel-level one-shot setup and ISR hookup shall be completed during integration
          when corresponding APIs are available in the build environment. */
    IfxAdc_enableModule(&MODULE_ADC);
    IfxAdc_Tmadc_initModuleConfig(&tmadcConfig, &MODULE_ADC);
    IfxAdc_Tmadc_initModule(&tmadc, &tmadcConfig);

    /* 10) Configure LED pin for application use (push-pull) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Persist initial duties and dead-times to module state for runtime updates */
    g_state.dutyCycles[0] = PHASE_U_DUTY;    g_state.phases[0] = 0.0f;  g_state.deadTimes[0] = dtmConfig[0].deadTime;
    g_state.dutyCycles[1] = PHASE_V_DUTY;    g_state.phases[1] = 0.0f;  g_state.deadTimes[1] = dtmConfig[1].deadTime;
    g_state.dutyCycles[2] = PHASE_W_DUTY;    g_state.phases[2] = 0.0f;  g_state.deadTimes[2] = dtmConfig[2].deadTime;

    /* Enable CPU interrupts globally */
    IfxCpu_enableInterrupts();
}
