/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 *
 * Production driver for TC4xx EGTM-ATOM complementary PWM (3-phase inverter)
 * with TMADC trigger routing and minimal ISR handlers.
 *
 * Notes:
 * - Watchdog disable is NOT placed in this module (CPU file responsibility).
 * - No STM timing logic in this module (handled by CpuX_Main.c).
 * - Follows iLLD unified PWM initialization patterns with EGTM.
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxAdc.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =============================
 * User/migration constants
 * ============================= */
#define NUM_OF_CHANNELS                 (3u)
#define NUM_OF_ADC_TRIG_CHANNELS        (1u)
#define PWM_FREQUENCY_HZ                (20000.0f)
#define PHASE_U_DUTY_INIT               (25.0f)
#define PHASE_V_DUTY_INIT               (50.0f)
#define PHASE_W_DUTY_INIT               (75.0f)
#define ADC_TRIG_DUTY_INIT              (50.0f)
#define PHASE_DUTY_STEP_PERCENT         (0.01f)

/* ISR priority macro for PWM period-event ISR (project-specific) */
#define ISR_PRIORITY_ATOM               (10)

/* Heartbeat LED pin macro (compound: port, pin) */
#define LED                              &MODULE_P03, 9

/* =============================
 * Pin mapping macros (validated list enforced)
 * ============================= */
/* Phase U (validated) */
#define PHASE_U_HS                      (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                      (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)

/* Phase V (placeholders – replace during integration with validated symbols) */
#define PHASE_V_HS                      (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT */
#define PHASE_V_LS                      (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT */

/* Phase W (placeholders – replace during integration with validated symbols) */
#define PHASE_W_HS                      (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT */
#define PHASE_W_LS                      (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT */

/* ADC Trigger monitor pin (placeholder – replace during integration) */
#define ADC_TRIG_MON_PIN                (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT */

/* =============================
 * Module persistent state
 * ============================= */

typedef struct
{
    /* Inverter PWM instance */
    IfxEgtm_Pwm               pwmInv;
    IfxEgtm_Pwm_Channel       channelsInv[NUM_OF_CHANNELS];
    float32                   dutyCycles[NUM_OF_CHANNELS];
    float32                   phases[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];

    /* ADC trigger PWM instance (single logical channel) */
    IfxEgtm_Pwm               pwmTrig;
    IfxEgtm_Pwm_Channel       channelsTrig[NUM_OF_ADC_TRIG_CHANNELS];

    /* TMADC driver handle placeholder (startup/init handled elsewhere) */
    IfxAdc_Tmadc              tmadc;
} EgtmAtom3phState;

/* IFX_STATIC is defined by Compilers.h and used by iLLD */
IFX_STATIC EgtmAtom3phState g_egtmAtomState;

/* =============================
 * ISR and callback declarations
 * ============================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/*
 * Unified PWM period-event callback (do not perform processing here).
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* No processing per design */
}

/*
 * PWM period-event hardware ISR: toggle heartbeat GPIO and return.
 */
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* =============================
 * Public API implementations
 * ============================= */

/*
 * Initialize EGTM ATOM for 3-phase complementary PWM and a single ADC trigger channel.
 *
 * - Configures 3 center-aligned complementary channels (U/V/W) on ATOM0 CH0..CH2 at 20 kHz
 *   with 1us rising/falling dead-time and initial duties 25/50/75 percent.
 * - Configures one edge-aligned 50% duty ATOM0 CH3 for ADC trigger/monitor.
 * - Routes EGTM trigger to TMADC using IfxEgtm_Trigger_trigToAdc.
 * - Enables ADC module and runs TMADC engine.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config            invConfig;
    IfxEgtm_Pwm_ChannelConfig     invChCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         invDtmCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      invOutCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   invIrqCfg;

    IfxEgtm_Pwm_Config            trigConfig;
    IfxEgtm_Pwm_ChannelConfig     trigChCfg[NUM_OF_ADC_TRIG_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      trigOutCfg[NUM_OF_ADC_TRIG_CHANNELS];

    /* 2) Initialize unified PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&invConfig, &MODULE_EGTM);

    /* 3) Output configuration for inverter complementary pairs (U/V/W) */
    /* Phase U */
    invOutCfg[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    invOutCfg[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    invOutCfg[0].polarity              = Ifx_ActiveState_high; /* HS active HIGH */
    invOutCfg[0].complementaryPolarity = Ifx_ActiveState_low;  /* LS active LOW  */
    invOutCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    invOutCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    invOutCfg[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    invOutCfg[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    invOutCfg[1].polarity              = Ifx_ActiveState_high;
    invOutCfg[1].complementaryPolarity = Ifx_ActiveState_low;
    invOutCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
    invOutCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    invOutCfg[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    invOutCfg[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    invOutCfg[2].polarity              = Ifx_ActiveState_high;
    invOutCfg[2].complementaryPolarity = Ifx_ActiveState_low;
    invOutCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
    invOutCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (1 us rising/falling) */
    invDtmCfg[0].deadTime.rising  = 1.0e-6f;
    invDtmCfg[0].deadTime.falling = 1.0e-6f;
    invDtmCfg[0].fastShutOff      = NULL_PTR;

    invDtmCfg[1].deadTime.rising  = 1.0e-6f;
    invDtmCfg[1].deadTime.falling = 1.0e-6f;
    invDtmCfg[1].fastShutOff      = NULL_PTR;

    invDtmCfg[2].deadTime.rising  = 1.0e-6f;
    invDtmCfg[2].deadTime.falling = 1.0e-6f;
    invDtmCfg[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration (period-event on channel 0) */
    invIrqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    invIrqCfg.isrProvider = IfxSrc_Tos_cpu0;
    invIrqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    invIrqCfg.vmId        = IfxSrc_VmId_0;
    invIrqCfg.periodEvent = IfxEgtm_periodEventFunction;
    invIrqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2) */
    /* CH0 -> Phase U */
    invChCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    invChCfg[0].phase     = 0.0f;
    invChCfg[0].duty      = PHASE_U_DUTY_INIT;
    invChCfg[0].dtm       = &invDtmCfg[0];
    invChCfg[0].output    = &invOutCfg[0];
    invChCfg[0].mscOut    = NULL_PTR;
    invChCfg[0].interrupt = &invIrqCfg; /* only channel 0 carries interrupt */

    /* CH1 -> Phase V */
    invChCfg[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    invChCfg[1].phase     = 0.0f;
    invChCfg[1].duty      = PHASE_V_DUTY_INIT;
    invChCfg[1].dtm       = &invDtmCfg[1];
    invChCfg[1].output    = &invOutCfg[1];
    invChCfg[1].mscOut    = NULL_PTR;
    invChCfg[1].interrupt = NULL_PTR;

    /* CH2 -> Phase W */
    invChCfg[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    invChCfg[2].phase     = 0.0f;
    invChCfg[2].duty      = PHASE_W_DUTY_INIT;
    invChCfg[2].dtm       = &invDtmCfg[2];
    invChCfg[2].output    = &invOutCfg[2];
    invChCfg[2].mscOut    = NULL_PTR;
    invChCfg[2].interrupt = NULL_PTR;

    /* 7) Main inverter config */
    invConfig.cluster             = IfxEgtm_Cluster_0;
    invConfig.subModule           = IfxEgtm_Pwm_SubModule_atom;
    invConfig.alignment           = IfxEgtm_Pwm_Alignment_center;
    invConfig.numChannels         = (uint8)NUM_OF_CHANNELS;
    invConfig.channels            = invChCfg;
    invConfig.frequency           = PWM_FREQUENCY_HZ;
    invConfig.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0; /* ATOM clock source */
    invConfig.dtmClockSource      = IfxEgtm_Dtm_ClockSource_systemClock;
    invConfig.syncUpdateEnabled   = TRUE;
    invConfig.syncStart           = TRUE;

    /* 8) Enable-guard and CMU setup (MANDATORY pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize inverter PWM (applies duties and dead-times) */
    IfxEgtm_Pwm_init(&g_egtmAtomState.pwmInv, g_egtmAtomState.channelsInv, &invConfig);

    /* 10) Persist initial state for later updates */
    g_egtmAtomState.dutyCycles[0] = invChCfg[0].duty;
    g_egtmAtomState.dutyCycles[1] = invChCfg[1].duty;
    g_egtmAtomState.dutyCycles[2] = invChCfg[2].duty;

    g_egtmAtomState.phases[0]     = invChCfg[0].phase;
    g_egtmAtomState.phases[1]     = invChCfg[1].phase;
    g_egtmAtomState.phases[2]     = invChCfg[2].phase;

    g_egtmAtomState.deadTimes[0]  = invDtmCfg[0].deadTime;
    g_egtmAtomState.deadTimes[1]  = invDtmCfg[1].deadTime;
    g_egtmAtomState.deadTimes[2]  = invDtmCfg[2].deadTime;

    /* 11) Configure the single ADC-trigger PWM instance (ATOM0 CH3, edge-aligned 50%) */
    IfxEgtm_Pwm_initConfig(&trigConfig, &MODULE_EGTM);

    trigOutCfg[0].pin                   = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_MON_PIN; /* monitor pin */
    trigOutCfg[0].complementaryPin      = NULL_PTR;
    trigOutCfg[0].polarity              = Ifx_ActiveState_high;
    trigOutCfg[0].complementaryPolarity = Ifx_ActiveState_low;
    trigOutCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    trigOutCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    trigChCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;
    trigChCfg[0].phase     = 0.0f;
    trigChCfg[0].duty      = ADC_TRIG_DUTY_INIT;
    trigChCfg[0].dtm       = NULL_PTR;
    trigChCfg[0].output    = &trigOutCfg[0];
    trigChCfg[0].mscOut    = NULL_PTR;
    trigChCfg[0].interrupt = NULL_PTR;

    trigConfig.cluster             = IfxEgtm_Cluster_0;
    trigConfig.subModule           = IfxEgtm_Pwm_SubModule_atom;
    trigConfig.alignment           = IfxEgtm_Pwm_Alignment_edge;
    trigConfig.numChannels         = (uint8)NUM_OF_ADC_TRIG_CHANNELS;
    trigConfig.channels            = trigChCfg;
    trigConfig.frequency           = PWM_FREQUENCY_HZ;
    trigConfig.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;
    trigConfig.dtmClockSource      = IfxEgtm_Dtm_ClockSource_systemClock;
    trigConfig.syncUpdateEnabled   = FALSE;
    trigConfig.syncStart           = FALSE;

    IfxEgtm_Pwm_init(&g_egtmAtomState.pwmTrig, g_egtmAtomState.channelsTrig, &trigConfig);

    /* 12) Configure heartbeat GPIO AFTER PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 13) Route EGTM ATOM trigger to TMADC (falling edge and mux selection are project ADC configs) */
    {
        /* ATOM0 CH3 -> ADC trigger signal 0 (mux selection handled in ADC config) */
        (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                        IfxEgtm_TrigSource_atom0,
                                        IfxEgtm_TrigChannel_3,
                                        IfxEgtm_Cfg_AdcTriggerSignal_0);
    }

    /* 14) Enable ADC module and run TMADC engine (detailed ADC cfg is elsewhere) */
    IfxAdc_enableModule(&MODULE_ADC);
    IfxAdc_Tmadc_runModule(&g_egtmAtomState.tmadc);
}

/*
 * Update three complementary channel duties synchronously.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy raw percent duties (no scaling) */
    g_egtmAtomState.dutyCycles[0] = requestDuty[0];
    g_egtmAtomState.dutyCycles[1] = requestDuty[1];
    g_egtmAtomState.dutyCycles[2] = requestDuty[2];

    /* Apply immediate synchronous update at next safe point */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtomState.pwmInv, (float32 *)g_egtmAtomState.dutyCycles);
}

/*
 * TMADC Result ISR: iterate configured result registers (CH0..CH4),
 * clear flags after consuming results (actual read handled elsewhere).
 */
void resultISR(void)
{
    uint8 i;
    for (i = 0u; i < 5u; ++i)
    {
        IfxAdc_TmadcResultReg reg = (IfxAdc_TmadcResultReg)i;
        if (IfxAdc_isTmadcResultAvailable((Ifx_ADC_TMADC *)NULL_PTR, reg) == TRUE)
        {
            /* Result read is managed by project-specific ADC data path (not here). */
            /* Honor wait-for-read by clearing the result flag. */
            IfxAdc_clearTmadcResultFlag((Ifx_ADC_TMADC *)NULL_PTR, reg);
        }
    }
}
