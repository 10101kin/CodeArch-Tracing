/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver implementing EGTM ATOM 3-phase complementary PWM and ADC trigger (TC4xx).
 *
 * Follows iLLD initialization patterns and migration guidelines:
 *  - TC3xx -> TC4xx: IfxGtm_* -> IfxEgtm_*
 *  - Unified PWM driver: IfxEgtm_Pwm
 *  - CMU enable guard and dynamic frequency setup
 *  - Interrupt routing via unified driver with minimal ISR
 *
 * Restrictions:
 *  - Do not place any watchdog disable here (only in CpuX main files).
 *  - No STM timing in this driver.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm.h"
#include "IfxAdc.h"          /* For IfxAdc_isTmadcResultAvailable / IfxAdc_clearTmadcResultFlag */
#include "IfxAdc_Tmadc.h"    /* For Ifx_ADC_TMADC / IfxAdc_TmadcResultReg types                 */
#include "IfxPort.h"

/*=============================
 * Numeric configuration macros
 *=============================*/
#define EGTM_INV_NUM_CHANNELS            (3U)
#define EGTM_ADC_NUM_CHANNELS            (1U)
#define EGTM_PWM_FREQUENCY_HZ            (20000.0f)      /* PWM switching frequency */
#define EGTM_PHASE_U_INIT_DUTY           (25.0f)
#define EGTM_PHASE_V_INIT_DUTY           (50.0f)
#define EGTM_PHASE_W_INIT_DUTY           (75.0f)
#define EGTM_PHASE_DUTY_STEP             (0.01f)
#define EGTM_ADC_TRIG_INIT_DUTY          (50.0f)
#define ISR_PRIORITY_ATOM                (100)           /* From INTERRUPT_PRIORITY_EVADC_CUR */

/*=============================
 * Pin macros
 *=============================*/
/* Debug ISR toggle pin: P03.9 */
#define LED &MODULE_P03, 9

/* User-requested EGTM ATOM output pins */
#define PHASE_U_HS   &IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT
#define PHASE_U_LS   &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT
#define PHASE_V_HS   &IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT
#define PHASE_V_LS   &IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT
#define PHASE_W_HS   &IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT
#define PHASE_W_LS   &IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT
#define ADC_TRIG_OUT &IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT

/*=============================
 * Module state
 *=============================*/
typedef struct
{
    IfxEgtm_Pwm               pwmInv;                              /* Inverter PWM handle */
    IfxEgtm_Pwm_Channel       invChannels[EGTM_INV_NUM_CHANNELS];  /* Persistent channels */
    float32                   dutyCycles[EGTM_INV_NUM_CHANNELS];   /* Duty in percent */
    float32                   phases[EGTM_INV_NUM_CHANNELS];       /* Phase offsets (percent or 0) */
    IfxEgtm_Pwm_DeadTime      deadTimes[EGTM_INV_NUM_CHANNELS];    /* Dead-time settings */

    IfxEgtm_Pwm               pwmAdc;                              /* ADC trigger PWM handle */
    IfxEgtm_Pwm_Channel       adcChannels[EGTM_ADC_NUM_CHANNELS];  /* Persistent ADC trigger channel */
    float32                   adcDuty;                             /* ADC trigger duty percent */
} EgtmAtom3ph_State;

IFX_STATIC EgtmAtom3ph_State g_egtmAtom3ph = {0};

/*=============================
 * ISR and callback declarations
 *=============================*/
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/* Doxygen: Unified EGTM PWM driver period event callback (no operation). */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* No operation; ISR handles observation */
}

/*=============================
 * ISR implementations
 *=============================*/
void interruptEgtmAtom(void)
{
    /* Minimal ISR: toggle debug pin for observation */
    IfxPort_togglePin(LED);
}

/* Optional TMADC result ISR: minimal work only */
void resultISR(void)
{
    IfxPort_togglePin(LED);

    /* Optional: check and clear TMADC result flags (placeholders; actual TMADC handle to be bound in application integration) */
    {
        Ifx_ADC_TMADC *tmadc = (Ifx_ADC_TMADC *)0; /* Replace with actual TMADC module pointer during system integration */
        int i;
        for (i = 0; i < 5; ++i)
        {
            IfxAdc_TmadcResultReg reg = (IfxAdc_TmadcResultReg)i;
            if (IfxAdc_isTmadcResultAvailable(tmadc, reg))
            {
                IfxAdc_clearTmadcResultFlag(tmadc, reg);
            }
        }
    }
}

/*=============================
 * Initialization
 *=============================*/
/**
 * Initialize unified EGTM PWM for a 3-phase complementary inverter (ATOM0 CH0..2) and a single-channel ADC trigger (ATOM0 CH3).
 *
 * Steps (per design):
 *  1) Declare all configuration structures locally (two separate IfxEgtm_Pwm_Config: inverter and ADC trigger)
 *  2) Initialize both with IfxEgtm_Pwm_initConfig
 *  3) Configure inverter: Cluster 0, ATOM submodule, center-aligned, sync update, 20 kHz, ATOM clock source = Clk_0, DTM clock source = cmuClock0; 3 channels with complementary outputs and 1 us dead-time
 *  4) Prepare InterruptConfig for base logical channel (pulse notify, cpu0, ISR_PRIORITY_ATOM, vmId 0), callback = IfxEgtm_periodEventFunction; other channels interrupt = NULL
 *  5) Configure ADC trigger: single channel, edge-aligned, 50% duty, non-complementary, frequency coherent with inverter
 *  6) EGTM enable guard with CMU clock setup
 *  7) Call IfxEgtm_Pwm_init for inverter then ADC trigger
 *  8) Route trigger to TMADC via IfxEgtm_Trigger_trigToAdc
 *  9) Store persistent state
 * 10) Configure debug I/O pin as push-pull output
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config            invCfg;
    IfxEgtm_Pwm_ChannelConfig     invChCfg[EGTM_INV_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      invOutCfg[EGTM_INV_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         invDtmCfg[EGTM_INV_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg;

    IfxEgtm_Pwm_Config            adcCfg;
    IfxEgtm_Pwm_ChannelConfig     adcChCfg[EGTM_ADC_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      adcOutCfg[EGTM_ADC_NUM_CHANNELS];

    /* 2) Initialize both config structs with defaults */
    IfxEgtm_Pwm_initConfig(&invCfg, &MODULE_EGTM);
    IfxEgtm_Pwm_initConfig(&adcCfg, &MODULE_EGTM);

    /* 3) Inverter configuration */
    invCfg.cluster              = IfxEgtm_Cluster_0;
    invCfg.subModule            = IfxEgtm_Pwm_SubModule_atom;
    invCfg.alignment            = IfxEgtm_Pwm_Alignment_center;
    invCfg.numChannels          = (uint8)EGTM_INV_NUM_CHANNELS;
    invCfg.channels             = &invChCfg[0];
    invCfg.frequency            = EGTM_PWM_FREQUENCY_HZ;
    invCfg.clockSource.atom     = (uint32)IfxEgtm_Cmu_Clk_0;          /* ATOM clock source */
    invCfg.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;  /* DTM clock source  */
    invCfg.syncUpdateEnabled    = TRUE;
    invCfg.syncStart            = TRUE;

    /* Output and DTM per-channel setup (complementary, 1 us dead-time both edges) */
    /* Channel 0: Phase U */
    invOutCfg[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    invOutCfg[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    invOutCfg[0].polarity               = Ifx_ActiveState_high;  /* High-side active HIGH */
    invOutCfg[0].complementaryPolarity  = Ifx_ActiveState_low;   /* Low-side  active LOW  */
    invOutCfg[0].outputMode             = IfxPort_OutputMode_pushPull;
    invOutCfg[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    invDtmCfg[0].deadTime.rising        = 1e-6f;
    invDtmCfg[0].deadTime.falling       = 1e-6f;
    invDtmCfg[0].fastShutOff            = NULL_PTR;

    /* Channel 1: Phase V */
    invOutCfg[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    invOutCfg[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    invOutCfg[1].polarity               = Ifx_ActiveState_high;
    invOutCfg[1].complementaryPolarity  = Ifx_ActiveState_low;
    invOutCfg[1].outputMode             = IfxPort_OutputMode_pushPull;
    invOutCfg[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    invDtmCfg[1].deadTime.rising        = 1e-6f;
    invDtmCfg[1].deadTime.falling       = 1e-6f;
    invDtmCfg[1].fastShutOff            = NULL_PTR;

    /* Channel 2: Phase W */
    invOutCfg[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    invOutCfg[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    invOutCfg[2].polarity               = Ifx_ActiveState_high;
    invOutCfg[2].complementaryPolarity  = Ifx_ActiveState_low;
    invOutCfg[2].outputMode             = IfxPort_OutputMode_pushPull;
    invOutCfg[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    invDtmCfg[2].deadTime.rising        = 1e-6f;
    invDtmCfg[2].deadTime.falling       = 1e-6f;
    invDtmCfg[2].fastShutOff            = NULL_PTR;

    /* Interrupt (base channel only) */
    irqCfg.mode         = (IfxEgtm_IrqMode)0;              /* pulse-notification mode */
    irqCfg.isrProvider  = IfxSrc_Tos_cpu0;                 /* CPU0 */
    irqCfg.priority     = (Ifx_Priority)ISR_PRIORITY_ATOM; /* Priority */
    irqCfg.vmId         = IfxSrc_VmId_0;                   /* VM0 */
    irqCfg.periodEvent  = (IfxEgtm_Pwm_callBack)IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent    = NULL_PTR;

    /* Logical channel configuration (indices 0..2, no phase shift) */
    invChCfg[0].timerCh     = IfxEgtm_Pwm_SubModule_Ch_0;
    invChCfg[0].phase       = 0.0f;
    invChCfg[0].duty        = EGTM_PHASE_U_INIT_DUTY;
    invChCfg[0].dtm         = &invDtmCfg[0];
    invChCfg[0].output      = &invOutCfg[0];
    invChCfg[0].mscOut      = NULL_PTR;
    invChCfg[0].interrupt   = &irqCfg;                     /* Base channel */

    invChCfg[1].timerCh     = IfxEgtm_Pwm_SubModule_Ch_1;
    invChCfg[1].phase       = 0.0f;
    invChCfg[1].duty        = EGTM_PHASE_V_INIT_DUTY;
    invChCfg[1].dtm         = &invDtmCfg[1];
    invChCfg[1].output      = &invOutCfg[1];
    invChCfg[1].mscOut      = NULL_PTR;
    invChCfg[1].interrupt   = NULL_PTR;                    /* No separate interrupt */

    invChCfg[2].timerCh     = IfxEgtm_Pwm_SubModule_Ch_2;
    invChCfg[2].phase       = 0.0f;
    invChCfg[2].duty        = EGTM_PHASE_W_INIT_DUTY;
    invChCfg[2].dtm         = &invDtmCfg[2];
    invChCfg[2].output      = &invOutCfg[2];
    invChCfg[2].mscOut      = NULL_PTR;
    invChCfg[2].interrupt   = NULL_PTR;                    /* No separate interrupt */

    /* 5) ADC trigger minimal configuration: single non-complementary, edge-aligned, 50% duty */
    adcCfg.cluster              = IfxEgtm_Cluster_0;
    adcCfg.subModule            = IfxEgtm_Pwm_SubModule_atom;
    adcCfg.alignment            = IfxEgtm_Pwm_Alignment_edge;
    adcCfg.numChannels          = (uint8)EGTM_ADC_NUM_CHANNELS;
    adcCfg.channels             = &adcChCfg[0];
    adcCfg.frequency            = EGTM_PWM_FREQUENCY_HZ;      /* Coherent timing */
    adcCfg.clockSource.atom     = (uint32)IfxEgtm_Cmu_Clk_0;  /* Same clock domain */
    adcCfg.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;
    adcCfg.syncUpdateEnabled    = TRUE;
    adcCfg.syncStart            = TRUE;

    adcOutCfg[0].pin                    = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_OUT;
    adcOutCfg[0].complementaryPin       = NULL_PTR;
    adcOutCfg[0].polarity               = Ifx_ActiveState_high;
    adcOutCfg[0].complementaryPolarity  = Ifx_ActiveState_low; /* Unused */
    adcOutCfg[0].outputMode             = IfxPort_OutputMode_pushPull;
    adcOutCfg[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    adcChCfg[0].timerCh     = IfxEgtm_Pwm_SubModule_Ch_0;  /* Logical index 0 */
    adcChCfg[0].phase       = 0.0f;
    adcChCfg[0].duty        = EGTM_ADC_TRIG_INIT_DUTY;
    adcChCfg[0].dtm         = NULL_PTR;                    /* No complementary DT */
    adcChCfg[0].output      = &adcOutCfg[0];
    adcChCfg[0].mscOut      = NULL_PTR;
    adcChCfg[0].interrupt   = NULL_PTR;                    /* No interrupt required */

    /* 6) EGTM enable guard with CMU clocks */
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

    /* 7) Initialize PWM drivers (inverter first, then ADC trigger) */
    IfxEgtm_Pwm_init(&g_egtmAtom3ph.pwmInv, &g_egtmAtom3ph.invChannels[0], &invCfg);
    IfxEgtm_Pwm_init(&g_egtmAtom3ph.pwmAdc, &g_egtmAtom3ph.adcChannels[0], &adcCfg);

    /* 8) Route ATOM trigger to TMADC via EGTM Trigger API
     *    Design selection: AdcTriggerSignal_0 on falling edge, Mux Sel 36. The API signature accepts (cluster, source, channel, signal).
     *    Here we select Cluster 0, ATOM source (0), channel 3 (ATOM0 CH3), signal 0. */
    {
        (void)IfxEgtm_Trigger_trigToAdc((IfxEgtm_Cluster)IfxEgtm_Cluster_0,
                                        (IfxEgtm_TrigSource)0,      /* ATOM source selector */
                                        (IfxEgtm_TrigChannel)3,     /* ATOM0 CH3 */
                                        (IfxEgtm_Cfg_AdcTriggerSignal)0); /* AdcTriggerSignal_0 */
    }

    /* 9) Store persistent state for runtime updates */
    g_egtmAtom3ph.dutyCycles[0] = EGTM_PHASE_U_INIT_DUTY;
    g_egtmAtom3ph.dutyCycles[1] = EGTM_PHASE_V_INIT_DUTY;
    g_egtmAtom3ph.dutyCycles[2] = EGTM_PHASE_W_INIT_DUTY;
    g_egtmAtom3ph.phases[0]     = 0.0f;
    g_egtmAtom3ph.phases[1]     = 0.0f;
    g_egtmAtom3ph.phases[2]     = 0.0f;
    g_egtmAtom3ph.deadTimes[0]  = invDtmCfg[0].deadTime;
    g_egtmAtom3ph.deadTimes[1]  = invDtmCfg[1].deadTime;
    g_egtmAtom3ph.deadTimes[2]  = invDtmCfg[2].deadTime;
    g_egtmAtom3ph.adcDuty       = EGTM_ADC_TRIG_INIT_DUTY;

    /* 10) Configure debug I/O pin used by ISR */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*=============================
 * Runtime update API
 *=============================*/
/**
 * Update the three inverter phase duties immediately and synchronously.
 * - requestDuty[0..2] are in percent 0..100
 * - Values are clamped, copied to persistent state, then applied via immediate multi-channel update.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    float32 d0 = requestDuty[0];
    float32 d1 = requestDuty[1];
    float32 d2 = requestDuty[2];

    /* Clamp to [0, 100] */
    if (d0 < 0.0f)  d0 = 0.0f;   else if (d0 > 100.0f) d0 = 100.0f;
    if (d1 < 0.0f)  d1 = 0.0f;   else if (d1 > 100.0f) d1 = 100.0f;
    if (d2 < 0.0f)  d2 = 0.0f;   else if (d2 > 100.0f) d2 = 100.0f;

    /* Copy to persistent state */
    g_egtmAtom3ph.dutyCycles[0] = d0;
    g_egtmAtom3ph.dutyCycles[1] = d1;
    g_egtmAtom3ph.dutyCycles[2] = d2;

    /* Immediate multi-channel update (percent values, no scaling) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3ph.pwmInv, (float32 *)g_egtmAtom3ph.dutyCycles);
}
