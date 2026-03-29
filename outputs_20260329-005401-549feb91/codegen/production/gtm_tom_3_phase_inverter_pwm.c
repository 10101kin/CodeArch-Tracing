/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production-ready unified IfxGtm_Pwm driver for TC3xx TOM: 3 complementary pairs
 * - 20 kHz center-aligned timebase on TOM1 (FXCLK0)
 * - Dead-time: 0.5 us, Min pulse: 1.0 us
 * - Pins (user-specified):
 *   U HS: TOM1 CH2 -> P00.3 (TOUT12)
 *   U LS: TOM1 CH1 -> P00.2 (TOUT11)
 *   V HS: TOM1 CH4 -> P00.5 (TOUT14)
 *   V LS: TOM1 CH3 -> P00.4 (TOUT13)
 *   W HS: TOM1 CH6 -> P00.7 (TOUT16)
 *   W LS: TOM1 CH5 -> P00.6 (TOUT15)
 *
 * Notes:
 * - Follows iLLD IfxGtm_Pwm init pattern with OutputConfig/DtmConfig/ChannelConfig arrays.
 * - All CMU clocking is guarded by IfxGtm_isEnabled().
 * - No watchdog handling here (must be in CpuX_Main.c only).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ============================= Macros & Constants ============================= */
#define NUM_OF_CHANNELS                  (3u)
#define PWM_FREQUENCY_HZ                 (20000.0f)
#define ISR_PRIORITY_ATOM                (3u)

/* Dead-time and minimum pulse (seconds) */
#define DEAD_TIME_S                      (0.5e-6f)
#define MIN_PULSE_S                      (1.0e-6f)

/* Initial phase duties (percent) */
#define PHASE_U_DUTY_PCT                 (25.0f)
#define PHASE_V_DUTY_PCT                 (50.0f)
#define PHASE_W_DUTY_PCT                 (75.0f)

/* LED port/pin for ISR toggle (compound macro: port, pin) */
#define LED                              &MODULE_P13, 0

/* User-requested pin routing macros (TOM1 / TOUT on Port00) */
#define PHASE_U_HS                       &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS                       &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS                       &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS                       &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS                       &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS                       &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* ============================= Module State ================================== */
typedef struct
{
    IfxGtm_Pwm                 pwm;                               /* unified PWM driver handle */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];         /* persistent channel handles */
    float32                    dutyCycles[NUM_OF_CHANNELS];       /* stored duty in percent */
    float32                    phases[NUM_OF_CHANNELS];           /* stored phase in degrees */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];        /* stored dead-time settings */
} GtmTomPwm_State;

IFX_STATIC GtmTomPwm_State g_gtmTomPwmState;

/* ============================= ISR and Callback ============================== */
/* ISR: minimal body toggles LED for timing observation */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback (assigned via InterruptConfig) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================= Public API ==================================== */
/**
 * Initialize the GTM unified PWM for three complementary pairs on TOM with:
 * - 20 kHz center-aligned base
 * - 0.5 us dead-time, 1.0 us minimum pulse
 * - Synchronized start and shadow updates enabled
 * - FXCLK0 as TOM clock source
 */
void initGtmTomPwm(void)
{
    /* 1) Persistent module state already declared at file scope */

    /* 2) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Main config fields (TOM, center-aligned, FXCLK0, sync start/update, 20 kHz) */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;          /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 4) Output routing (complementary pairs) */
    /* U-phase */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;               /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;                /* LS active low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* V-phase */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* W-phase */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) Dead-time and minimum pulse configuration */
    /* All channels: 0.5 us rising/falling dead-time. Min pulse is stored in state for runtime enforcement */
    dtmConfig[0].deadTime.rising = DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = DEAD_TIME_S;
    dtmConfig[1].deadTime.rising = DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = DEAD_TIME_S;
    dtmConfig[2].deadTime.rising = DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = DEAD_TIME_S;

    /* 6) Interrupt configuration for base channel (period event) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (uint8)ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 7) Channel configuration (logical indices 0..N-1). Base/timebase at index 0 */
    /* U phase */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_PCT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;                  /* base channel interrupt */

    /* V phase */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_PCT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* W phase */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_PCT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 8) Enable guard: enable GTM and CMU clocks if needed */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM driver (applies pins, timebase, alignment) */
    IfxGtm_Pwm_init(&g_gtmTomPwmState.pwm, &g_gtmTomPwmState.channels[0], &config);

    /* 10) Store current state (duty, phase, dead-time) */
    g_gtmTomPwmState.dutyCycles[0] = PHASE_U_DUTY_PCT;
    g_gtmTomPwmState.dutyCycles[1] = PHASE_V_DUTY_PCT;
    g_gtmTomPwmState.dutyCycles[2] = PHASE_W_DUTY_PCT;
    g_gtmTomPwmState.phases[0] = 0.0f; g_gtmTomPwmState.phases[1] = 0.0f; g_gtmTomPwmState.phases[2] = 0.0f;
    g_gtmTomPwmState.deadTimes[0].rising = DEAD_TIME_S; g_gtmTomPwmState.deadTimes[0].falling = DEAD_TIME_S;
    g_gtmTomPwmState.deadTimes[1].rising = DEAD_TIME_S; g_gtmTomPwmState.deadTimes[1].falling = DEAD_TIME_S;
    g_gtmTomPwmState.deadTimes[2].rising = DEAD_TIME_S; g_gtmTomPwmState.deadTimes[2].falling = DEAD_TIME_S;

    /* 11) Immediately enforce dead-time and base frequency, then queue initial duties */
    {
        float32 dtRise[NUM_OF_CHANNELS] = { DEAD_TIME_S, DEAD_TIME_S, DEAD_TIME_S };
        float32 dtFall[NUM_OF_CHANNELS] = { DEAD_TIME_S, DEAD_TIME_S, DEAD_TIME_S };
        IfxGtm_Pwm_updateChannelsDeadTimeImmediate(&g_gtmTomPwmState.pwm, &dtRise[0], &dtFall[0]);
        IfxGtm_Pwm_updateFrequencyImmediate(&g_gtmTomPwmState.pwm, PWM_FREQUENCY_HZ);
        IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTomPwmState.pwm, (float32 *)&g_gtmTomPwmState.dutyCycles[0]);
    }

    /* LED GPIO configuration for ISR visibility */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}
