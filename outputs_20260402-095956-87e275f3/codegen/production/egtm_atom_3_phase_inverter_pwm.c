/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM ATOM 3-phase complementary PWM + ADC trigger (TC4xx).
 *
 * Implementation notes:
 *  - Uses unified IfxEgtm_Pwm driver per iLLD patterns.
 *  - EGTM clock enable guard follows authoritative enable pattern.
 *  - No watchdog API calls here (must be in CpuX_Main.c only).
 *  - ISR toggles a debug pin; unified driver uses callback routing internally.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD dependencies */
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"

/* ==============================
 * Numeric configuration macros
 * ============================== */
#define EGTM_NUM_PHASE_CHANNELS          (3U)
#define EGTM_NUM_ADC_TRIG_CHANNELS       (1U)

#define EGTM_PWM_SWITCHING_FREQ_HZ       (20000.0f)   /* 20 kHz */
#define EGTM_PHASE_U_INIT_DUTY_PC        (25.0f)
#define EGTM_PHASE_V_INIT_DUTY_PC        (50.0f)
#define EGTM_PHASE_W_INIT_DUTY_PC        (75.0f)
#define EGTM_PHASE_DUTY_STEP_PC          (0.01f)
#define EGTM_ADC_TRIG_DUTY_PC            (50.0f)

#define ISR_PRIORITY_ATOM                (100U)
#define ISR_PRIORITY_TMADC_RESULT        (25U)

/* ==============================
 * Pin selection macros (KIT_A3G_TC4D7_LITE, TC4D7)
 * User-requested pins (highest priority)
 * ============================== */
/* Complementary pairs: ATOM0 CH0..2 on P20.8..P20.13 */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ADC trigger output: ATOM0 CH3 on P33.0 */
#define ADC_TRIG_PIN (&IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT)

/* Debug ISR toggle pin (P03.9) as compound macro for two-arg call */
#define DEBUG_PIN    &MODULE_P03, 9

/* ==============================
 * Module state (persistent)
 * ============================== */

typedef struct
{
    IfxEgtm_Pwm             pwm;                                   /* Inverter PWM handle */
    IfxEgtm_Pwm_Channel     channels[EGTM_NUM_PHASE_CHANNELS];     /* Inverter channel state (persistent) */
    float32                 dutyCycles[EGTM_NUM_PHASE_CHANNELS];   /* Current duties in percent */
    float32                 phases[EGTM_NUM_PHASE_CHANNELS];       /* Phase offsets (unused = 0) */
    IfxEgtm_Pwm_DeadTime    deadTimes[EGTM_NUM_PHASE_CHANNELS];    /* Dead-times per channel */
} EgtmAtom3phInverter_State;

typedef struct
{
    IfxEgtm_Pwm             pwm;                                   /* ADC trigger PWM handle */
    IfxEgtm_Pwm_Channel     channels[EGTM_NUM_ADC_TRIG_CHANNELS];  /* ADC trigger channel state (persistent) */
    float32                 dutyCycle;                              /* Duty of ADC trigger channel */
} EgtmAtomAdcTrig_State;

typedef struct
{
    EgtmAtom3phInverter_State  inverter;
    EgtmAtomAdcTrig_State      adcTrig;
} EgtmAtomModule_State;

IFX_STATIC EgtmAtomModule_State g_egtmAtomState;

/* ===========================================
 * Forward ISR declarations (vector functions)
 * =========================================== */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
IFX_INTERRUPT(resultISR,       0, ISR_PRIORITY_TMADC_RESULT);

/* =====================================
 * Period-event callback (no operation)
 * ===================================== */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* No operation per design; ISR toggles debug pin */
}

/* ======================
 * ISR implementations
 * ====================== */
void interruptEgtmAtom(void)
{
    /* Minimal ISR body per design: toggle debug I/O */
    IfxPort_togglePin(DEBUG_PIN);
}

void resultISR(void)
{
    /* Minimal ISR body: toggle debug I/O */
    IfxPort_togglePin(DEBUG_PIN);

    /* Optional result handling example (guarded on NULL pointer to avoid side-effects here): */
    {
        Ifx_ADC_TMADC *tmadc = (Ifx_ADC_TMADC *)0; /* Provide a valid TMADC handle from ADC init when integrating */
        if (tmadc != (Ifx_ADC_TMADC *)0)
        {
            /* Channels 0..4 one-shot example */
            uint8 reg;
            for (reg = 0U; reg < 5U; ++reg)
            {
                if (IfxAdc_isTmadcResultAvailable(tmadc, (IfxAdc_TmadcResultReg)reg) == TRUE)
                {
                    IfxAdc_clearTmadcResultFlag(tmadc, (IfxAdc_TmadcResultReg)reg);
                }
            }
        }
    }
}

/* =========================
 * Initialization function
 * ========================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config            invConfig;
    IfxEgtm_Pwm_ChannelConfig     invChCfg[EGTM_NUM_PHASE_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      invOutCfg[EGTM_NUM_PHASE_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         invDtmCfg[EGTM_NUM_PHASE_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg;

    IfxEgtm_Pwm_Config            adcCfg;
    IfxEgtm_Pwm_ChannelConfig     adcChCfg[EGTM_NUM_ADC_TRIG_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      adcOutCfg[EGTM_NUM_ADC_TRIG_CHANNELS];

    /* 2) Initialize both config structs with defaults */
    IfxEgtm_Pwm_initConfig(&invConfig, &MODULE_EGTM);
    IfxEgtm_Pwm_initConfig(&adcCfg, &MODULE_EGTM);

    /* 3) Configure 3-phase inverter (ATOM0 CH0..2) */
    /* Output configuration: complementary, HS active-high, LS active-low */
    invOutCfg[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    invOutCfg[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    invOutCfg[0].polarity               = Ifx_ActiveState_high;
    invOutCfg[0].complementaryPolarity  = Ifx_ActiveState_low;
    invOutCfg[0].outputMode             = IfxPort_OutputMode_pushPull;
    invOutCfg[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    invOutCfg[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    invOutCfg[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    invOutCfg[1].polarity               = Ifx_ActiveState_high;
    invOutCfg[1].complementaryPolarity  = Ifx_ActiveState_low;
    invOutCfg[1].outputMode             = IfxPort_OutputMode_pushPull;
    invOutCfg[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    invOutCfg[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    invOutCfg[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    invOutCfg[2].polarity               = Ifx_ActiveState_high;
    invOutCfg[2].complementaryPolarity  = Ifx_ActiveState_low;
    invOutCfg[2].outputMode             = IfxPort_OutputMode_pushPull;
    invOutCfg[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: 1 us both edges */
    invDtmCfg[0].deadTime.rising = 1e-6f; invDtmCfg[0].deadTime.falling = 1e-6f; invDtmCfg[0].fastShutOff = (IfxEgtm_Pwm_FastShutoffConfig *)0;
    invDtmCfg[1].deadTime.rising = 1e-6f; invDtmCfg[1].deadTime.falling = 1e-6f; invDtmCfg[1].fastShutOff = (IfxEgtm_Pwm_FastShutoffConfig *)0;
    invDtmCfg[2].deadTime.rising = 1e-6f; invDtmCfg[2].deadTime.falling = 1e-6f; invDtmCfg[2].fastShutOff = (IfxEgtm_Pwm_FastShutoffConfig *)0;

    /* Interrupt configuration: pulse notification on CPU0; period event only for base channel */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = (IfxEgtm_Pwm_callBack)IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = (IfxEgtm_Pwm_callBack)0;

    /* Channel configurations: logical indices 0..2, no phase shift for center-aligned */
    invChCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    invChCfg[0].phase     = 0.0f;
    invChCfg[0].duty      = EGTM_PHASE_U_INIT_DUTY_PC;
    invChCfg[0].dtm       = &invDtmCfg[0];
    invChCfg[0].output    = &invOutCfg[0];
    invChCfg[0].mscOut    = (IfxEgtm_MscOut *)0;
    invChCfg[0].interrupt = &irqCfg; /* base logical channel only */

    invChCfg[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    invChCfg[1].phase     = 0.0f;
    invChCfg[1].duty      = EGTM_PHASE_V_INIT_DUTY_PC;
    invChCfg[1].dtm       = &invDtmCfg[1];
    invChCfg[1].output    = &invOutCfg[1];
    invChCfg[1].mscOut    = (IfxEgtm_MscOut *)0;
    invChCfg[1].interrupt = (IfxEgtm_Pwm_InterruptConfig *)0;

    invChCfg[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    invChCfg[2].phase     = 0.0f;
    invChCfg[2].duty      = EGTM_PHASE_W_INIT_DUTY_PC;
    invChCfg[2].dtm       = &invDtmCfg[2];
    invChCfg[2].output    = &invOutCfg[2];
    invChCfg[2].mscOut    = (IfxEgtm_MscOut *)0;
    invChCfg[2].interrupt = (IfxEgtm_Pwm_InterruptConfig *)0;

    /* Main inverter config fields */
    invConfig.cluster            = IfxEgtm_Cluster_0;
    invConfig.subModule          = IfxEgtm_Pwm_SubModule_atom;
    invConfig.alignment          = IfxEgtm_Pwm_Alignment_center;
    invConfig.numChannels        = (uint8)EGTM_NUM_PHASE_CHANNELS;
    invConfig.channels           = invChCfg;
    invConfig.frequency          = EGTM_PWM_SWITCHING_FREQ_HZ;
    invConfig.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;           /* ATOM clock source */
    invConfig.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;   /* DTM clock source */
#if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE
    invConfig.highResEnable      = FALSE;
    invConfig.dtmHighResEnable   = FALSE;
#endif
    invConfig.syncUpdateEnabled  = TRUE;
    invConfig.syncStart          = TRUE;

    /* 5) ADC trigger handle: single channel, edge-aligned, 50% duty, non-complementary */
    adcOutCfg[0].pin                    = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_PIN;
    adcOutCfg[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)0;
    adcOutCfg[0].polarity               = Ifx_ActiveState_high;
    adcOutCfg[0].complementaryPolarity  = Ifx_ActiveState_low; /* ignored */
    adcOutCfg[0].outputMode             = IfxPort_OutputMode_pushPull;
    adcOutCfg[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    adcChCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;  /* ATOM0 CH3 */
    adcChCfg[0].phase     = 0.0f;
    adcChCfg[0].duty      = EGTM_ADC_TRIG_DUTY_PC;
    adcChCfg[0].dtm       = (IfxEgtm_Pwm_DtmConfig *)0;  /* non-complementary */
    adcChCfg[0].output    = &adcOutCfg[0];
    adcChCfg[0].mscOut    = (IfxEgtm_MscOut *)0;
    adcChCfg[0].interrupt = (IfxEgtm_Pwm_InterruptConfig *)0; /* no ISR for ADC trigger channel */

    adcCfg.cluster            = IfxEgtm_Cluster_0;
    adcCfg.subModule          = IfxEgtm_Pwm_SubModule_atom;
    adcCfg.alignment          = IfxEgtm_Pwm_Alignment_edge;
    adcCfg.numChannels        = (uint8)EGTM_NUM_ADC_TRIG_CHANNELS;
    adcCfg.channels           = adcChCfg;
    adcCfg.frequency          = EGTM_PWM_SWITCHING_FREQ_HZ;      /* coherent with inverter */
    adcCfg.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;
    adcCfg.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;
#if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE
    adcCfg.highResEnable      = FALSE;
    adcCfg.dtmHighResEnable   = FALSE;
#endif
    adcCfg.syncUpdateEnabled  = TRUE;
    adcCfg.syncStart          = TRUE;

    /* 6) EGTM enable guard + CMU clock configuration (MANDATORY) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Initialize unified PWM drivers: inverter first, then ADC trigger */
    IfxEgtm_Pwm_init(&g_egtmAtomState.inverter.pwm, g_egtmAtomState.inverter.channels, &invConfig);
    IfxEgtm_Pwm_init(&g_egtmAtomState.adcTrig.pwm,   g_egtmAtomState.adcTrig.channels,   &adcCfg);

    /* 8) Route ADC trigger via EGTM Trigger API
       Note: Signal requirements: AdcTriggerSignal_0, TriggerMuxSel_36, falling edge.
       The available trigToAdc() signature maps source and channel; MUX/edge selection is handled by the trigger fabric configuration.
    */
    {
        /* ATOM0, channel 3 -> ADC trigger signal 0 */
        (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                        (IfxEgtm_TrigSource)0,          /* Source selection for ATOM0 (implementation-specific) */
                                        (IfxEgtm_TrigChannel)3,         /* Channel 3 */
                                        (IfxEgtm_Cfg_AdcTriggerSignal)0 /* AdcTriggerSignal_0 */);
    }

    /* 9) Store persistent state values (duties, phases, dead-times) */
    g_egtmAtomState.inverter.dutyCycles[0] = EGTM_PHASE_U_INIT_DUTY_PC;
    g_egtmAtomState.inverter.dutyCycles[1] = EGTM_PHASE_V_INIT_DUTY_PC;
    g_egtmAtomState.inverter.dutyCycles[2] = EGTM_PHASE_W_INIT_DUTY_PC;

    g_egtmAtomState.inverter.phases[0] = 0.0f;
    g_egtmAtomState.inverter.phases[1] = 0.0f;
    g_egtmAtomState.inverter.phases[2] = 0.0f;

    g_egtmAtomState.inverter.deadTimes[0] = invDtmCfg[0].deadTime;
    g_egtmAtomState.inverter.deadTimes[1] = invDtmCfg[1].deadTime;
    g_egtmAtomState.inverter.deadTimes[2] = invDtmCfg[2].deadTime;

    g_egtmAtomState.adcTrig.dutyCycle = EGTM_ADC_TRIG_DUTY_PC;

    /* 10) Configure debug I/O pin for ISR toggling */
    IfxPort_setPinModeOutput(DEBUG_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ==============================
 * Runtime duty update function
 * ============================== */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Clamp and store */
    float32 d0 = requestDuty[0]; if (d0 < 0.0f) d0 = 0.0f; if (d0 > 100.0f) d0 = 100.0f; g_egtmAtomState.inverter.dutyCycles[0] = d0;
    float32 d1 = requestDuty[1]; if (d1 < 0.0f) d1 = 0.0f; if (d1 > 100.0f) d1 = 100.0f; g_egtmAtomState.inverter.dutyCycles[1] = d1;
    float32 d2 = requestDuty[2]; if (d2 < 0.0f) d2 = 0.0f; if (d2 > 100.0f) d2 = 100.0f; g_egtmAtomState.inverter.dutyCycles[2] = d2;

    /* Immediate synchronous multi-channel update in percent */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtomState.inverter.pwm, (float32 *)g_egtmAtomState.inverter.dutyCycles);
}
