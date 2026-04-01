/*
 * egtm_atom_tmadc_consolidated.c
 * Consolidated eGTM (ATOM) + TMADC initialization for TC4xx
 */

#include "egtm_atom_tmadc_consolidated.h"

/* iLLD core types */
#include "Ifx_Types.h"

/* eGTM PWM unified driver */
#include "IfxEgtm_Pwm.h"
/* eGTM CMU functions (clock enable/setting) */
#include "IfxEgtm_Cmu.h"
/* eGTM ATOM PinMap helper for routing a TOUT to a pad */
#include "IfxEgtm_PinMap.h"
/* eGTM Trigger routing to ADC */
#include "IfxEgtm_Trigger.h"
/* ATOM timer helper (for ADC trigger channel) */
#include "IfxEgtm_Atom_Timer.h"

/* TMADC module interfaces */
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"

/* GPIO */
#include "IfxPort.h"

/* =====================================================================================
 * Configuration macros (values from USER-CONFIRMED MIGRATION VALUES / REQUIREMENTS)
 * ===================================================================================== */
#define NUM_OF_CHANNELS                 (3U)
#define PWM_FREQUENCY                   (20000.0f)
#define PHASE_U_DUTY                    (25.0f)
#define PHASE_V_DUTY                    (50.0f)
#define PHASE_W_DUTY                    (75.0f)
#define PHASE_DUTY_STEP                 (0.01f)
#define PWM_DEAD_TIME                   (1e-6f)

/* ISR priority for the eGTM ATOM PWM (debug ISR) */
#define ISR_PRIORITY_ATOM               (25)

/* LED on P03.9 (user requirement): compound macro expands to 2 args (port, pin) */
#define LED                             &MODULE_P03, 9

/* =====================================================================================
 * Pin selection macros (validated symbols only; fallback to NULL_PTR if not listed)
 * ===================================================================================== */
/* Phase U (ATOM0 Ch0) — validated list provides these exact symbols */
#define PHASE_U_HS                      (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                      (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)

/* Phase V (ATOM0 Ch1) — user requested P20.10/11; not present in validated list → NULL_PTR placeholders */
#define PHASE_V_HS                      (NULL_PTR) /* Requested P20.10 (ATOM0_1) — bind a valid TOUT symbol when available */
#define PHASE_V_LS                      (NULL_PTR) /* Requested P20.11 (ATOM0_1N) — bind a valid TOUT symbol when available */

/* Phase W (ATOM0 Ch2) — user requested P20.12/13; not present in validated list → NULL_PTR placeholders */
#define PHASE_W_HS                      (NULL_PTR) /* Requested P20.12 (ATOM0_2) — bind a valid TOUT symbol when available */
#define PHASE_W_LS                      (NULL_PTR) /* Requested P20.13 (ATOM0_2N) — bind a valid TOUT symbol when available */

/* ADC trigger output (ATOM0 Ch3) — user requested P33.0; not present in validated list → NULL_PTR placeholder */
#define ADC_TRIG_TOUT                   (NULL_PTR) /* Requested P33.0 (ATOM0_3) — bind a valid TOUT symbol when available */

/* =====================================================================================
 * Module state (persistent)
 * ===================================================================================== */

typedef struct
{
    IfxEgtm_Pwm               pwm;                                 /* unified eGTM PWM driver handle */
    IfxEgtm_Pwm_Channel       channels[NUM_OF_CHANNELS];           /* persistent channel objects (populated by init) */
    float32                   dutyCycles[NUM_OF_CHANNELS];         /* duty in percent */
    float32                   phases[NUM_OF_CHANNELS];             /* phase offset (0..1 or driver-specific units) */
    IfxEgtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];          /* per-channel deadtime (rising/falling) */
} EGTM_AtomTmadc_State;

IFX_STATIC EGTM_AtomTmadc_State g_egtmAtomTmadc = {0};

/* =====================================================================================
 * ISR and callback (must be defined before init)
 * ===================================================================================== */

/*
 * PWM ISR: minimal body — toggle LED for instrumentation
 * Note: ISR is installed by the driver infrastructure via InterruptConfig; here we only define it.
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback for PWM driver — empty body by design */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Optional TMADC result ISR placeholder per requirements (priority 25, TOS cpu0)
 * The actual SRC routing/enabling must be done in the ADC integration code. */
IFX_INTERRUPT(resultISR, 0, 25);
void resultISR(void)
{
    /* Per user requirement: read TMADC0.CH0–CH4 results here using IfxAdc_Tmadc_readChannelResult()
       and acknowledge/clear source as required. API not listed in available mocks, so left as placeholder. */
}

/* =====================================================================================
 * Initialization: eGTM (ATOM) complementary PWM + ATOM trigger + TMADC
 * ===================================================================================== */

/**
 * Initialize consolidated eGTM ATOM0 (3-phase complementary PWM @20 kHz with 1 us dead-time),
 * setup ATOM trigger channel for TMADC, route trigger to ADC, initialize TMADC module, and
 * configure LED GPIO. All hardware enabling is deferred until after configuration is ready.
 */
void initEgtmAtom3phInv(void)
{
    /* ---------------------------
     * 1) Local configuration data
     * --------------------------- */
    IfxEgtm_Pwm_Config            pwmConfig;
    IfxEgtm_Pwm_ChannelConfig     chCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      outCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg; /* used for base channel */

    IfxEgtm_Atom_Timer            adcTrigTimer; /* ATOM trigger channel driver */

    IfxAdc_Tmadc_Config           tmadcCfg;
    IfxAdc_Tmadc                  tmadc;

    /* ---------------------------
     * 2) PWM unified config setup
     * --------------------------- */
    IfxEgtm_Pwm_initConfig(&pwmConfig, &MODULE_EGTM);

    /* Complementary outputs and DTM for 3 logical channels (ATOM0 Ch0..Ch2) */
    /* Channel 0: Phase U */
    outCfg[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    outCfg[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    outCfg[0].polarity               = Ifx_ActiveState_high;    /* HS: active HIGH */
    outCfg[0].complementaryPolarity  = Ifx_ActiveState_low;     /* LS: active LOW  */
    outCfg[0].outputMode             = IfxPort_OutputMode_pushPull;
    outCfg[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 1: Phase V */
    outCfg[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS; /* NULL_PTR placeholder until pin is bound */
    outCfg[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS; /* NULL_PTR placeholder until pin is bound */
    outCfg[1].polarity               = Ifx_ActiveState_high;
    outCfg[1].complementaryPolarity  = Ifx_ActiveState_low;
    outCfg[1].outputMode             = IfxPort_OutputMode_pushPull;
    outCfg[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 2: Phase W */
    outCfg[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS; /* NULL_PTR placeholder until pin is bound */
    outCfg[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS; /* NULL_PTR placeholder until pin is bound */
    outCfg[2].polarity               = Ifx_ActiveState_high;
    outCfg[2].complementaryPolarity  = Ifx_ActiveState_low;
    outCfg[2].outputMode             = IfxPort_OutputMode_pushPull;
    outCfg[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM dead-time for each channel: 1e-6 s rising/falling */
    dtmCfg[0].deadTime.rising = PWM_DEAD_TIME; dtmCfg[0].deadTime.falling = PWM_DEAD_TIME; dtmCfg[0].fastShutOff = NULL_PTR;
    dtmCfg[1].deadTime.rising = PWM_DEAD_TIME; dtmCfg[1].deadTime.falling = PWM_DEAD_TIME; dtmCfg[1].fastShutOff = NULL_PTR;
    dtmCfg[2].deadTime.rising = PWM_DEAD_TIME; dtmCfg[2].deadTime.falling = PWM_DEAD_TIME; dtmCfg[2].fastShutOff = NULL_PTR;

    /* Interrupt configuration for base channel: period-event callback (empty) */
    irqCfg.mode        = (IfxEgtm_IrqMode)0;          /* default/implementation-defined */
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* Channel configurations (logical indices map to ATOM Ch0..Ch2) */
    chCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0; chCfg[0].phase = 0.0f;       chCfg[0].duty = PHASE_U_DUTY;
    chCfg[0].dtm       = &dtmCfg[0];                  chCfg[0].output = &outCfg[0]; chCfg[0].mscOut = NULL_PTR; chCfg[0].interrupt = &irqCfg;

    chCfg[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1; chCfg[1].phase = 0.0f;       chCfg[1].duty = PHASE_V_DUTY;
    chCfg[1].dtm       = &dtmCfg[1];                  chCfg[1].output = &outCfg[1]; chCfg[1].mscOut = NULL_PTR; chCfg[1].interrupt = NULL_PTR;

    chCfg[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2; chCfg[2].phase = 0.0f;       chCfg[2].duty = PHASE_W_DUTY;
    chCfg[2].dtm       = &dtmCfg[2];                  chCfg[2].output = &outCfg[2]; chCfg[2].mscOut = NULL_PTR; chCfg[2].interrupt = NULL_PTR;

    /* Main PWM config */
    pwmConfig.cluster             = IfxEgtm_Cluster_0;
    pwmConfig.subModule           = IfxEgtm_Pwm_SubModule_atom;           /* ATOM submodule */
    pwmConfig.alignment           = IfxEgtm_Pwm_Alignment_center;         /* center-aligned */
    pwmConfig.numChannels         = (uint8)NUM_OF_CHANNELS;
    pwmConfig.channels            = &chCfg[0];
    pwmConfig.frequency           = PWM_FREQUENCY;                        /* 20 kHz */
    pwmConfig.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;            /* CMU Clock 0 for ATOM */
    pwmConfig.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;    /* DTM clock source: CMU Clock 0 */
    pwmConfig.syncUpdateEnabled   = TRUE;                                  /* keep sync update enabled */
    pwmConfig.syncStart           = TRUE;                                  /* sync start all channels */

    /* ---------------------------------------------
     * 3) ATOM trigger channel (separate timer chan)
     * --------------------------------------------- */
    /* Set its frequency to match PWM (20 kHz), set 50% trigger point later after PWM init when period is known */

    /* -------------------------------------------------------
     * 4) Configure trigger routing from ATOM to ADC (TMADC)
     * ------------------------------------------------------- */
    /* Route ATOM0.CH3 to ADC trigger signal 0. Edge/mux selection specifics are not available in the mock API. */
    (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                    (IfxEgtm_TrigSource)0,     /* Source: ATOM0 (implementation-defined enum value 0) */
                                    (IfxEgtm_TrigChannel)3,    /* Channel: CH3 */
                                    (IfxEgtm_Cfg_AdcTriggerSignal)0); /* AdcTriggerSignal_0 */

    /* --------------------------------------------------------------
     * 5) Enable-guard: enable eGTM, configure CMU clocks inside guard
     * -------------------------------------------------------------- */
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

    /* ----------------------------------
     * 6) Initialize unified eGTM PWM
     * ---------------------------------- */
    IfxEgtm_Pwm_init(&g_egtmAtomTmadc.pwm, &g_egtmAtomTmadc.channels[0], &pwmConfig);

    /* Persist initial duties/phases/dead-times into module state */
    g_egtmAtomTmadc.dutyCycles[0] = chCfg[0].duty; g_egtmAtomTmadc.phases[0] = chCfg[0].phase; g_egtmAtomTmadc.deadTimes[0] = dtmCfg[0].deadTime;
    g_egtmAtomTmadc.dutyCycles[1] = chCfg[1].duty; g_egtmAtomTmadc.phases[1] = chCfg[1].phase; g_egtmAtomTmadc.deadTimes[1] = dtmCfg[1].deadTime;
    g_egtmAtomTmadc.dutyCycles[2] = chCfg[2].duty; g_egtmAtomTmadc.phases[2] = chCfg[2].phase; g_egtmAtomTmadc.deadTimes[2] = dtmCfg[2].deadTime;

    /* -------------------------------------------------
     * 7) Initialize and start ATOM timer for trigger
     * ------------------------------------------------- */
    {
        boolean ok;
        ok = IfxEgtm_Atom_Timer_setFrequency(&adcTrigTimer, PWM_FREQUENCY); /* match PWM period */
        (void)ok; /* production: handle error as needed */
        /* 50% trigger point based on PWM ticks configured by driver */
        IfxEgtm_Atom_Timer_setTrigger(&adcTrigTimer, (uint32)(g_egtmAtomTmadc.pwm.periodTicks / 2U));
        IfxEgtm_Atom_Timer_run(&adcTrigTimer);
    }

    /* --------------------------------------------------------------
     * 8) Connect trigger channel output to pad using PinMap API
     * -------------------------------------------------------------- */
    if (ADC_TRIG_TOUT != NULL_PTR)
    {
        IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)ADC_TRIG_TOUT,
                                   IfxPort_OutputMode_pushPull,
                                   IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }

    /* ---------------------------
     * 9) Initialize TMADC module
     * --------------------------- */
    IfxAdc_enableModule(&MODULE_ADC);
    IfxAdc_Tmadc_initModuleConfig(&tmadcCfg, &MODULE_ADC);
    /* Per requirements: configure TMADC0.CH0–CH4 one-shot, 100 ns sampling, wait-for-read,
       enable channel event and ISR installation. Channel-level APIs are not available in the
       mock interface; complete channel setup must be added during integration. */
    IfxAdc_Tmadc_initModule(&tmadc, &tmadcCfg);

    /* ---------------------------------
     * 10) LED pin configured as output
     * --------------------------------- */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull);
    IfxPort_setPinPadDriver(LED, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}
