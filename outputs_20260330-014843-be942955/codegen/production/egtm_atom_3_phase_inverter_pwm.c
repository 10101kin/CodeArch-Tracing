/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver for TC4xx eGTM/ATOM 3-Phase Complementary Center-Aligned PWM
 * MIGRATION: TC3xx -> TC4xx (IfxEgtm_* APIs)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* Required iLLD headers (selected) */
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros (configuration values) ========================= */
#define NUM_OF_CHANNELS              (3U)
#define PWM_FREQUENCY_HZ             (20000.0f)
#define ISR_PRIORITY_ATOM            (20)
#define PHASE_U_DUTY_INIT            (25.0f)
#define PHASE_V_DUTY_INIT            (50.0f)
#define PHASE_W_DUTY_INIT            (75.0f)
#define PHASE_DUTY_STEP              (10.0f)

/* LED: P03.9, compound macro used by IfxPort_* APIs */
#define LED                          &MODULE_P03, 9

/* ========================= Pin Macros (validated or placeholders) ========================= */
/*
 * Use ONLY validated pin symbols. For unavailable validated symbols, use NULL_PTR
 * and replace during integration with the correct &IfxEgtm_ATOMx_y[_N]_TOUTz_Pxx_y_OUT symbol.
 */
/* U phase */
#define PHASE_U_HS                   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT64_P20_8_OUT */
#define PHASE_U_LS                   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT) /* Validated */
/* V phase */
#define PHASE_V_HS                   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT55_P21_4_OUT */
#define PHASE_V_LS                   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT67_P20_11_OUT */
/* W phase */
#define PHASE_W_HS                   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT68_P20_12_OUT */
#define PHASE_W_LS                   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_xN_TOUT69_P20_13_OUT */

/* ========================= Module State ========================= */
typedef struct
{
    IfxEgtm_Pwm              pwm;                                   /* PWM handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];             /* Persistent channel instances */
    float32                  dutyCycles[NUM_OF_CHANNELS];           /* Duty cycles in percent */
    float32                  phases[NUM_OF_CHANNELS];               /* Phases in percent (0.0f initial) */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];            /* Dead-time copies */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback (file scope) ========================= */
/**
 * eGTM ATOM period-event ISR - toggles LED
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/**
 * Period-event callback used by the PWM driver (body must be empty)
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */
/**
 * Initialize a 3-channel complementary, center-aligned inverter PWM using the unified eGTM high-level PWM driver.
 * Algorithm/order strictly follows the SW detailed design and iLLD patterns.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as locals */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];

    /* 2) Initialize the main config with driver defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration for each channel (U, V, W) */
    /* U */
    output[0].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;  /* HS active-high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;   /* LS active-low  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V */
    output[1].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W */
    output[2].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM dead-time per channel: rising=1us, falling=1us */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: period event, pulse notify, CPU0, prio=20 */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: indices 0..2 => U/V/W with initial duties 25/50/75 */
    /* CH0 - U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* base channel gets interrupt */

    /* CH1 - V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* CH2 - W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Complete main config fields */
    config.cluster            = IfxEgtm_Cluster_0;                         /* ATOM0 Cluster0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;                /* ATOM */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;              /* Center-aligned */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;                           /* 20 kHz */
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;                  /* ATOM clock source = CMU.CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;          /* DTM clock source = CMU.CLK0 */
    config.syncUpdateEnabled  = TRUE;                                       /* synced updates */
    config.syncStart          = TRUE;                                       /* synced start */

    /* 8) eGTM enable guard and CMU clocks setup (MANDATORY PATTERN) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver: applies duties, alignment, pins, DTM, sync */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store persistent state copies (duties, phases, dead-times) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]     = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO (no set high/low here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Step each of the three phase duties by a fixed STEP in percent and wrap around.
 * Apply the new duties atomically to all channels.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap rule: reset to 0 if (duty + STEP) >= 100, then unconditionally add STEP */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply atomically */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
