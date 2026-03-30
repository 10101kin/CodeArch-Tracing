/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for TC4xx eGTM ATOM 3-phase complementary PWM using IfxEgtm_Pwm.
 *
 * Notes:
 * - No watchdog disable here (CpuX_Main.c only per architecture).
 * - Follows iLLD init patterns and unified PWM driver usage.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (configuration values) ========================= */
#define NUM_OF_CHANNELS          (3u)
#define PWM_FREQUENCY            (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM        (20)

#define PHASE_U_DUTY             (25.0f)
#define PHASE_V_DUTY             (50.0f)
#define PHASE_W_DUTY             (75.0f)
#define PHASE_DUTY_STEP          (10.0f)

/* LED pin: P03.9 (compound macro: port, pin) */
#define LED                      &MODULE_P03, 9

/* ========================= Pin routing macros ========================= */
/*
 * Pin routing placeholders. Use ONLY validated symbols from IfxEgtm_PinMap.h.
 * The user requested pins (P20.8/TOUT64, P20.9/TOUT65, P21.4/TOUT55, P20.11/TOUT67, P20.12/TOUT68, P20.13/TOUT69)
 * must be verified for TC4D7 in the eGTM ATOM0 Cluster 0 mapping.
 *
 * Validated list (excerpt) includes examples like:
 *   &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
 *   &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT
 *   &IfxEgtm_ATOM0_0N_TOUT54_P21_3_OUT
 *   ...
 *
 * Replace NULL_PTR below with the exact validated symbols for your board once confirmed.
 */
#define PHASE_U_HS   (NULL_PTR) /* Replace with validated HS pin, e.g., &IfxEgtm_ATOMx_y_TOUT64_P20_8_OUT */
#define PHASE_U_LS   (NULL_PTR) /* Example available: &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT if it matches your channel */
#define PHASE_V_HS   (NULL_PTR) /* Replace with validated HS pin, e.g., &IfxEgtm_ATOMx_y_TOUT55_P21_4_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Replace with validated LS pin, e.g., &IfxEgtm_ATOMx_y_TOUT67_P20_11_OUT */
#define PHASE_W_HS   (NULL_PTR) /* Replace with validated HS pin, e.g., &IfxEgtm_ATOMx_y_TOUT68_P20_12_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Replace with validated LS pin, e.g., &IfxEgtm_ATOMx_y_TOUT69_P20_13_OUT */

/* ========================= Module state ========================= */
typedef struct
{
    IfxEgtm_Pwm              pwm;                                /* Driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];          /* Persistent channels (must be static) */
    float32                  dutyCycles[NUM_OF_CHANNELS];        /* Duty in percent */
    float32                  phases[NUM_OF_CHANNELS];            /* Phase (0..1), configured as 0.0 */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];         /* Dead-time cache */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback ========================= */
/* ISR for ATOM period event – toggles LED only (minimal ISR as per best practices) */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period callback used by PWM driver – MUST be empty (no LED toggle here) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Initialization ========================= */
/*
 * Initialize a 3-phase complementary PWM on eGTM ATOM (Cluster 0) using IfxEgtm_Pwm.
 * - 3 complementary pairs
 * - Center-aligned, 20 kHz
 * - Dead-time: 1 us rising and falling
 * - Initial duties: U=25%, V=50%, W=75%
 * - Sync start and sync update enabled
 * - ATOM clock source: CLK0, DTM clock source: CMU Clock 0
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local config structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration (complementary pairs) */
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

    /* 4) Dead-time configuration: 1 us rising, 1 us falling */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration (base channel only) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2) */
    /* CH0 → Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;  /* Base channel IRQ */

    /* CH1 → Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;          /* No IRQ on follower channels */

    /* CH2 → Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main configuration */
    config.cluster           = IfxEgtm_Cluster_0;
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;
    config.alignment         = IfxEgtm_Pwm_Alignment_center;
    config.numChannels       = (uint8)NUM_OF_CHANNELS;
    config.channels          = channelConfig;
    config.frequency         = PWM_FREQUENCY;
    config.clockSource.atom  = (uint32)IfxEgtm_Cmu_Clk_0;         /* ATOM uses CLK0 */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock index 0 */
    config.syncUpdateEnabled = TRUE;
    config.syncStart         = TRUE;

    /* 8) Enable guard: eGTM enable and CMU clock setup */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver (programs channels, routes pins, applies duty/phase/dead-time) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Cache initial state in module-state struct */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure the LED GPIO pin (used by ISR) as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}
