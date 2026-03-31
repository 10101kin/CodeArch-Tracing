/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: eGTM ATOM 3-phase complementary PWM for TC4xx.
 * - Unified IfxEgtm_Pwm high-level driver
 * - Center-aligned, complementary outputs with dead-time insertion
 * - Period-event ISR toggles LED; callback is empty as required
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * User/migration configuration
 * ============================= */
#define NUM_OF_CHANNELS         (3u)
#define PWM_FREQUENCY           (20000.0f)         /* Hz */
#define ISR_PRIORITY_ATOM       (20)

/* Initial duties in percent */
#define PHASE_U_DUTY            (25.0f)
#define PHASE_V_DUTY            (50.0f)
#define PHASE_W_DUTY            (75.0f)
#define PHASE_DUTY_STEP         (10.0f)

/* LED: compound macro (port, pin) */
#define LED                     &MODULE_P03, 9     /* P03.9 */

/*
 * eGTM ATOM TOUT pin macros
 *
 * NOTE:
 *   Use ONLY validated pin symbols when available. For pins that are not in the
 *   validated list excerpt, keep NULL_PTR placeholders and replace during
 *   integration with the matching IfxEgtm pin symbols (from IfxEgtm_PinMap.h):
 *     U_HS: P20.8  -> TOUT64  : &IfxEgtm_ATOMx_y_TOUT64_P20_8_OUT
 *     U_LS: P20.9  -> TOUT65  : &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT (validated)
 *     V_HS: P21.4  -> TOUT55  : &IfxEgtm_ATOMx_y_TOUT55_P21_4_OUT
 *     V_LS: P20.11 -> TOUT67  : &IfxEgtm_ATOMx_y_TOUT67_P20_11_OUT
 *     W_HS: P20.12 -> TOUT68  : &IfxEgtm_ATOMx_y_TOUT68_P20_12_OUT
 *     W_LS: P20.13 -> TOUT69  : &IfxEgtm_ATOMx_y_TOUT69_P20_13_OUT
 */
#define PHASE_U_HS              (NULL_PTR)                      /* TODO: replace with &IfxEgtm_ATOMx_y_TOUT64_P20_8_OUT */
#define PHASE_U_LS              (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)  /* validated */
#define PHASE_V_HS              (NULL_PTR)                      /* TODO: replace with &IfxEgtm_ATOMx_y_TOUT55_P21_4_OUT */
#define PHASE_V_LS              (NULL_PTR)                      /* TODO: replace with &IfxEgtm_ATOMx_y_TOUT67_P20_11_OUT */
#define PHASE_W_HS              (NULL_PTR)                      /* TODO: replace with &IfxEgtm_ATOMx_y_TOUT68_P20_12_OUT */
#define PHASE_W_LS              (NULL_PTR)                      /* TODO: replace with &IfxEgtm_ATOMx_y_TOUT69_P20_13_OUT */

/* =============================
 * Module state (persistent)
 * ============================= */

typedef struct
{
    IfxEgtm_Pwm                pwm;                               /* Driver handle */
    IfxEgtm_Pwm_Channel        channels[NUM_OF_CHANNELS];         /* Persistent channels storage */
    float32                    dutyCycles[NUM_OF_CHANNELS];       /* Duty in percent */
    float32                    phases[NUM_OF_CHANNELS];           /* Phase (center alignment -> 0) */
    IfxEgtm_Pwm_DeadTime       deadTimes[NUM_OF_CHANNELS];        /* Dead-time per channel (s) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =============================
 * ISR and callback declarations
 * ============================= */

/* Period-event ISR: minimal work, toggle LED only */
IFX_INTERRUPT(interruptEgtmAtomPeriod, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtomPeriod(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: intentionally empty */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Public API implementations
 * ============================= */

/**
 * Initialize complementary 3-channel eGTM ATOM PWM (center-aligned, 20 kHz).
 * - Configures outputs (HS active-high, LS active-low, push-pull)
 * - Sets 1 us dead-time (rising/falling)
 * - Base channel has period-event interrupt on CPU0 (priority 20)
 * - Enables eGTM CMU clocks under guard and initializes the unified driver
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration (HS active-high, LS active-low, push-pull) */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;  /* HS active-high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;   /* LS active-low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us rising/falling (seconds) */
    dtmConfig[0].deadTime.rising = 1.0e-6f; dtmConfig[0].deadTime.falling = 1.0e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1.0e-6f; dtmConfig[1].deadTime.falling = 1.0e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1.0e-6f; dtmConfig[2].deadTime.falling = 1.0e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt: period event on CPU0, priority 20, VM 0; dutyEvent disabled */
    /* All fields populated; base channel (0) will reference this */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;  /* pulse notify on period */
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical channels 0..2, zero phase (center-aligned) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;           /* base channel: period ISR source */

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
    config.cluster           = IfxEgtm_Cluster_0;                      /* ATOM0 cluster 0 */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;             /* ATOM submodule */
    config.alignment         = IfxEgtm_Pwm_Alignment_center;           /* center-aligned */
    config.syncStart         = TRUE;                                    /* synchronous start */
    config.syncUpdateEnabled = TRUE;                                    /* synchronous updates */
    config.numChannels       = (uint8)NUM_OF_CHANNELS;
    config.channels          = &channelConfig[0];
    config.frequency         = PWM_FREQUENCY;                           /* Hz */
    config.clockSource.atom  = (uint32)IfxEgtm_Cmu_Clk_0;               /* ATOM uses CLKn: CLK0 */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM CMU clock 0 */

    /* 8) eGTM enable-guard + CMU clock setup (1:1 with module frequency) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM (unified driver) with persistent channels storage */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initial state (duties, phases, dead-times) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (no level forced here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update three PWM duties with wrap-then-step and apply immediately.
 * Duty values are in percent (0..100). Applies atomically to all channels.
 */
void updateEgtmAtom3phInvDuty(void)
{
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
