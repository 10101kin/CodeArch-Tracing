/*
 * egtm_atom_tmadc_consolidated.c
 *
 * Consolidated TC4xx production driver replacing TC3xx GTM TOM + EVADC with
 * eGTM ATOM + TMADC on KIT_A3G_TC4D7_LITE.
 *
 * Implements:
 *  - eGTM ATOM0 CH0..CH2 center-aligned complementary PWM at 20 kHz with 1 us dead-time
 *  - ATOM0 CH3 50% duty trigger for TMADC routing
 *  - eGTM CMU enable/clock setup via enable guard
 *  - TMADC module enable + module init
 *  - LED GPIO prepared for application toggling
 *
 * Notes:
 *  - Watchdogs are NOT modified here (per AURIX architecture: only in CpuX_Main.c)
 *  - ADC channel pin binding and ISR/SRC setup are board- and project-specific; placeholders kept
 */

#include "egtm_atom_tmadc_consolidated.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Atom_Timer.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"

/* =============================
 * Numeric configuration macros
 * ============================= */
#define NUM_OF_CHANNELS                (3U)
#define PWM_FREQUENCY_HZ               (20000.0f)
#define PHASE_U_DUTY                   (25.0f)
#define PHASE_V_DUTY                   (50.0f)
#define PHASE_W_DUTY                   (75.0f)
#define PHASE_DUTY_STEP                (0.01f)
#define PWM_DEAD_TIME_S                (1.0e-6f)

/* ISR priority for eGTM ATOM base interrupt (used by PWM driver) */
#define ISR_PRIORITY_ATOM              (25)

/* LED on P03.9 (compound macro: port, pin) */
#define LED                            &MODULE_P03, 9

/* =============================
 * Validated ATOM TOUT pin macros
 * =============================
 * Use only validated pin symbols from provided list. For pins not listed,
 * leave as NULL_PTR placeholders to be bound during board integration.
 */
/* Phase U (ATOM0 CH0) — validated */
#define PHASE_U_HS                     (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                     (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)

/* Phase V (ATOM0 CH1) — user requested P20.10 / P20.11: placeholders (TBD) */
#define PHASE_V_HS                     (NULL_PTR) /* Replace with validated TOUT for P20.10 */
#define PHASE_V_LS                     (NULL_PTR) /* Replace with validated TOUT for P20.11 */

/* Phase W (ATOM0 CH2) — user requested P20.12 / P20.13: placeholders (TBD) */
#define PHASE_W_HS                     (NULL_PTR) /* Replace with validated TOUT for P20.12 */
#define PHASE_W_LS                     (NULL_PTR) /* Replace with validated TOUT for P20.13 */

/* ATOM0 CH3 trigger output — user requested P33.0: placeholder (TBD) */
#define TRIG_ATOM_TOUT                 (NULL_PTR) /* Replace with validated TOUT for P33.0 */

/* =============================
 * Module state
 * ============================= */

typedef struct
{
    IfxEgtm_Pwm             pwm;                                   /* PWM driver handle */
    IfxEgtm_Pwm_Channel     channels[NUM_OF_CHANNELS];             /* Persistent channels array */
    float32                 dutyCycles[NUM_OF_CHANNELS];           /* Duty in percent [0..100] */
    float32                 phases[NUM_OF_CHANNELS];               /* Phase in percent [0..100] */
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];            /* Stored dead-times */
} EgtmAtom3ph_State;

IFX_STATIC EgtmAtom3ph_State g_egtmAtom3phState;

/* =============================
 * ISR and period callback
 * ============================= */

/* PWM interrupt: minimal ISR toggles debug LED only */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback for PWM driver — empty by design */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Initialization function
 * ============================= */

/**
 * Initialize eGTM ATOM 3-phase inverter PWM (CH0..CH2), ATOM trigger (CH3), and TMADC module.
 *
 * Sequence:
 *  1) Build all config locally; keep PWM handle + channels persistent in module state.
 *  2) Configure 3 logical channels (0..2) as complementary, center-aligned, with 1us DTM dead-time;
 *     initial duties 25/50/75 percent; CMU Clock 0 as PWM/DTM clock; sync update enabled; 20kHz.
 *  3) Prepare ATOM timer (separate channel) to generate ADC trigger at 50% of PWM period.
 *  4) Route trigger from ATOM0.CH3 to ADC trigger signal via IfxEgtm_Trigger_trigToAdc.
 *  5) Enable-guard eGTM; set GCLK and CLK0 based on runtime module frequency; enable FXCLK|CLK0.
 *  6) Initialize unified eGTM PWM with persistent channels array.
 *  7) Start ATOM timer and set 50% trigger point.
 *  8) Bind trigger channel to pad using PinMap API.
 *  9) Enable TMADC module and initialize module configuration.
 * 10) Configure LED GPIO as push-pull output.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration containers */
    IfxEgtm_Pwm_Config            pwmCfg;
    IfxEgtm_Pwm_ChannelConfig     chCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      outCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg;

    /* Initialize unified eGTM PWM defaults */
    IfxEgtm_Pwm_initConfig(&pwmCfg, &MODULE_EGTM);

    /* 2) Output configuration: complementary pairs with required polarity */
    /* Phase U (logical channel 0) */
    outCfg[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    outCfg[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    outCfg[0].polarity               = Ifx_ActiveState_high;    /* HS active high */
    outCfg[0].complementaryPolarity  = Ifx_ActiveState_low;     /* LS active low  */
    outCfg[0].outputMode             = IfxPort_OutputMode_pushPull;
    outCfg[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (logical channel 1) */
    outCfg[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS; /* TBD */
    outCfg[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS; /* TBD */
    outCfg[1].polarity               = Ifx_ActiveState_high;
    outCfg[1].complementaryPolarity  = Ifx_ActiveState_low;
    outCfg[1].outputMode             = IfxPort_OutputMode_pushPull;
    outCfg[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (logical channel 2) */
    outCfg[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS; /* TBD */
    outCfg[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS; /* TBD */
    outCfg[2].polarity               = Ifx_ActiveState_high;
    outCfg[2].complementaryPolarity  = Ifx_ActiveState_low;
    outCfg[2].outputMode             = IfxPort_OutputMode_pushPull;
    outCfg[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 2) DTM configuration: 1us rising/falling dead-time, DTM clock = CMU CLK0 */
    dtmCfg[0].deadTime.rising = PWM_DEAD_TIME_S;
    dtmCfg[0].deadTime.falling = PWM_DEAD_TIME_S;
    dtmCfg[0].fastShutOff = NULL_PTR;

    dtmCfg[1].deadTime.rising = PWM_DEAD_TIME_S;
    dtmCfg[1].deadTime.falling = PWM_DEAD_TIME_S;
    dtmCfg[1].fastShutOff = NULL_PTR;

    dtmCfg[2].deadTime.rising = PWM_DEAD_TIME_S;
    dtmCfg[2].deadTime.falling = PWM_DEAD_TIME_S;
    dtmCfg[2].fastShutOff = NULL_PTR;

    /* 2) Interrupt configuration (attach to base logical channel 0) */
    irqCfg.mode        = (IfxEgtm_IrqMode)0;             /* pulse/notify mode per project defaults */
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 2) Channel configurations: CH0..CH2 center-aligned complementary */
    chCfg[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    chCfg[0].phase      = 0.0f;
    chCfg[0].duty       = PHASE_U_DUTY;
    chCfg[0].dtm        = &dtmCfg[0];
    chCfg[0].output     = &outCfg[0];
    chCfg[0].mscOut     = NULL_PTR;
    chCfg[0].interrupt  = &irqCfg; /* assign interrupt on base channel */

    chCfg[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    chCfg[1].phase      = 0.0f;
    chCfg[1].duty       = PHASE_V_DUTY;
    chCfg[1].dtm        = &dtmCfg[1];
    chCfg[1].output     = &outCfg[1];
    chCfg[1].mscOut     = NULL_PTR;
    chCfg[1].interrupt  = NULL_PTR;

    chCfg[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    chCfg[2].phase      = 0.0f;
    chCfg[2].duty       = PHASE_W_DUTY;
    chCfg[2].dtm        = &dtmCfg[2];
    chCfg[2].output     = &outCfg[2];
    chCfg[2].mscOut     = NULL_PTR;
    chCfg[2].interrupt  = NULL_PTR;

    /* 2) Main PWM configuration — ATOM submodule, center-aligned, sync update */
    pwmCfg.cluster             = IfxEgtm_Cluster_0;
    pwmCfg.subModule           = IfxEgtm_Pwm_SubModule_atom;
    pwmCfg.alignment           = IfxEgtm_Pwm_Alignment_center;
    pwmCfg.numChannels         = (uint8)NUM_OF_CHANNELS;
    pwmCfg.channels            = chCfg;
    pwmCfg.frequency           = PWM_FREQUENCY_HZ;
    pwmCfg.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;         /* CMU CLK0 for ATOM */
    pwmCfg.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM sourced by CMU CLK0 */
    pwmCfg.syncUpdateEnabled   = TRUE;
    pwmCfg.syncStart           = TRUE;

    /* 5) eGTM enable guard and CMU clock setup */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Runtime-determined module frequency */
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Configure GCLK and CLK0 based on runtime frequency context */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, (uint32)freq, 1U);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, (uint32)freq);
        /* Enable FXCLK (for TOM) and CLK0 (for ATOM) */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize unified eGTM PWM with persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phState.pwm, g_egtmAtom3phState.channels, &pwmCfg);

    /* 10) Configure LED pin (push-pull output) for application use */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinPadDriver(LED, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 7) ATOM timer on trigger channel: set frequency and 50% trigger point, then run */
    {
        IfxEgtm_Atom_Timer atomTimer; /* Local driver context (channel selection is implementation-specific) */
        boolean ok = IfxEgtm_Atom_Timer_setFrequency(&atomTimer, PWM_FREQUENCY_HZ);
        if (ok == TRUE)
        {
            /* Use PWM periodTicks/2 as a 50% trigger reference */
            IfxEgtm_Atom_Timer_setTrigger(&atomTimer, (uint32)(g_egtmAtom3phState.pwm.periodTicks / 2U));
            IfxEgtm_Atom_Timer_run(&atomTimer);
        }
        else
        {
            /* Error path: leave timer stopped (could log or assert in integration) */
        }
    }

    /* 8) Bind trigger channel output to pad via PinMap API (placeholder until final pin binding) */
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)TRIG_ATOM_TOUT, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) Route ATOM trigger to ADC trigger signal using eGTM Trigger API */
    {
        /* Cluster 0, ATOM0 source, logical trigger channel 3, ADC trigger signal 0 */
        boolean routed = IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0, (IfxEgtm_TrigSource)0, (IfxEgtm_TrigChannel)3, (IfxEgtm_Cfg_AdcTriggerSignal)0);
        (void)routed; /* In production, handle FALSE (routing failure) accordingly */
    }

    /* 9) TMADC: enable module and initialize module configuration */
    {
        IfxAdc_Tmadc_Config adcCfg;
        IfxAdc_Tmadc       tmadc;
        IfxAdc_enableModule(&MODULE_ADC);
        IfxAdc_Tmadc_initModuleConfig(&adcCfg, &MODULE_ADC);
        /* Channel one-shot, sampling time, ISR/SRC, and run/start to be configured by project-specific code */
        IfxAdc_Tmadc_initModule(&tmadc, &adcCfg);
    }

    /* Persist initial duties, phases, and dead-times into module state for runtime updates */
    g_egtmAtom3phState.dutyCycles[0] = chCfg[0].duty;
    g_egtmAtom3phState.dutyCycles[1] = chCfg[1].duty;
    g_egtmAtom3phState.dutyCycles[2] = chCfg[2].duty;

    g_egtmAtom3phState.phases[0] = chCfg[0].phase;
    g_egtmAtom3phState.phases[1] = chCfg[1].phase;
    g_egtmAtom3phState.phases[2] = chCfg[2].phase;

    g_egtmAtom3phState.deadTimes[0] = dtmCfg[0].deadTime;
    g_egtmAtom3phState.deadTimes[1] = dtmCfg[1].deadTime;
    g_egtmAtom3phState.deadTimes[2] = dtmCfg[2].deadTime;
}
