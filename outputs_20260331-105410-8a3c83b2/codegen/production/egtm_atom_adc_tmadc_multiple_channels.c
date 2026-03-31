/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 *
 * Production driver for EGTM ATOM 3-phase inverter PWM (complementary, center-aligned)
 * and a separate ATOM channel used as TMADC trigger. Targets TC4xx EGTM/ATOM using
 * the unified high-level IfxEgtm_Pwm driver.
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (numeric constants and pins) ========================= */
#define NUM_OF_CHANNELS            (3U)
#define PWM_FREQUENCY              (20000.0f)
#define ISR_PRIORITY_ATOM          (25)

/* Phase initial duties in percent */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)

/* Duty step (percent) retained for compatibility with ramp usage elsewhere */
#define PHASE_DUTY_STEP            (0.01f)

/* LED pin (compound macro: port, pin). User requirement: P03.9, initial state handled by application */
#define LED                        &MODULE_P03, 9

/* EGTM ATOM complementary pair pins (validated, user-requested) */
#define PHASE_U_HS                 (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                 (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                 (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                 (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                 (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                 (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ========================= Module state ========================= */

typedef struct
{
    IfxEgtm_Pwm            pwm;                              /* driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];        /* persistent channel state (owned by driver) */
    float32                dutyCycles[NUM_OF_CHANNELS];      /* duty percent per channel */
    float32                phases[NUM_OF_CHANNELS];          /* phase offset per channel (deg or percent depending on usage) */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];       /* stored deadtimes (seconds) */
} EgtmAtom3phState;

typedef struct
{
    IfxEgtm_Pwm            pwm;                              /* driver handle for trigger */
    IfxEgtm_Pwm_Channel    channels[1];                      /* single channel used for TMADC trigger */
    float32                duty;                             /* stored duty for trigger */
} EgtmAtomTrigState;

IFX_STATIC EgtmAtom3phState g_egtmAtom3phInv;
IFX_STATIC EgtmAtomTrigState g_egtmAtomAdcTrig;

/* ========================= ISR and callback ========================= */

/* Period-event callback required by unified driver (empty by design) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Debug/diagnostic ISR for EGTM ATOM: toggle LED only */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* ========================= Internal helpers (none) ========================= */

/* ========================= Public API implementation ========================= */

/**
 * Initialize a 3-channel complementary, center-aligned PWM inverter on EGTM Cluster 0 ATOM
 * channels 0..2 at 20 kHz with 1 us dead-time on both edges using the unified high-level PWM driver.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare local config structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output (pin) configuration for complementary pairs */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin     = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity             = Ifx_ActiveState_high;    /* HS active HIGH */
    output[0].complementaryPolarity= Ifx_ActiveState_low;     /* LS active LOW  */
    output[0].outputMode           = IfxPort_OutputMode_pushPull;
    output[0].padDriver            = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin     = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity             = Ifx_ActiveState_high;
    output[1].complementaryPolarity= Ifx_ActiveState_low;
    output[1].outputMode           = IfxPort_OutputMode_pushPull;
    output[1].padDriver            = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin     = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity             = Ifx_ActiveState_high;
    output[2].complementaryPolarity= Ifx_ActiveState_low;
    output[2].outputMode           = IfxPort_OutputMode_pushPull;
    output[2].padDriver            = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (seconds). Use 1e-6f literal per guideline */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration (attach to base channel). Fully populate fields */
    irqCfg.mode        = (IfxEgtm_IrqMode)0;           /* pulse notify mode if available */
    irqCfg.isrProvider = (IfxSrc_Tos)0;                /* cpu0 */
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = (IfxSrc_VmId)0;               /* IfxSrc_VmId_0 */
    irqCfg.periodEvent = (IfxEgtm_Pwm_callBack)IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = (IfxEgtm_Pwm_callBack)NULL_PTR;

    /* 6) Per-channel configuration */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;              /* base channel provides interrupt */

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

    /* 7) Main config fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;     /* CLOCK SOURCE UNION: set atom only */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* 4) Enable-guard and CMU clock setup */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 5) Initialize PWM with persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 6) Store initial duty and dead-times into module state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 7) Configure status LED pin AFTER PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Runtime update for the 3-phase inverter: copy duty[0..2] to persistent array and apply immediately.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy requested duties into module state without scaling */
    g_egtmAtom3phInv.dutyCycles[0] = requestDuty[0];
    g_egtmAtom3phInv.dutyCycles[1] = requestDuty[1];
    g_egtmAtom3phInv.dutyCycles[2] = requestDuty[2];

    /* Apply immediately using driver's multi-channel update API */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}

/**
 * Configure a single-channel PWM on EGTM Cluster 0 ATOM channel 3 to generate an edge-aligned 50% duty
 * time-base used as a trigger source for TMADC. Output mapped to P33.0 (ATOM0_3 TOUT22).
 */
void initEgtmAtomAdcTrigger(void)
{
    /* 1) Local config structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    chCfg[1];
    IfxEgtm_Pwm_OutputConfig     outCfg[1];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Initialize main config and set fields */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output mapping to P33.0 (single-ended output) */
    outCfg[0].pin                    = (IfxEgtm_Pwm_ToutMap *)&IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT;
    outCfg[0].complementaryPin      = NULL_PTR;
    outCfg[0].polarity              = Ifx_ActiveState_high;
    outCfg[0].complementaryPolarity = Ifx_ActiveState_low;
    outCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Interrupt configuration (optional, populated for completeness) */
    irqCfg.mode        = (IfxEgtm_IrqMode)0;           /* pulse notify mode if available */
    irqCfg.isrProvider = (IfxSrc_Tos)0;                /* cpu0 */
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = (IfxSrc_VmId)0;               /* IfxSrc_VmId_0 */
    irqCfg.periodEvent = (IfxEgtm_Pwm_callBack)IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = (IfxEgtm_Pwm_callBack)NULL_PTR;

    /* Channel configuration: ATOM ch3, 50% duty, edge aligned */
    chCfg[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;
    chCfg[0].phase     = 0.0f;
    chCfg[0].duty      = PHASE_V_DUTY;                 /* 50% */
    chCfg[0].dtm       = NULL_PTR;                     /* no complementary output => no DTM */
    chCfg[0].output    = &outCfg[0];
    chCfg[0].mscOut    = NULL_PTR;
    chCfg[0].interrupt = &irqCfg;

    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_edge;
    config.syncStart          = FALSE;
    config.syncUpdateEnabled  = FALSE;
    config.numChannels        = 1U;
    config.channels           = &chCfg[0];
    config.frequency          = PWM_FREQUENCY;         /* 20 kHz */
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;  /* CLOCK SOURCE UNION: atom */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* 4) Ensure EGTM is enabled and CMU clocks configured */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 5) Initialize PWM for TMADC trigger with persistent channel storage */
    IfxEgtm_Pwm_init(&g_egtmAtomAdcTrig.pwm, &g_egtmAtomAdcTrig.channels[0], &config);

    /* Store initial duty */
    g_egtmAtomAdcTrig.duty = chCfg[0].duty;
}
