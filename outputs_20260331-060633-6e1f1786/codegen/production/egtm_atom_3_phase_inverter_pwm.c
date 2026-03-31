/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver: eGTM ATOM unified PWM for 3-phase complementary inverter
 */
#include "egtm_atom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ==========================
 * Configuration Macros
 * ========================== */
#define EGTM_ATOM_NUM_CHANNELS      (3u)
#define PWM_FREQUENCY               (20000.0f)     /* Hz */
#define ISR_PRIORITY_ATOM           (20)

/* Initial duties in percent */
#define PHASE_U_DUTY                (25.0f)
#define PHASE_V_DUTY                (50.0f)
#define PHASE_W_DUTY                (75.0f)
#define PHASE_DUTY_STEP             (10.0f)

/* User-requested LED on P03.9 */
#define LED                         &MODULE_P03, 9
#define LED_PORT                    (&MODULE_P03)
#define LED_PIN                     (9u)

/* ==========================
 * Pin Routing Macros (validated list only)
 * Use NULL_PTR placeholders where a validated symbol was not provided.
 * ========================== */
/* Phase U: HS=P20.8 (TOUT64) [not in validated list], LS=P20.9 (TOUT65) */
#define PHASE_U_HS    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT during integration */
#define PHASE_U_LS    (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)

/* Phase V: HS=P21.4 (TOUT55), LS=P20.11 (TOUT67) [not in validated list] */
#define PHASE_V_HS    (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_y_TOUT55_P21_4_OUT during integration */
#define PHASE_V_LS    (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_y_TOUT67_P20_11_OUT during integration */

/* Phase W: HS=P20.12 (TOUT68), LS=P20.13 (TOUT69) [not in validated list] */
#define PHASE_W_HS    (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_y_TOUT68_P20_12_OUT during integration */
#define PHASE_W_LS    (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_y_TOUT69_P20_13_OUT during integration */

/* ==========================
 * Module State
 * ========================== */
typedef struct
{
    IfxEgtm_Pwm             pwm;                                      /* unified PWM driver handle */
    IfxEgtm_Pwm_Channel     channels[EGTM_ATOM_NUM_CHANNELS];         /* persistent channel storage */
    float32                 dutyCycles[EGTM_ATOM_NUM_CHANNELS];       /* duty in percent */
    float32                 phases[EGTM_ATOM_NUM_CHANNELS];           /* phase offset (fraction of period) */
    IfxEgtm_Pwm_DeadTime    deadTimes[EGTM_ATOM_NUM_CHANNELS];        /* per-channel dead-times (seconds) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ==========================
 * ISR and Callback (declare before init)
 * ========================== */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ==========================
 * Public Functions
 * ========================== */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[EGTM_ATOM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[EGTM_ATOM_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   irqCfg;
    IfxEgtm_Pwm_OutputConfig      output[EGTM_ATOM_NUM_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration for each logical channel (HS/LS complementary) */
    /* Channel 0 → Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;           /* HS active-high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;            /* LS active-low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 1 → Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 2 → Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us rising, 1 us falling (in seconds) */
    dtmConfig[0].deadTime.rising = 1.0e-6f; dtmConfig[0].deadTime.falling = 1.0e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1.0e-6f; dtmConfig[1].deadTime.falling = 1.0e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1.0e-6f; dtmConfig[2].deadTime.falling = 1.0e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration (period event) */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Per-channel configuration */
    /* CH0 → Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;              /* only first channel installs interrupt */

    /* CH1 → Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* CH2 → Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Complete main configuration */
    config.cluster            = IfxEgtm_Cluster_0;                     /* Cluster 0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;            /* ATOM sub-module */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;          /* center-aligned */
    config.numChannels        = (uint8)EGTM_ATOM_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;             /* ATOM time base from CMU CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;     /* DTM from CMU Clock0 */
    config.syncUpdateEnabled  = TRUE;
    config.syncStart          = TRUE;

    /* 8) Enable-guard and CMU setup */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent handle and channels */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist initial state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED pin as push-pull output (no state change here) */
    IfxPort_setPinModeOutput(LED_PORT, LED_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(void)
{
    /* Duty wrap rule: reset to 0 if next step would reach/exceed 100, then always add step */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply all three duties atomically (percent values) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
