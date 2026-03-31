/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 * Production driver: EGTM ATOM PWM 3-phase inverter + TMADC trigger (TC4xx)
 */

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"
#include "egtm_atom_adc_tmadc_multiple_channels.h"

/* ==========================
 * Macros (numerical values)
 * ========================== */
#define EGTM_INV_NUM_CHANNELS             (3u)
#define EGTM_ADC_TRIG_NUM_CHANNELS        (1u)
#define EGTM_PWM_FREQUENCY_HZ             (20000.0f)
#define EGTM_PHASE_U_DUTY                 (25.0f)
#define EGTM_PHASE_V_DUTY                 (50.0f)
#define EGTM_PHASE_W_DUTY                 (75.0f)
#define EGTM_PHASE_DUTY_STEP              (0.01f)
#define EGTM_ADC_TRIG_DUTY                (50.0f)
#define ISR_PRIORITY_ATOM                 (25)

/* ==========================
 * Pin assignment macros
 * ========================== */
/* 3-phase inverter complementary pairs (EGTM ATOM0 CH0..2) */
#define PHASE_U_HS                        (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                        (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                        (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                        (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                        (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                        (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ADC trigger output (EGTM ATOM0 CH3) */
#define EGTM_ADC_TRIG_PIN                 (&IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT)

/* Status LED: P03.9 (compound macro: port, pin) */
#define LED                               &MODULE_P03, 9u

/* ==========================
 * Module state structures
 * ========================== */

typedef struct
{
    IfxEgtm_Pwm               pwm;                                        /* PWM driver handle */
    IfxEgtm_Pwm_Channel       channels[EGTM_INV_NUM_CHANNELS];            /* Persistent channels array */
    float32                   dutyCycles[EGTM_INV_NUM_CHANNELS];          /* Duty cycle cache [%] */
    float32                   phases[EGTM_INV_NUM_CHANNELS];              /* Phase offsets [%], reserved */
    IfxEgtm_Pwm_DeadTime      deadTimes[EGTM_INV_NUM_CHANNELS];           /* Dead-time cache (s) */
} EgtmAtom3phInv_State;

typedef struct
{
    IfxEgtm_Pwm               pwm;                                        /* PWM driver handle for ADC trigger */
    IfxEgtm_Pwm_Channel       channels[EGTM_ADC_TRIG_NUM_CHANNELS];       /* Persistent channel for trigger */
    float32                   dutyCycles[EGTM_ADC_TRIG_NUM_CHANNELS];     /* Duty for trigger channel */
    float32                   phases[EGTM_ADC_TRIG_NUM_CHANNELS];         /* Not used */
    IfxEgtm_Pwm_DeadTime      deadTimes[EGTM_ADC_TRIG_NUM_CHANNELS];      /* Not used (no complementary) */
} EgtmAtomAdcTrig_State;

IFX_STATIC EgtmAtom3phInv_State   g_egtmAtom3phInv;
IFX_STATIC EgtmAtomAdcTrig_State  g_egtmAtomAdcTrig;

/* ==========================
 * ISR and callbacks
 * ========================== */

/* Period event callback (must be visible, empty body) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ATOM interrupt (debug): toggle LED only */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* ==========================
 * Public API implementation
 * ========================== */

void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_INV_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM_INV_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_INV_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output configurations for complementary pairs */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* LS active low  */
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

    /* 3) Dead-time configurations: 1 us on both edges (seconds) */
    dtmConfig[0].deadTime.rising = 1.0e-6f;  dtmConfig[0].deadTime.falling = 1.0e-6f;  dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1.0e-6f;  dtmConfig[1].deadTime.falling = 1.0e-6f;  dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1.0e-6f;  dtmConfig[2].deadTime.falling = 1.0e-6f;  dtmConfig[2].fastShutOff = NULL_PTR;

    /* 4) Interrupt configuration (attach to base channel only) */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 5) Channel configurations */
    /* Channel 0: Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = EGTM_PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;

    /* Channel 1: Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = EGTM_PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Channel 2: Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = EGTM_PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 6) Main PWM configuration fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.numChannels        = (uint8)EGTM_INV_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;  /* UNION: set only .atom for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_systemClock;

    /* 7) Enable-guard and CMU clock setup */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize the PWM driver with persistent channel storage */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 9) Persist initial duty and dead-time values for runtime updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure status LED pin (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy request duty cycles into persistent state (no scaling) */
    g_egtmAtom3phInv.dutyCycles[0] = requestDuty[0];
    g_egtmAtom3phInv.dutyCycles[1] = requestDuty[1];
    g_egtmAtom3phInv.dutyCycles[2] = requestDuty[2];

    /* Apply immediately to all configured channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}

void initEgtmAtomAdcTrigger(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_ADC_TRIG_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_ADC_TRIG_NUM_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output (single channel, non-complementary) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)EGTM_ADC_TRIG_PIN;
    output[0].complementaryPin      = NULL_PTR;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 3) Channel config: ATOM ch3, 50% duty, edge-aligned */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = EGTM_ADC_TRIG_DUTY;
    channelConfig[0].dtm       = NULL_PTR;
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR;

    /* 4) Main PWM configuration fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_edge;
    config.syncStart          = FALSE;
    config.syncUpdateEnabled  = FALSE;
    config.numChannels        = (uint8)EGTM_ADC_TRIG_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;  /* UNION: set only .atom for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_systemClock;

    /* 5) Ensure EGTM is enabled and clocks are configured */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM for ADC trigger channel */
    IfxEgtm_Pwm_init(&g_egtmAtomAdcTrig.pwm, &g_egtmAtomAdcTrig.channels[0], &config);

    /* Persist initial values (optional) */
    g_egtmAtomAdcTrig.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtomAdcTrig.phases[0]     = channelConfig[0].phase;
}
