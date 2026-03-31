/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 * Production driver: EGTM ATOM 3-phase complementary PWM and TMADC trigger (TC4xx)
 *
 * Implementation follows authoritative iLLD patterns and migration TC3xx -> TC4xx.
 * Submodules:
 *  - 3-phase inverter: EGTM Cluster 0, ATOM channels 0..2, center-aligned, complementary outputs, 1 us dead-time
 *  - ADC trigger:     EGTM Cluster 0, ATOM channel 3, edge-aligned 50% duty (TMADC connection configured elsewhere)
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (config values) ========================= */
#define EGTM_NUM_CHANNELS                (3U)
#define EGTM_NUM_ADC_TRIG_CHANNELS       (1U)

#define EGTM_PWM_FREQUENCY_HZ            (20000.0f)         /* 20 kHz */
#define PHASE_U_DUTY                     (25.0f)            /* percent */
#define PHASE_V_DUTY                     (50.0f)            /* percent */
#define PHASE_W_DUTY                     (75.0f)            /* percent */
#define PHASE_DUTY_STEP                  (0.01f)            /* percent step (not used here) */

/* ISR priority for ATOM PWM */
#define ISR_PRIORITY_ATOM                (25)

/* LED pin (compound macro: port, pin) */
#define LED                              &MODULE_P03, 9

/* ========================= Pin routing macros (validated) ========================= */
/* 3-phase complementary outputs: EGTM ATOM0 channels 0..2 */
#define PHASE_U_HS                       (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                       (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                       (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                       (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                       (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                       (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ADC trigger output on ATOM0 channel 3 */
#define ADC_TRIG_PIN                     (&IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT)

/* ========================= Module state ========================= */

typedef struct
{
    IfxEgtm_Pwm            pwm;                                 /* driver handle */
    IfxEgtm_Pwm_Channel    channels[EGTM_NUM_CHANNELS];         /* persistent channel runtime data */
    float32                dutyCycles[EGTM_NUM_CHANNELS];       /* duty in percent */
    float32                phases[EGTM_NUM_CHANNELS];           /* phase in percent (0 by default) */
    IfxEgtm_Pwm_DeadTime   deadTimes[EGTM_NUM_CHANNELS];        /* dead-time (seconds) */
} EgtmAtom3phInv_State;

typedef struct
{
    IfxEgtm_Pwm            pwm;                                 /* driver handle */
    IfxEgtm_Pwm_Channel    channels[EGTM_NUM_ADC_TRIG_CHANNELS];
    float32                dutyCycles[EGTM_NUM_ADC_TRIG_CHANNELS];
    float32                phases[EGTM_NUM_ADC_TRIG_CHANNELS];
    IfxEgtm_Pwm_DeadTime   deadTimes[EGTM_NUM_ADC_TRIG_CHANNELS];
} EgtmAtomAdcTrig_State;

IFX_STATIC EgtmAtom3phInv_State   g_egtmAtom3phInv;
IFX_STATIC EgtmAtomAdcTrig_State  g_egtmAtomAdcTrig;

/* ========================= ISR and callback ========================= */
/* Empty period-event callback (assigned via InterruptConfig) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ATOM ISR: toggle LED only */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API implementations ========================= */

/*
 * Initialize 3-channel complementary, center-aligned PWM inverter on EGTM Cluster 0 ATOM CH0..2
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare local config structures */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[EGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[EGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      output[EGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg;

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output configuration (complementary pairs, polarity convention) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;  /* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;   /* LS active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us both edges (seconds) */
    dtmConfig[0].deadTime.rising    = 1.0e-6f;
    dtmConfig[0].deadTime.falling   = 1.0e-6f;
    dtmConfig[0].fastShutOff        = NULL_PTR;

    dtmConfig[1].deadTime.rising    = 1.0e-6f;
    dtmConfig[1].deadTime.falling   = 1.0e-6f;
    dtmConfig[1].fastShutOff        = NULL_PTR;

    dtmConfig[2].deadTime.rising    = 1.0e-6f;
    dtmConfig[2].deadTime.falling   = 1.0e-6f;
    dtmConfig[2].fastShutOff        = NULL_PTR;

    /* 5) Interrupt configuration for base channel (period event) */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical channels 0..2 mapped to ATOM CH0..2) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;               /* base interrupt on first channel */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.numChannels        = (uint8)EGTM_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;           /* ATOM clock source */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;   /* DTM clock source */

    /* 8) Enable-guard and CMU clock setup */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM (persistent channels array stored in module state) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initial duty/dead-time into persistent state arrays */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure status LED GPIO as output after PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Runtime update: copy requestDuty[0..2] to persistent state and update immediately
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy duties into persistent state (no scaling) */
    g_egtmAtom3phInv.dutyCycles[0] = requestDuty[0];
    g_egtmAtom3phInv.dutyCycles[1] = requestDuty[1];
    g_egtmAtom3phInv.dutyCycles[2] = requestDuty[2];

    /* Apply immediately to configured channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}

/*
 * Initialize single-channel ATOM0 CH3 as edge-aligned 50% duty time-base for TMADC trigger
 */
void initEgtmAtomAdcTrigger(void)
{
    /* 1) Declare local config structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_NUM_ADC_TRIG_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_NUM_ADC_TRIG_CHANNELS];

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output configuration: route ATOM0 CH3 to P33.0 */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_PIN;
    output[0].complementaryPin      = NULL_PTR;                       /* not used */
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;            /* irrelevant here */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel config: physical ATOM channel 3, 50% duty */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = 50.0f;
    channelConfig[0].dtm       = NULL_PTR;                            /* no complementary output */
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR;                            /* no interrupt for trigger */

    /* Main PWM configuration */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_edge;           /* edge-aligned */
    config.syncStart          = FALSE;
    config.syncUpdateEnabled  = FALSE;
    config.numChannels        = (uint8)EGTM_NUM_ADC_TRIG_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* 4) Ensure EGTM is enabled and clocks configured */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 5) Initialize PWM (use persistent channel storage) */
    IfxEgtm_Pwm_init(&g_egtmAtomAdcTrig.pwm, &g_egtmAtomAdcTrig.channels[0], &config);

    /* Store initial duty/phase in persistent state (not strictly required for trigger) */
    g_egtmAtomAdcTrig.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtomAdcTrig.phases[0]     = channelConfig[0].phase;
}
