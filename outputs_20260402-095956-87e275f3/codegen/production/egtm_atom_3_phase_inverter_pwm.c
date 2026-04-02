/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver for EGTM ATOM 3-phase complementary PWM + ADC trigger (TC4xx)
 *
 * Notes:
 * - This module does NOT disable watchdogs (per architecture rule).
 * - Uses unified IfxEgtm_Pwm high-level driver with ATOM submodule.
 * - Interrupt installation (vector table) is outside this module.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxAdc.h"
#include "IfxPort.h"
#include "IfxEgtm_PinMap.h"

/* ===================== Numeric configuration macros (from requirements) ===================== */
#define EGTM_INV_NUM_CHANNELS         (3U)
#define EGTM_ADC_NUM_CHANNELS         (1U)

#define EGTM_PWM_FREQUENCY_HZ         (20000.0f)   /* PWM switching frequency */

#define PHASE_U_DUTY_PCT              (25.0f)
#define PHASE_V_DUTY_PCT              (50.0f)
#define PHASE_W_DUTY_PCT              (75.0f)
#define PHASE_DUTY_STEP_PCT           (0.01f)

/* Interrupt priority for EGTM ATOM PWM period event */
#define ISR_PRIORITY_ATOM             (100U)

/* Debug ISR toggle pin: P03.9 */
#define LED                           (&MODULE_P03), 9

/* ===================== User-requested EGTM ATOM pin assignments ===================== */
/* 3-phase complementary PWM on EGTM Cluster 0 using ATOM0 CH0..2 */
#define PHASE_U_HS                    (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                    (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                    (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                    (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                    (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                    (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ADC trigger output on ATOM0 CH3 edge-aligned 50% (P33.0) */
#define ADC_TRIG_TOUT                 (&IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT)

/* ===================== EGTM Trigger routing selection (cast literals to enum types) ===================== */
/* Route ATOM0 CH3 to TMADC trigger fabric: AdcTriggerSignal_0, TriggerMuxSel_36, falling edge (edge/mux selection realized in SoC routing; API here selects signal/channel) */
#define EGTM_TRIG_SOURCE_ATOM0        ((IfxEgtm_TrigSource)0)
#define EGTM_TRIG_CHANNEL_3           ((IfxEgtm_TrigChannel)3)
#define EGTM_ADC_TRIGGER_SIGNAL_0     ((IfxEgtm_Cfg_AdcTriggerSignal)0)

/* ===================== Module persistent state ===================== */
typedef struct
{
    IfxEgtm_Pwm                pwmInverter;                        /* unified PWM handle: 3-phase inverter */
    IfxEgtm_Pwm_Channel        channelsInverter[EGTM_INV_NUM_CHANNELS];
    float32                    dutyCycles[EGTM_INV_NUM_CHANNELS];
    float32                    phases[EGTM_INV_NUM_CHANNELS];
    IfxEgtm_Pwm_DeadTime       deadTimes[EGTM_INV_NUM_CHANNELS];

    IfxEgtm_Pwm                pwmAdcTrig;                         /* unified PWM handle: single-channel ADC trigger */
    IfxEgtm_Pwm_Channel        channelsAdcTrig[EGTM_ADC_NUM_CHANNELS];

    Ifx_ADC_TMADC             *tmadc;                              /* optional reference for resultISR (set by app if needed) */
} EgtmAtom3ph_State;

IFX_STATIC EgtmAtom3ph_State g_egtmAtom3phState = {0};

/* ===================== ISR and callback declarations (must appear before init) ===================== */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty per design */
}

void resultISR(void)
{
    /* Minimal body: toggle debug pin; optionally clear TMADC result flags if available */
    IfxPort_togglePin(LED);

    if (g_egtmAtom3phState.tmadc != NULL_PTR)
    {
        uint8 ch;
        for (ch = 0U; ch < 5U; ++ch)
        {
            IfxAdc_TmadcResultReg rr = (IfxAdc_TmadcResultReg)ch;
            if (IfxAdc_isTmadcResultAvailable(g_egtmAtom3phState.tmadc, rr) != (boolean)0)
            {
                IfxAdc_clearTmadcResultFlag(g_egtmAtom3phState.tmadc, rr);
            }
        }
    }
}

/* ===================== Initialization ===================== */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config           invCfg;
    IfxEgtm_Pwm_ChannelConfig    invChCfg[EGTM_INV_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     invOut[EGTM_INV_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        invDtm[EGTM_INV_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    IfxEgtm_Pwm_Config           adcCfg;
    IfxEgtm_Pwm_ChannelConfig    adcChCfg[EGTM_ADC_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     adcOut[EGTM_ADC_NUM_CHANNELS];

    /* 2) Initialize both config structs with defaults */
    IfxEgtm_Pwm_initConfig(&invCfg, &MODULE_EGTM);
    IfxEgtm_Pwm_initConfig(&adcCfg, &MODULE_EGTM);

    /* 3) Configure 3-phase inverter (ATOM, center-aligned, sync update) */
    /* Output configuration: complementary HS/LS per phase, active-high HS, active-low LS */
    invOut[0].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    invOut[0].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    invOut[0].polarity                = Ifx_ActiveState_high;
    invOut[0].complementaryPolarity   = Ifx_ActiveState_low;
    invOut[0].outputMode              = IfxPort_OutputMode_pushPull;
    invOut[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    invOut[1].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    invOut[1].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    invOut[1].polarity                = Ifx_ActiveState_high;
    invOut[1].complementaryPolarity   = Ifx_ActiveState_low;
    invOut[1].outputMode              = IfxPort_OutputMode_pushPull;
    invOut[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    invOut[2].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    invOut[2].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    invOut[2].polarity                = Ifx_ActiveState_high;
    invOut[2].complementaryPolarity   = Ifx_ActiveState_low;
    invOut[2].outputMode              = IfxPort_OutputMode_pushPull;
    invOut[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time: 1us for rising and falling (DTM) */
    invDtm[0].deadTime.rising         = 1e-6f;
    invDtm[0].deadTime.falling        = 1e-6f;
    invDtm[0].fastShutOff             = NULL_PTR;

    invDtm[1].deadTime.rising         = 1e-6f;
    invDtm[1].deadTime.falling        = 1e-6f;
    invDtm[1].fastShutOff             = NULL_PTR;

    invDtm[2].deadTime.rising         = 1e-6f;
    invDtm[2].deadTime.falling        = 1e-6f;
    invDtm[2].fastShutOff             = NULL_PTR;

    /* Interrupt configuration: pulse notification, CPU0, configured priority, VM0 */
    irqCfg.mode                       = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider                = IfxSrc_Tos_cpu0;
    irqCfg.priority                   = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId                       = IfxSrc_VmId_0;
    irqCfg.periodEvent                = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent                  = NULL_PTR;

    /* Channel configuration: logical indices 0..2, no phase shift */
    invChCfg[0].timerCh               = IfxEgtm_Pwm_SubModule_Ch_0;
    invChCfg[0].phase                 = 0.0f;
    invChCfg[0].duty                  = PHASE_U_DUTY_PCT;
    invChCfg[0].dtm                   = &invDtm[0];
    invChCfg[0].output                = &invOut[0];
    invChCfg[0].mscOut                = NULL_PTR;
    invChCfg[0].interrupt             = &irqCfg;     /* base channel gets interrupt */

    invChCfg[1].timerCh               = IfxEgtm_Pwm_SubModule_Ch_1;
    invChCfg[1].phase                 = 0.0f;
    invChCfg[1].duty                  = PHASE_V_DUTY_PCT;
    invChCfg[1].dtm                   = &invDtm[1];
    invChCfg[1].output                = &invOut[1];
    invChCfg[1].mscOut                = NULL_PTR;
    invChCfg[1].interrupt             = NULL_PTR;    /* no interrupt on this channel */

    invChCfg[2].timerCh               = IfxEgtm_Pwm_SubModule_Ch_2;
    invChCfg[2].phase                 = 0.0f;
    invChCfg[2].duty                  = PHASE_W_DUTY_PCT;
    invChCfg[2].dtm                   = &invDtm[2];
    invChCfg[2].output                = &invOut[2];
    invChCfg[2].mscOut                = NULL_PTR;
    invChCfg[2].interrupt             = NULL_PTR;    /* no interrupt on this channel */

    invCfg.cluster                    = IfxEgtm_Cluster_0;
    invCfg.subModule                  = IfxEgtm_Pwm_SubModule_atom;
    invCfg.alignment                  = IfxEgtm_Pwm_Alignment_center;
    invCfg.syncUpdateEnabled          = TRUE;
    invCfg.syncStart                  = TRUE;
    invCfg.numChannels                = (uint8)EGTM_INV_NUM_CHANNELS;
    invCfg.channels                   = invChCfg;
    invCfg.frequency                  = EGTM_PWM_FREQUENCY_HZ;
    invCfg.clockSource.atom           = (uint32)IfxEgtm_Cmu_Clk_0;  /* ATOM clock source */
    invCfg.dtmClockSource             = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* 5) Configure ADC trigger: single channel, edge-aligned, 50% duty, non-complementary */
    adcOut[0].pin                     = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_TOUT;
    adcOut[0].complementaryPin        = NULL_PTR;
    adcOut[0].polarity                = Ifx_ActiveState_high;
    adcOut[0].complementaryPolarity   = Ifx_ActiveState_low;
    adcOut[0].outputMode              = IfxPort_OutputMode_pushPull;
    adcOut[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    adcChCfg[0].timerCh               = IfxEgtm_Pwm_SubModule_Ch_0;  /* logical index */
    adcChCfg[0].phase                 = 0.0f;
    adcChCfg[0].duty                  = 50.0f;                      /* 50% */
    adcChCfg[0].dtm                   = NULL_PTR;                   /* non-complementary */
    adcChCfg[0].output                = &adcOut[0];
    adcChCfg[0].mscOut                = NULL_PTR;
    adcChCfg[0].interrupt             = NULL_PTR;                   /* no interrupt */

    adcCfg.cluster                    = IfxEgtm_Cluster_0;
    adcCfg.subModule                  = IfxEgtm_Pwm_SubModule_atom;
    adcCfg.alignment                  = IfxEgtm_Pwm_Alignment_edge;
    adcCfg.syncUpdateEnabled          = TRUE;
    adcCfg.syncStart                  = TRUE;
    adcCfg.numChannels                = (uint8)EGTM_ADC_NUM_CHANNELS;
    adcCfg.channels                   = adcChCfg;
    adcCfg.frequency                  = EGTM_PWM_FREQUENCY_HZ;      /* coherent timing */
    adcCfg.clockSource.atom           = (uint32)IfxEgtm_Cmu_Clk_0;
    adcCfg.dtmClockSource             = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* 6) Enable EGTM + CMU clocks if not already enabled (MANDATORY enable guard pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        {
            float32 frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, frequency);
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 7) Initialize unified PWM drivers: inverter first, then ADC trigger */
    IfxEgtm_Pwm_init(&g_egtmAtom3phState.pwmInverter,
                     g_egtmAtom3phState.channelsInverter,
                     &invCfg);

    IfxEgtm_Pwm_init(&g_egtmAtom3phState.pwmAdcTrig,
                     g_egtmAtom3phState.channelsAdcTrig,
                     &adcCfg);

    /* 8) Route ATOM0 CH3 trigger to ADC fabric */
    {
        boolean routed = IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                                   EGTM_TRIG_SOURCE_ATOM0,
                                                   EGTM_TRIG_CHANNEL_3,
                                                   EGTM_ADC_TRIGGER_SIGNAL_0);
        (void)routed; /* optional: could be checked/logged by application */
    }

    /* 9) Store persistent module state: duties, phases, dead-times */
    g_egtmAtom3phState.dutyCycles[0]  = invChCfg[0].duty;
    g_egtmAtom3phState.dutyCycles[1]  = invChCfg[1].duty;
    g_egtmAtom3phState.dutyCycles[2]  = invChCfg[2].duty;

    g_egtmAtom3phState.phases[0]      = invChCfg[0].phase;
    g_egtmAtom3phState.phases[1]      = invChCfg[1].phase;
    g_egtmAtom3phState.phases[2]      = invChCfg[2].phase;

    g_egtmAtom3phState.deadTimes[0]   = invDtm[0].deadTime;
    g_egtmAtom3phState.deadTimes[1]   = invDtm[1].deadTime;
    g_egtmAtom3phState.deadTimes[2]   = invDtm[2].deadTime;

    /* 10) Configure debug I/O pin (ISR toggle) as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ===================== Runtime duty update ===================== */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Clamp inputs to [0, 100], copy to persistent state, then apply immediately (single API call) */
    float32 d0 = requestDuty[0];
    float32 d1 = requestDuty[1];
    float32 d2 = requestDuty[2];

    if (d0 < 0.0f)  { d0 = 0.0f; }
    if (d0 > 100.0f){ d0 = 100.0f; }
    if (d1 < 0.0f)  { d1 = 0.0f; }
    if (d1 > 100.0f){ d1 = 100.0f; }
    if (d2 < 0.0f)  { d2 = 0.0f; }
    if (d2 > 100.0f){ d2 = 100.0f; }

    g_egtmAtom3phState.dutyCycles[0] = d0;
    g_egtmAtom3phState.dutyCycles[1] = d1;
    g_egtmAtom3phState.dutyCycles[2] = d2;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phState.pwmInverter,
                                            (float32 *)g_egtmAtom3phState.dutyCycles);
}
