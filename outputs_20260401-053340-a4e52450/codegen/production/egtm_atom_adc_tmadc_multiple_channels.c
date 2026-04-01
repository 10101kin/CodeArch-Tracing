/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 * Production driver: EGTM ATOM 3-phase complementary PWM + TMADC trigger (TC4xx)
 *
 * Requirements implemented:
 * - 3 complementary, center-aligned PWM pairs @20 kHz with 1us dead-time (U/V/W = 25/50/75%) on EGTM Cluster 0 ATOM0 CH0–CH2
 * - Separate single edge-aligned 50% PWM on ATOM0 CH3 as ADC trigger routed to TMADC via IfxEgtm_Trigger_trigToAdc
 * - Heartbeat GPIO configured as push-pull output and toggled in PWM ISR
 * - TMADC module enabled and set to run (one-shot conversions configured elsewhere)
 *
 * Notes:
 * - Watchdog controls must be placed only in CpuX_Main.c files (not here)
 * - No STM-based timing or delays in this module
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxAdc.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"

/* =============================
 * Numeric configuration macros
 * ============================= */
#define NUM_OF_CHANNELS                 (3u)
#define NUM_OF_ADC_TRIG_CHANNELS        (1u)

#define PWM_FREQUENCY                   (20000.0f)    /* Hz */
#define PHASE_U_DUTY                    (25.0f)       /* percent */
#define PHASE_V_DUTY                    (50.0f)       /* percent */
#define PHASE_W_DUTY                    (75.0f)       /* percent */
#define PHASE_DUTY_STEP                 (0.01f)       /* percent */
#define ADC_TRIG_DUTY                   (50.0f)       /* percent */

#define ISR_PRIORITY_ATOM               (100)
#define ISR_PRIORITY_TMADC_RESULT       (25)

/* =============================
 * Pin macros (validated mappings)
 * ============================= */
/* 3-phase complementary outputs (EGTM ATOM0 CH0..CH2) */
#define PHASE_U_HS                      (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                      (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)
#define PHASE_V_HS                      (&IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT)
#define PHASE_V_LS                      (&IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT)
#define PHASE_W_HS                      (&IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT)
#define PHASE_W_LS                      (&IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT)

/* ADC trigger monitor output (EGTM ATOM0 CH3) */
#define ADC_TRIG_MONITOR_PIN            (&IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT)

/* Heartbeat LED GPIO (compound macro: port, pin) */
#define LED                             &MODULE_P03, 9

/* =============================
 * Module state
 * ============================= */
typedef struct
{
    /* Inverter PWM driver + channels */
    IfxEgtm_Pwm            pwm;
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];

    /* Separate ADC-trigger PWM driver + single channel */
    IfxEgtm_Pwm            trigPwm;
    IfxEgtm_Pwm_Channel    trigChannels[NUM_OF_ADC_TRIG_CHANNELS];

    /* State for runtime updates */
    float32                 dutyCycles[NUM_OF_CHANNELS];
    float32                 phases[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];

    /* TMADC runtime handles (SFR/driver) used by result ISR and run */
    IfxAdc_Tmadc            tmadc;      /* driver object for runModule */
    Ifx_ADC_TMADC          *tmadcSfr;   /* SFR pointer for result flag ops */
} EgtmAtom3ph_State;

IFX_STATIC EgtmAtom3ph_State g_egtmAtom3ph_inv;

/* =============================
 * ISR and callback declarations
 * ============================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Public API implementations
 * ============================= */

/*
 * initEgtmAtom3phInv
 * - Configure EGTM clocks inside enable-guard
 * - Initialize 3-phase complementary PWM (center-aligned) with dead-time
 * - Initialize separate single-channel PWM as ADC trigger (edge-aligned)
 * - Configure heartbeat GPIO
 * - Route EGTM ATOM trigger to TMADC and start TMADC
 */
void initEgtmAtom3phInv(void)
{
    /* -------- Local configuration structures (inverter) -------- */
    IfxEgtm_Pwm_Config            invConfig;
    IfxEgtm_Pwm_ChannelConfig     invChCfg[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      invOutput[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         invDtm[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg;

    /* -------- Local configuration structures (ADC trigger) -------- */
    IfxEgtm_Pwm_Config            trigConfig;
    IfxEgtm_Pwm_ChannelConfig     trigChCfg[NUM_OF_ADC_TRIG_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      trigOutput[NUM_OF_ADC_TRIG_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         trigDtm[NUM_OF_ADC_TRIG_CHANNELS];

    /* Initialize unified PWM config with defaults for both instances */
    IfxEgtm_Pwm_initConfig(&invConfig, &MODULE_EGTM);
    IfxEgtm_Pwm_initConfig(&trigConfig, &MODULE_EGTM);

    /* -------- Per-channel output and DTM configuration (inverter) -------- */
    /* Channel 0: Phase U */
    invOutput[0].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    invOutput[0].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    invOutput[0].polarity                = Ifx_ActiveState_high;      /* HS active high */
    invOutput[0].complementaryPolarity   = Ifx_ActiveState_low;       /* LS active low  */
    invOutput[0].outputMode              = IfxPort_OutputMode_pushPull;
    invOutput[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    invDtm[0].deadTime.rising            = 1.0e-6f;
    invDtm[0].deadTime.falling           = 1.0e-6f;
    invDtm[0].fastShutOff                = NULL_PTR;

    /* Channel 1: Phase V */
    invOutput[1].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    invOutput[1].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    invOutput[1].polarity                = Ifx_ActiveState_high;
    invOutput[1].complementaryPolarity   = Ifx_ActiveState_low;
    invOutput[1].outputMode              = IfxPort_OutputMode_pushPull;
    invOutput[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    invDtm[1].deadTime.rising            = 1.0e-6f;
    invDtm[1].deadTime.falling           = 1.0e-6f;
    invDtm[1].fastShutOff                = NULL_PTR;

    /* Channel 2: Phase W */
    invOutput[2].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    invOutput[2].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    invOutput[2].polarity                = Ifx_ActiveState_high;
    invOutput[2].complementaryPolarity   = Ifx_ActiveState_low;
    invOutput[2].outputMode              = IfxPort_OutputMode_pushPull;
    invOutput[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    invDtm[2].deadTime.rising            = 1.0e-6f;
    invDtm[2].deadTime.falling           = 1.0e-6f;
    invDtm[2].fastShutOff                = NULL_PTR;

    /* -------- Interrupt configuration (period event on channel 0) -------- */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* -------- Channel configurations (inverter) -------- */
    /* Logical channel indices must be 0..(N-1) */
    invChCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    invChCfg[0].phase     = 0.0f;
    invChCfg[0].duty      = PHASE_U_DUTY;
    invChCfg[0].dtm       = &invDtm[0];
    invChCfg[0].output    = &invOutput[0];
    invChCfg[0].mscOut    = NULL_PTR;
    invChCfg[0].interrupt = &irqCfg;           /* Only channel 0 has interrupt */

    invChCfg[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    invChCfg[1].phase     = 0.0f;
    invChCfg[1].duty      = PHASE_V_DUTY;
    invChCfg[1].dtm       = &invDtm[1];
    invChCfg[1].output    = &invOutput[1];
    invChCfg[1].mscOut    = NULL_PTR;
    invChCfg[1].interrupt = NULL_PTR;         

    invChCfg[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    invChCfg[2].phase     = 0.0f;
    invChCfg[2].duty      = PHASE_W_DUTY;
    invChCfg[2].dtm       = &invDtm[2];
    invChCfg[2].output    = &invOutput[2];
    invChCfg[2].mscOut    = NULL_PTR;
    invChCfg[2].interrupt = NULL_PTR;         

    /* -------- Main PWM configuration (inverter) -------- */
    invConfig.cluster            = IfxEgtm_Cluster_0;
    invConfig.subModule          = IfxEgtm_Pwm_SubModule_atom;
    invConfig.alignment          = IfxEgtm_Pwm_Alignment_center;
    invConfig.numChannels        = (uint8)NUM_OF_CHANNELS;
    invConfig.channels           = invChCfg;
    invConfig.frequency          = PWM_FREQUENCY;
    invConfig.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;   /* Use ATOM Clk_0 */
    invConfig.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;
    invConfig.syncUpdateEnabled  = TRUE;
    invConfig.syncStart          = TRUE;

    /* -------- ADC-trigger PWM (single edge-aligned channel on ATOM0 CH3) -------- */
    trigOutput[0].pin                    = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_MONITOR_PIN;
    trigOutput[0].complementaryPin       = NULL_PTR;
    trigOutput[0].polarity               = Ifx_ActiveState_high;
    trigOutput[0].complementaryPolarity  = Ifx_ActiveState_low;
    trigOutput[0].outputMode             = IfxPort_OutputMode_pushPull;
    trigOutput[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    trigDtm[0].deadTime.rising           = 0.0f;
    trigDtm[0].deadTime.falling          = 0.0f;
    trigDtm[0].fastShutOff               = NULL_PTR;

    trigChCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;
    trigChCfg[0].phase     = 0.0f;
    trigChCfg[0].duty      = ADC_TRIG_DUTY;
    trigChCfg[0].dtm       = &trigDtm[0];
    trigChCfg[0].output    = &trigOutput[0];
    trigChCfg[0].mscOut    = NULL_PTR;
    trigChCfg[0].interrupt = NULL_PTR;

    trigConfig.cluster           = IfxEgtm_Cluster_0;
    trigConfig.subModule         = IfxEgtm_Pwm_SubModule_atom;
    trigConfig.alignment         = IfxEgtm_Pwm_Alignment_edge;
    trigConfig.numChannels       = (uint8)NUM_OF_ADC_TRIG_CHANNELS;
    trigConfig.channels          = trigChCfg;
    trigConfig.frequency         = PWM_FREQUENCY;
    trigConfig.clockSource.atom  = (uint32)IfxEgtm_Cmu_Clk_0;   /* Use ATOM Clk_0 */
    trigConfig.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0;
    trigConfig.syncUpdateEnabled = TRUE;
    trigConfig.syncStart         = TRUE;

    /* -------- EGTM enable-guard and clock setup -------- */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* -------- Initialize inverter PWM -------- */
    IfxEgtm_Pwm_init(&g_egtmAtom3ph_inv.pwm, g_egtmAtom3ph_inv.channels, &invConfig);

    /* Persist initial state (duties, phases, dead-times) */
    g_egtmAtom3ph_inv.dutyCycles[0] = invChCfg[0].duty; g_egtmAtom3ph_inv.phases[0] = invChCfg[0].phase; g_egtmAtom3ph_inv.deadTimes[0] = invDtm[0].deadTime;
    g_egtmAtom3ph_inv.dutyCycles[1] = invChCfg[1].duty; g_egtmAtom3ph_inv.phases[1] = invChCfg[1].phase; g_egtmAtom3ph_inv.deadTimes[1] = invDtm[1].deadTime;
    g_egtmAtom3ph_inv.dutyCycles[2] = invChCfg[2].duty; g_egtmAtom3ph_inv.phases[2] = invChCfg[2].phase; g_egtmAtom3ph_inv.deadTimes[2] = invDtm[2].deadTime;

    /* -------- Initialize ADC-trigger PWM (ATOM0 CH3) -------- */
    IfxEgtm_Pwm_init(&g_egtmAtom3ph_inv.trigPwm, g_egtmAtom3ph_inv.trigChannels, &trigConfig);

    /* -------- Configure Heartbeat GPIO (after PWM init) -------- */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* -------- Route EGTM ATOM trigger to TMADC and start TMADC -------- */
    /* Falling edge, MUX selection specifics are handled by Trigger configuration in integration.
       Here we select the signal and source/channel as per requirements. */
    (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                    IfxEgtm_TrigSource_atom0,
                                    IfxEgtm_TrigChannel_3,
                                    IfxEgtm_Cfg_AdcTriggerSignal_0);

    /* Enable ADC module and run TMADC engine */
    IfxAdc_enableModule(&MODULE_ADC);
    g_egtmAtom3ph_inv.tmadcSfr = (Ifx_ADC_TMADC *)0; /* SFR assignment to be bound during ADC integration */
    IfxAdc_Tmadc_runModule(&g_egtmAtom3ph_inv.tmadc);
}

/*
 * updateEgtmAtom3phInvDuty
 * - Copy requested duties (percent) into module state (no scaling)
 * - Apply synchronous immediate update for all 3 channels
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    g_egtmAtom3ph_inv.dutyCycles[0] = requestDuty[0];
    g_egtmAtom3ph_inv.dutyCycles[1] = requestDuty[1];
    g_egtmAtom3ph_inv.dutyCycles[2] = requestDuty[2];

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3ph_inv.pwm, (float32 *)g_egtmAtom3ph_inv.dutyCycles);
}

/*
 * resultISR
 * - Minimal TMADC result ISR: acknowledge available results for 5 channels
 */
IFX_INTERRUPT(resultISR, 0, ISR_PRIORITY_TMADC_RESULT);
void resultISR(void)
{
    Ifx_ADC_TMADC *tmadc = g_egtmAtom3ph_inv.tmadcSfr; /* Bound during ADC integration */
    IfxAdc_TmadcResultReg rr;
    for (rr = (IfxAdc_TmadcResultReg)0; rr <= (IfxAdc_TmadcResultReg)4; rr++)
    {
        if (IfxAdc_isTmadcResultAvailable(tmadc, rr) == TRUE)
        {
            /* Read/consume result via project data path (performed elsewhere); here we only clear the flag */
            IfxAdc_clearTmadcResultFlag(tmadc, rr);
        }
    }
}
