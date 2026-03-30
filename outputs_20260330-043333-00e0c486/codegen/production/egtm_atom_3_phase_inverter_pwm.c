/**
 * @file egtm_atom_3_phase_inverter_pwm.c
 * @brief Production driver: eGTM ATOM0 Cluster 0 complementary, center-aligned 3-phase inverter PWM (TC4xx)
 *
 * - Submodule: ATOM0 Cluster 0
 * - Frequency: 20 kHz
 * - Dead-time: 1 us rising/falling
 * - Initial duties: U=25%, V=50%, W=75%
 * - Duty step: 10%
 * - Interrupt: priority 20, provider CPU0, VM Id 0
 * - Pins (user requested, placeholders if not in validated list):
 *   U HS=P20.8 (TOUT64), U LS=P20.9 (TOUT65)
 *   V HS=P21.4 (TOUT55), V LS=P20.11 (TOUT67)
 *   W HS=P20.12 (TOUT68), W LS=P20.13 (TOUT69)
 *   LED=P03.9
 *
 * Notes:
 * - Uses IfxEgtm_Pwm unified high-level driver
 * - Follows iLLD init pattern: initConfig -> customize -> init
 * - eGTM clocks enabled in guard, using getModuleFrequency + setGclkFrequency + setClkFrequency
 * - No watchdog handling here (handled only in CpuX_Main.c)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define NUM_OF_CHANNELS        (3u)
#define PWM_FREQUENCY          (20000.0f)
#define ISR_PRIORITY_ATOM      (20)

#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)
#define PHASE_DUTY_STEP        (10.0f)

/* LED P03.9 (compound macro: port, pin) */
#define LED                    &MODULE_P03, 9

/* =============================
 * Validated/Requested Pin Mapping
 * ============================= */
/*
 * Use ONLY validated pin symbols when available. For user-requested pins not
 * present in the validated list, provide NULL_PTR placeholders to be replaced
 * during board integration with the correct IfxEgtm_PinMap symbol.
 */
/* U phase */
#define PHASE_U_HS   (NULL_PTR)                               /* Replace with validated &IfxEgtm_ATOM0_x_TOUTxx_P20_8_OUT */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)     /* Validated: P20.9 TOUT65 */
/* V phase */
#define PHASE_V_HS   (NULL_PTR)                               /* Replace with validated &IfxEgtm_ATOM0_x_TOUTxx_P21_4_OUT */
#define PHASE_V_LS   (NULL_PTR)                               /* Replace with validated &IfxEgtm_ATOM0_x_TOUTxx_P20_11_OUT */
/* W phase */
#define PHASE_W_HS   (NULL_PTR)                               /* Replace with validated &IfxEgtm_ATOM0_x_TOUTxx_P20_12_OUT */
#define PHASE_W_LS   (NULL_PTR)                               /* Replace with validated &IfxEgtm_ATOM0_x_TOUTxx_P20_13_OUT */

/* =============================
 * Module State
 * ============================= */

typedef struct
{
    IfxEgtm_Pwm             pwm;                                   /* Driver handle */
    IfxEgtm_Pwm_Channel     channels[NUM_OF_CHANNELS];             /* Persistent channel data */
    float32                 dutyCycles[NUM_OF_CHANNELS];            /* Duty in percent */
    float32                 phases[NUM_OF_CHANNELS];                /* Phase offsets (0.0f) */
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];            /* Dead-time settings */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =============================
 * ISR (period) — minimal handler
 * ============================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* =============================
 * Period callback (assigned in InterruptConfig)
 * ============================= */
void interruptEgtmAtomPeriod(void)
{
    /* Per SW detailed design: toggle LED on PWM period event */
    IfxPort_togglePin(LED);
}

/**
 * @brief Initialize a 3-channel complementary, center-aligned PWM on eGTM ATOM0 Cluster 0.
 *
 * Algorithm (per design):
 * 1) Declare all configuration structures locally
 * 2) Load defaults with IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM)
 * 3) Configure complementary output pins and polarities
 * 4) Configure DTM dead-times (1 us rising/falling)
 * 5) Configure interrupt on base channel only
 * 6) Fill 3 logical channels (Ch_0..Ch_2) with phase=0 and duties U=25, V=50, W=75
 * 7) Complete main config: ATOM, Cluster 0, center alignment, syncStart/Update, 20 kHz, atom clock=CLK0, DTM clock=cmuClock0
 * 8) Inside enable guard: enable eGTM and configure CMU using module frequency, enable FXCLK and CLK0
 * 9) Initialize PWM with persistent handle and channels array
 * 10) Store initial duties/phases/dead-times in state
 * 11) Configure LED P03.9 as push-pull output
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration (complementary pairs) */
    /* Phase U */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high; /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;  /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase V */
    output[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase W */
    output[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM dead-time configuration: 1 us rising/falling */
    dtmConfig[0].deadTime.rising = 1e-6f; dtmConfig[0].deadTime.falling = 1e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1e-6f; dtmConfig[1].deadTime.falling = 1e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1e-6f; dtmConfig[2].deadTime.falling = 1e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration (base channel only) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = interruptEgtmAtomPeriod;  /* Base channel period callback */
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical channels 0..2, phase=0, duties U/V/W */
    /* CH0 → Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;   /* base channel interrupt */

    /* CH1 → Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;          /* no interrupt on CH1 */

    /* CH2 → Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;          /* no interrupt on CH2 */

    /* 7) Main configuration */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;
    config.cluster           = IfxEgtm_Cluster_0;
    config.alignment         = IfxEgtm_Pwm_Alignment_center;
    config.numChannels       = NUM_OF_CHANNELS;
    config.channels          = channelConfig;
    config.frequency         = PWM_FREQUENCY;
    config.clockSource.atom  = IfxEgtm_Cmu_Clk_0;                 /* ATOM clock source */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0; /* Align DTM to CMU CLK0 */
    config.syncUpdateEnabled = TRUE;
    config.syncStart         = TRUE;

    /* 8) eGTM enable + CMU clock configuration inside guard */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent handle/state channels */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store initial state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) LED GPIO configuration (no set-high/low) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Update three phase duties in 10% steps with wrap-around and apply immediately.
 *
 * Wrap rule: if ((duty + STEP) >= 100) duty = 0; then add STEP unconditionally.
 */
void updateEgtmAtom3phInvDuty(void)
{
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
