/**********************************************************************************************************************
 *  File: egtm_atom_3_phase_inverter_pwm.c
 *  Brief: eGTM ATOM 3-phase complementary PWM driver for TC4xx (IfxEgtm_Pwm)
 *
 *  Notes:
 *   - Uses unified IfxEgtm_Pwm driver on ATOM (Cluster 0), center-aligned, 20 kHz, complementary outputs with 1 us DT
 *   - ISR toggles LED P03.9; period-event callback is empty. No watchdog handling here (CPU files only).
 *   - Pin routing via OutputConfig array; verify TOUT-to-pin mapping for target device per IfxEgtm_PinMap.h
 *********************************************************************************************************************/

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ==========================================================
 * Macros (configuration constants)
 * ========================================================== */
#define NUM_OF_CHANNELS          (3u)
#define PWM_FREQUENCY            (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM        (20)

/* Initial duties in percent */
#define PHASE_U_DUTY             (25.0f)
#define PHASE_V_DUTY             (50.0f)
#define PHASE_W_DUTY             (75.0f)

/* Duty step (not used in init; reserved for application updates) */
#define PHASE_DUTY_STEP          (10.0f)

/* LED macro: compound form (port, pin) */
#define LED                      &MODULE_P03, 9

/* ==========================================================
 * Pin routing macros (use only validated symbols when available)
 * ========================================================== */
/*
 * User-requested routing (to be validated on the target):
 *   U: HS -> P20.8 / TOUT64, LS -> P20.9 / TOUT65
 *   V: HS -> P21.4 / TOUT55, LS -> P20.11 / TOUT67
 *   W: HS -> P20.12 / TOUT68, LS -> P20.13 / TOUT69
 *
 * From validated list, only the following symbol is available among requested pins:
 *   &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT
 *
 * For unavailable validated pin symbols, keep NULL_PTR placeholders and replace during integration
 * with the exact symbols from IfxEgtm_PinMap.h for the target device/package.
 */
#define PHASE_U_HS   (NULL_PTR)                              /* Replace with &IfxEgtm_ATOMx_y_TOUT64_P20_8_OUT */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)    /* Validated symbol available */
#define PHASE_V_HS   (NULL_PTR)                              /* Replace with &IfxEgtm_ATOMx_y_TOUT55_P21_4_OUT */
#define PHASE_V_LS   (NULL_PTR)                              /* Replace with &IfxEgtm_ATOMx_y_TOUT67_P20_11_OUT */
#define PHASE_W_HS   (NULL_PTR)                              /* Replace with &IfxEgtm_ATOMx_y_TOUT68_P20_12_OUT */
#define PHASE_W_LS   (NULL_PTR)                              /* Replace with &IfxEgtm_ATOMx_y_TOUT69_P20_13_OUT */

/* ==========================================================
 * Module state
 * ========================================================== */
typedef struct
{
    IfxEgtm_Pwm            pwm;                                      /* Driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];                /* Persistent channels (driver writes here) */
    float32                dutyCycles[NUM_OF_CHANNELS];               /* Cached duties in percent */
    float32                phases[NUM_OF_CHANNELS];                   /* Cached phases in percent [0..100) */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];                /* Cached dead times (seconds) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ==========================================================
 * Interrupt service routine and callback
 * ========================================================== */
/** ISR: eGTM ATOM period interrupt (CPU0). Minimal body per best practice. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/** Period-event callback: must be empty. Do not toggle LED here. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ==========================================================
 * Public API implementation
 * ========================================================== */
/**
 * Initialize a 3-phase complementary PWM using the unified eGTM PWM driver on ATOM in cluster 0.
 * - Three logical channels (0..2) are configured as complementary pairs (HS active-high, LS active-low).
 * - Center-aligned mode, 20 kHz, syncStart + syncUpdate enabled, ATOM clock CLK0, DTM clock 0.
 * - Dead-time: 1 us rising and falling for each pair.
 * - Period ISR priority 20 on CPU0; ISR toggles LED P03.9 (callback is empty).
 * - Pin routing is handled by the driver via OutputConfig array.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configurations: complementary pairs, HS active-high, LS active-low, push-pull */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;    /* HS: active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;     /* LS: active LOW  */
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

    /* 4) Dead-time configurations: 1 us rising and falling for each pair */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration: pulse notify, CPU0, priority 20, period-event callback, no duty-event */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations (logical indices 0..2) */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* base channel only */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main configuration fields */
    config.cluster              = IfxEgtm_Cluster_0;                         /* eGTM Cluster 0 */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;                /* ATOM submodule */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;              /* Center-aligned */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                              /* Hz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                         /* ATOM CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;         /* DTM clock 0 */
    config.syncUpdateEnabled    = TRUE;
    config.syncStart            = TRUE;

    /* 8) Enable guard: enable eGTM and configure CMU dynamically (FXCLK + CLK0) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);                                     /* unity divider */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);                   /* CLK0 = freq */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM: programs channels, routes pins, applies duties/phases/dead-times */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Cache initial duties, phases, and dead-times in module state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO (used by ISR) as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
