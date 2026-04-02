/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM ATOM 3-phase complementary PWM and ADC trigger (TC4xx).
 *
 * Notes:
 * - Watchdog disable MUST NOT be placed here; only in CpuX_Main.c (IfxWtu_* APIs).
 * - No STM timing here; scheduling belongs in application (Cpu0_Main.c).
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc.h"
#include "IfxPort.h"

/* ========================= Macros (configuration constants) ========================= */
#define NUM_OF_CHANNELS              (3U)
#define PWM_FREQUENCY                (20000.0f)
#define ISR_PRIORITY_ATOM            (100)

/* Complementary pair pin mapping (user-validated) */
#define PHASE_U_HS                   (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                   (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                   (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                   (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                   (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ADC trigger pin (single channel output) */
/* User: ATOM0_CH3 on P33.0 */
#define ADC_TRIG_PIN                 (&IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT)

/* Initial duties (percent) */
#define PHASE_U_DUTY                 (25.0f)
#define PHASE_V_DUTY                 (50.0f)
#define PHASE_W_DUTY                 (75.0f)
#define PHASE_DUTY_STEP              (0.01f)

/* LED/debug pin for ISR toggle: P03.9 (compound macro expands to 2 args) */
#define LED                          (&MODULE_P03), (9U)

/* ========================= Module state ========================= */
typedef struct
{
    IfxEgtm_Pwm                pwm;                     /* Inverter PWM driver handle */
    IfxEgtm_Pwm_Channel        channels[NUM_OF_CHANNELS];
    float32                    dutyCycles[NUM_OF_CHANNELS];
    float32                    phases[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DeadTime       deadTimes[NUM_OF_CHANNELS];

    /* Dedicated ADC trigger PWM (single channel) */
    IfxEgtm_Pwm                pwmAdcTrig;
    IfxEgtm_Pwm_Channel        chAdcTrig[1];
    float32                    trigDuty;
} EgtmAtom3PhState;

IFX_STATIC EgtmAtom3PhState g_egtmAtom3phState;

/* ========================= ISR and callback ========================= */
/**
 * EGTM PWM period ISR: minimal body, toggle debug pin only.
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/**
 * Unified EGTM PWM driver period-event callback (does nothing).
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/**
 * TMADC result ISR: minimal body, toggle debug pin only.
 * Optional per-channel result handling should be done in application if needed.
 */
void resultISR(void)
{
    IfxPort_togglePin(LED);

    /* Optional example (disabled here): check/clear TMADC results CH0..CH4
       Requires a valid TMADC module pointer from system integration.
       for (IfxAdc_TmadcResultReg r = (IfxAdc_TmadcResultReg)0; r <= (IfxAdc_TmadcResultReg)4; r++)
       {
           if (IfxAdc_isTmadcResultAvailable(&MODULE_TMADC0, r))
           {
               IfxAdc_clearTmadcResultFlag(&MODULE_TMADC0, r);
           }
       }
    */
}

/* ========================= Public API ========================= */
/**
 * Initialize EGTM PWM for a 3-phase complementary inverter (ATOM0 CH0..2, center-aligned, 20 kHz, 1 us DT)
 * and a separate single-channel PWM used as ADC trigger (ATOM0 CH3, edge-aligned 50%).
 * - Configures EGTM clocks on first use
 * - Applies routing for all outputs from configured TOUT maps
 * - Installs unified driver interrupt callback and uses external ISR function for toggling debug pin
 * - Routes the ADC trigger via IfxEgtm_Trigger_trigToAdc
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config            invCfg;
    IfxEgtm_Pwm_ChannelConfig     invChCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      invOut[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         invDtm[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg;

    IfxEgtm_Pwm_Config            trigCfg;
    IfxEgtm_Pwm_ChannelConfig     trigChCfg[1];
    IfxEgtm_Pwm_OutputConfig      trigOut[1];

    /* 2) Initialize both config structs with defaults */
    IfxEgtm_Pwm_initConfig(&invCfg, &MODULE_EGTM);
    IfxEgtm_Pwm_initConfig(&trigCfg, &MODULE_EGTM);

    /* 3) Configure 3-phase inverter: ATOM, center-aligned, sync update, 20 kHz, CMU CLK0, DTM via CMU CLK0 */
    /* Output configuration for complementary pairs */
    invOut[0].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    invOut[0].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    invOut[0].polarity                = Ifx_ActiveState_high;   /* HS active HIGH */
    invOut[0].complementaryPolarity   = Ifx_ActiveState_low;    /* LS active LOW  */
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

    /* Dead-time configuration: 1 us both edges for all phases */
    invDtm[0].deadTime.rising = 1e-6f; invDtm[0].deadTime.falling = 1e-6f; invDtm[0].fastShutOff = NULL_PTR;
    invDtm[1].deadTime.rising = 1e-6f; invDtm[1].deadTime.falling = 1e-6f; invDtm[1].fastShutOff = NULL_PTR;
    invDtm[2].deadTime.rising = 1e-6f; invDtm[2].deadTime.falling = 1e-6f; invDtm[2].fastShutOff = NULL_PTR;

    /* Interrupt configuration: pulse notification on period, CPU0, priority macro, VM0 */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* Channel configuration: contiguous logical indices 0..2, center-aligned (phase = 0) */
    invChCfg[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    invChCfg[0].phase      = 0.0f;
    invChCfg[0].duty       = PHASE_U_DUTY;
    invChCfg[0].dtm        = &invDtm[0];
    invChCfg[0].output     = &invOut[0];
    invChCfg[0].mscOut     = NULL_PTR;
    invChCfg[0].interrupt  = &irqCfg;           /* base channel gets interrupt */

    invChCfg[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    invChCfg[1].phase      = 0.0f;
    invChCfg[1].duty       = PHASE_V_DUTY;
    invChCfg[1].dtm        = &invDtm[1];
    invChCfg[1].output     = &invOut[1];
    invChCfg[1].mscOut     = NULL_PTR;
    invChCfg[1].interrupt  = NULL_PTR;          /* no direct interrupt on follower channels */

    invChCfg[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    invChCfg[2].phase      = 0.0f;
    invChCfg[2].duty       = PHASE_W_DUTY;
    invChCfg[2].dtm        = &invDtm[2];
    invChCfg[2].output     = &invOut[2];
    invChCfg[2].mscOut     = NULL_PTR;
    invChCfg[2].interrupt  = NULL_PTR;

    /* Main inverter configuration */
    invCfg.cluster             = IfxEgtm_Cluster_0;
    invCfg.subModule           = IfxEgtm_Pwm_SubModule_atom;
    invCfg.alignment           = IfxEgtm_Pwm_Alignment_center;
    invCfg.numChannels         = (uint8)NUM_OF_CHANNELS;
    invCfg.channels            = invChCfg;
    invCfg.frequency           = PWM_FREQUENCY;
    invCfg.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;          /* ATOM clock source */
    invCfg.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;  /* DTM uses CMU CLK0 */
    invCfg.syncUpdateEnabled   = TRUE;
    invCfg.syncStart           = TRUE;

    /* 5) Configure ADC trigger: single channel, edge-aligned 50% duty, non-complementary */
    trigOut[0].pin                    = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_PIN;
    trigOut[0].complementaryPin       = NULL_PTR;
    trigOut[0].polarity               = Ifx_ActiveState_high;
    trigOut[0].complementaryPolarity  = Ifx_ActiveState_low;
    trigOut[0].outputMode             = IfxPort_OutputMode_pushPull;
    trigOut[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    trigChCfg[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_3;
    trigChCfg[0].phase      = 0.0f;              /* edge-aligned */
    trigChCfg[0].duty       = 50.0f;             /* 50% */
    trigChCfg[0].dtm        = NULL_PTR;          /* non-complementary */
    trigChCfg[0].output     = &trigOut[0];
    trigChCfg[0].mscOut     = NULL_PTR;
    trigChCfg[0].interrupt  = NULL_PTR;

    trigCfg.cluster             = IfxEgtm_Cluster_0;
    trigCfg.subModule           = IfxEgtm_Pwm_SubModule_atom;
    trigCfg.alignment           = IfxEgtm_Pwm_Alignment_edge;
    trigCfg.numChannels         = 1U;
    trigCfg.channels            = trigChCfg;
    trigCfg.frequency           = PWM_FREQUENCY;          /* coherent with inverter timing */
    trigCfg.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;
    trigCfg.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;
    trigCfg.syncUpdateEnabled   = TRUE;
    trigCfg.syncStart           = TRUE;

    /* 6) EGTM enable guard and CMU clock setup (MANDATORY pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Dynamically set GCLK and CLK0 to module frequency, then enable clocks */
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 7) Initialize PWM drivers: inverter first, then ADC trigger */
    IfxEgtm_Pwm_init(&g_egtmAtom3phState.pwm, g_egtmAtom3phState.channels, &invCfg);
    IfxEgtm_Pwm_init(&g_egtmAtom3phState.pwmAdcTrig, g_egtmAtom3phState.chAdcTrig, &trigCfg);

    /* 8) Route ADC trigger from ATOM0 CH3 to TMADC (ADC Trigger fabric) */
    {
        /* Use Cluster 0, source ATOM0, channel 3, signal index 0 as requested */
        boolean routed = IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                                  (IfxEgtm_TrigSource)0,   /* ATOM0 source */
                                                  (IfxEgtm_TrigChannel)3,  /* channel 3 */
                                                  (IfxEgtm_Cfg_AdcTriggerSignal)0 /* AdcTriggerSignal_0 */);
        (void)routed; /* Optionally check routing success in application */
    }

    /* 9) Store persistent module state (duties/phases/deadTimes) */
    g_egtmAtom3phState.dutyCycles[0] = invChCfg[0].duty;
    g_egtmAtom3phState.dutyCycles[1] = invChCfg[1].duty;
    g_egtmAtom3phState.dutyCycles[2] = invChCfg[2].duty;

    g_egtmAtom3phState.phases[0] = invChCfg[0].phase;
    g_egtmAtom3phState.phases[1] = invChCfg[1].phase;
    g_egtmAtom3phState.phases[2] = invChCfg[2].phase;

    g_egtmAtom3phState.deadTimes[0] = invDtm[0].deadTime;
    g_egtmAtom3phState.deadTimes[1] = invDtm[1].deadTime;
    g_egtmAtom3phState.deadTimes[2] = invDtm[2].deadTime;

    g_egtmAtom3phState.trigDuty = trigChCfg[0].duty;

    /* 10) Configure the debug I/O pin used by ISR */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three inverter phase duties immediately and synchronously.
 * requestDuty[0..2] in percent; clamp to [0,100] and apply via unified driver.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    float32 u = requestDuty[0];
    float32 v = requestDuty[1];
    float32 w = requestDuty[2];

    if (u < 0.0f) { u = 0.0f; } else if (u > 100.0f) { u = 100.0f; }
    if (v < 0.0f) { v = 0.0f; } else if (v > 100.0f) { v = 100.0f; }
    if (w < 0.0f) { w = 0.0f; } else if (w > 100.0f) { w = 100.0f; }

    g_egtmAtom3phState.dutyCycles[0] = u;
    g_egtmAtom3phState.dutyCycles[1] = v;
    g_egtmAtom3phState.dutyCycles[2] = w;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phState.pwm, (float32 *)g_egtmAtom3phState.dutyCycles);
}
