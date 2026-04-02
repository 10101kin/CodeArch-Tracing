/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production-ready EGTM ATOM 3-phase complementary PWM + ADC trigger (TC4xx).
 *
 * Notes:
 * - No watchdog disable here (Cpu0_Main.c only per AURIX standard).
 * - Follows unified IfxEgtm_Pwm init pattern with EGTM CMU enable guard.
 * - Uses validated/user-requested EGTM ATOM pins.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc.h"
#include "IfxPort.h"

/* =============================
 * Configuration Macros (from requirements)
 * ============================= */
#define NUM_OF_CHANNELS                 (3u)
#define NUM_OF_ADC_TRIG_CHANNELS        (1u)

#define PWM_SWITCH_FREQ_HZ              (20000.0f)

#define PHASE_U_DUTY_INIT               (25.0f)
#define PHASE_V_DUTY_INIT               (50.0f)
#define PHASE_W_DUTY_INIT               (75.0f)
#define PHASE_DUTY_STEP                 (0.01f)

/* Interrupt priorities */
#define ISR_PRIORITY_ATOM               (100)

/* Debug LED (ISR toggle pin): P03.9 */
#define EGTM_DEBUG_LED                  &MODULE_P03, 9

/* =============================
 * Pin Routing Macros (validated/user-requested)
 * ============================= */
#define PHASE_U_HS                      (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                      (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                      (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                      (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                      (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                      (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

#define ADC_TRIG_PIN                    (&IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT)

/* =============================
 * Module State
 * ============================= */
typedef struct
{
    IfxEgtm_Pwm            pwm;                                 /* unified PWM driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];           /* persistent channel handles */
    float32                dutyCycles[NUM_OF_CHANNELS];          /* percent 0..100 */
    float32                phases[NUM_OF_CHANNELS];              /* phase offset (percent of period) */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];          /* per-channel dead-time (seconds) */
} EgtmAtom3ph_State;

typedef struct
{
    IfxEgtm_Pwm            pwm;                                 /* ADC trigger PWM driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_ADC_TRIG_CHANNELS];  /* persistent channel handles */
    float32                dutyCycle;                           /* percent 0..100 */
} EgtmAtomAdcTrig_State;

IFX_STATIC EgtmAtom3ph_State     g_egtmAtom3phState;
IFX_STATIC EgtmAtomAdcTrig_State g_egtmAdcTrigState;

/* =============================
 * ISR and Callback Declarations
 * ============================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

void IfxEgtm_periodEventFunction(void *data);

void resultISR(void);

/* =============================
 * ISR and Callback Implementations
 * ============================= */
void interruptEgtmAtom(void)
{
    /* Minimal ISR: toggle debug pin to observe PWM period events */
    IfxPort_togglePin(EGTM_DEBUG_LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    /* Unified EGTM PWM driver period event callback - intentionally empty */
    (void)data;
}

void resultISR(void)
{
    /* Minimal TMADC result ISR: toggle debug pin */
    IfxPort_togglePin(EGTM_DEBUG_LED);

    /* Optional: check and clear TMADC result flags if a handle is available */
    {
        Ifx_ADC_TMADC *tmadc = (Ifx_ADC_TMADC *)0; /* to be provided by TMADC integration */
        if (tmadc != NULL_PTR)
        {
            /* Example result registers 0..4 */
            IfxAdc_TmadcResultReg rr;
            for (rr = (IfxAdc_TmadcResultReg)0; rr <= (IfxAdc_TmadcResultReg)4; rr++)
            {
                if (IfxAdc_isTmadcResultAvailable(tmadc, rr))
                {
                    IfxAdc_clearTmadcResultFlag(tmadc, rr);
                }
            }
        }
    }
}

/* =============================
 * Initialization
 * ============================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config            invCfg;
    IfxEgtm_Pwm_ChannelConfig     invChCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      invOutCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         invDtmCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   invIrqCfg;

    IfxEgtm_Pwm_Config            adcCfg;
    IfxEgtm_Pwm_ChannelConfig     adcChCfg[NUM_OF_ADC_TRIG_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      adcOutCfg[NUM_OF_ADC_TRIG_CHANNELS];

    /* 2) Initialize both config structs with defaults */
    IfxEgtm_Pwm_initConfig(&invCfg, &MODULE_EGTM);
    IfxEgtm_Pwm_initConfig(&adcCfg, &MODULE_EGTM);

    /* 3) Configure 3-phase inverter complementary outputs and DTM dead-times (1 us) */
    /* Output configuration: complementary, high-side active HIGH, low-side active LOW */
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

    /* Dead-time: 1 us on both rising and falling edges */
    invDtmCfg[0].deadTime.rising = 1e-6f; invDtmCfg[0].deadTime.falling = 1e-6f; invDtmCfg[0].fastShutOff = NULL_PTR;
    invDtmCfg[1].deadTime.rising = 1e-6f; invDtmCfg[1].deadTime.falling = 1e-6f; invDtmCfg[1].fastShutOff = NULL_PTR;
    invDtmCfg[2].deadTime.rising = 1e-6f; invDtmCfg[2].deadTime.falling = 1e-6f; invDtmCfg[2].fastShutOff = NULL_PTR;

    /* 4) Prepare InterruptConfig: pulse notification, CPU0, configured priority, VMID 0, period callback */
    invIrqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    invIrqCfg.isrProvider = IfxSrc_Tos_cpu0;
    invIrqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    invIrqCfg.vmId        = IfxSrc_VmId_0;
    invIrqCfg.periodEvent = IfxEgtm_periodEventFunction;
    invIrqCfg.dutyEvent   = NULL_PTR;

    /* Channel configuration for inverter: ATOM0 logical channels 0..2, center-aligned, no phase shift */
    invChCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    invChCfg[0].phase     = 0.0f;
    invChCfg[0].duty      = PHASE_U_DUTY_INIT;
    invChCfg[0].dtm       = &invDtmCfg[0];
    invChCfg[0].output    = &invOutCfg[0];
    invChCfg[0].mscOut    = NULL_PTR;
    invChCfg[0].interrupt = &invIrqCfg; /* base channel interrupt only */

    invChCfg[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    invChCfg[1].phase     = 0.0f;
    invChCfg[1].duty      = PHASE_V_DUTY_INIT;
    invChCfg[1].dtm       = &invDtmCfg[1];
    invChCfg[1].output    = &invOutCfg[1];
    invChCfg[1].mscOut    = NULL_PTR;
    invChCfg[1].interrupt = NULL_PTR;

    invChCfg[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    invChCfg[2].phase     = 0.0f;
    invChCfg[2].duty      = PHASE_W_DUTY_INIT;
    invChCfg[2].dtm       = &invDtmCfg[2];
    invChCfg[2].output    = &invOutCfg[2];
    invChCfg[2].mscOut    = NULL_PTR;
    invChCfg[2].interrupt = NULL_PTR;

    /* Main inverter config */
    invCfg.cluster             = IfxEgtm_Cluster_0;
    invCfg.subModule           = IfxEgtm_Pwm_SubModule_atom;
    invCfg.alignment           = IfxEgtm_Pwm_Alignment_center;
    invCfg.numChannels         = (uint8)NUM_OF_CHANNELS;
    invCfg.channels            = invChCfg;
    invCfg.frequency           = PWM_SWITCH_FREQ_HZ;
    invCfg.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;      /* ATOM clock source */
    invCfg.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;
    invCfg.syncUpdateEnabled   = TRUE;
    invCfg.syncStart           = TRUE;

    /* 5) Configure ADC trigger channel: edge-aligned, 50% duty, non-complementary */
    adcOutCfg[0].pin                   = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_PIN;
    adcOutCfg[0].complementaryPin      = NULL_PTR;
    adcOutCfg[0].polarity              = Ifx_ActiveState_high;
    adcOutCfg[0].complementaryPolarity = Ifx_ActiveState_low;
    adcOutCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    adcOutCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    adcChCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;          /* dedicated ADC trigger channel */
    adcChCfg[0].phase     = 0.0f;
    adcChCfg[0].duty      = 50.0f;                                /* coherent with inverter timing */
    adcChCfg[0].dtm       = NULL_PTR;
    adcChCfg[0].output    = &adcOutCfg[0];
    adcChCfg[0].mscOut    = NULL_PTR;
    adcChCfg[0].interrupt = NULL_PTR;

    adcCfg.cluster             = IfxEgtm_Cluster_0;
    adcCfg.subModule           = IfxEgtm_Pwm_SubModule_atom;
    adcCfg.alignment           = IfxEgtm_Pwm_Alignment_edge;
    adcCfg.numChannels         = (uint8)NUM_OF_ADC_TRIG_CHANNELS;
    adcCfg.channels            = adcChCfg;
    adcCfg.frequency           = PWM_SWITCH_FREQ_HZ;
    adcCfg.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;
    adcCfg.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;
    adcCfg.syncUpdateEnabled   = TRUE;
    adcCfg.syncStart           = TRUE;

    /* 6) EGTM enable guard and CMU clock setup (MANDATORY pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Initialize PWM drivers: inverter first, then ADC trigger */
    IfxEgtm_Pwm_init(&g_egtmAtom3phState.pwm, g_egtmAtom3phState.channels, &invCfg);
    IfxEgtm_Pwm_init(&g_egtmAdcTrigState.pwm,  g_egtmAdcTrigState.channels,  &adcCfg);

    /* 8) Route EGTM trigger to ADC fabric (signal/channel per requirements) */
    {
        /* AdcTriggerSignal_0 from ATOM0 CH3 on Cluster 0. Mux/edge selection per SoC routing; falling edge as required. */
        (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                        IfxEgtm_TrigSource_atom0,
                                        (IfxEgtm_TrigChannel)IfxEgtm_Pwm_SubModule_Ch_3,
                                        IfxEgtm_Cfg_AdcTriggerSignal_0);
    }

    /* 9) Store persistent module state for runtime updates */
    g_egtmAtom3phState.dutyCycles[0] = invChCfg[0].duty;
    g_egtmAtom3phState.dutyCycles[1] = invChCfg[1].duty;
    g_egtmAtom3phState.dutyCycles[2] = invChCfg[2].duty;

    g_egtmAtom3phState.phases[0] = invChCfg[0].phase;
    g_egtmAtom3phState.phases[1] = invChCfg[1].phase;
    g_egtmAtom3phState.phases[2] = invChCfg[2].phase;

    g_egtmAtom3phState.deadTimes[0] = invDtmCfg[0].deadTime;
    g_egtmAtom3phState.deadTimes[1] = invDtmCfg[1].deadTime;
    g_egtmAtom3phState.deadTimes[2] = invDtmCfg[2].deadTime;

    g_egtmAdcTrigState.dutyCycle = adcChCfg[0].duty;

    /* 10) Configure debug I/O pin after PWM init */
    IfxPort_setPinModeOutput(EGTM_DEBUG_LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =============================
 * Runtime Update
 * ============================= */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    if (requestDuty == NULL_PTR)
    {
        return;
    }

    /* Clamp to [0, 100] and copy to persistent state */
    float32 d0 = requestDuty[0];
    float32 d1 = requestDuty[1];
    float32 d2 = requestDuty[2];

    if (d0 < 0.0f) { d0 = 0.0f; } else if (d0 > 100.0f) { d0 = 100.0f; }
    if (d1 < 0.0f) { d1 = 0.0f; } else if (d1 > 100.0f) { d1 = 100.0f; }
    if (d2 < 0.0f) { d2 = 0.0f; } else if (d2 > 100.0f) { d2 = 100.0f; }

    g_egtmAtom3phState.dutyCycles[0] = d0;
    g_egtmAtom3phState.dutyCycles[1] = d1;
    g_egtmAtom3phState.dutyCycles[2] = d2;

    /* Immediate synchronous multi-channel update */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phState.pwm, (float32 *)g_egtmAtom3phState.dutyCycles);
}
