/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief Production driver: GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm API
 *
 * Behavior:
 *  - Three complementary PWM pairs (U, V, W) on TOM1, center-aligned, 20 kHz
 *  - Dead-time: 0.5 us for all pairs; Minimum pulse: 1.0 us (stored in state)
 *  - Initial duties: U=25%%, V=50%%, W=75%%
 *  - FXCLK0 clock source, synchronized start and updates
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Pin Mapping ========================= */
#define NUM_OF_CHANNELS                (3U)
#define PWM_FREQUENCY_HZ               (20000.0f)
#define ISR_PRIORITY_ATOM              (3)

/* LED for ISR debug toggle: port, pin (compound macro argument) */
#define LED                            &MODULE_P13, 0

/* User-requested TOM1 pin routing (validated symbols from iLLD PinMap) */
#define PHASE_U_HS                     &IfxGtm_TOM1_2_TOUT12_P00_3_OUT  /* TOM1 CH2 -> P00.3 (TOUT12) */
#define PHASE_U_LS                     &IfxGtm_TOM1_1_TOUT11_P00_2_OUT  /* TOM1 CH1 -> P00.2 (TOUT11) */
#define PHASE_V_HS                     &IfxGtm_TOM1_4_TOUT14_P00_5_OUT  /* TOM1 CH4 -> P00.5 (TOUT14) */
#define PHASE_V_LS                     &IfxGtm_TOM1_3_TOUT13_P00_4_OUT  /* TOM1 CH3 -> P00.4 (TOUT13) */
#define PHASE_W_HS                     &IfxGtm_TOM1_6_TOUT16_P00_7_OUT  /* TOM1 CH6 -> P00.7 (TOUT16) */
#define PHASE_W_LS                     &IfxGtm_TOM1_5_TOUT15_P00_6_OUT  /* TOM1 CH5 -> P00.6 (TOUT15) */

/* Initial duties in percent */
#define PHASE_U_DUTY_PCT               (25.0f)
#define PHASE_V_DUTY_PCT               (50.0f)
#define PHASE_W_DUTY_PCT               (75.0f)

/* Dead-time and min-pulse (seconds) */
#define DEAD_TIME_S                    (0.5e-6f)
#define MIN_PULSE_S                    (1.0e-6f)

/* ========================= Internal State ========================= */

typedef struct
{
    IfxGtm_Pwm                 pwm;                               /* Driver handle */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];         /* Persistent channels array */
    float32                    dutyCycles[NUM_OF_CHANNELS];       /* Duty cycle percent per phase */
    float32                    phases[NUM_OF_CHANNELS];           /* Phase offsets (s) */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];        /* Dead-time settings per phase */
    float32                    minPulseS;                         /* Minimum pulse width (s) */
} GtmTom3Ph_State;

IFX_STATIC GtmTom3Ph_State g_gtmTom3Ph;

/* ========================= ISR and Callback ========================= */

/* ISR: toggle LED only (priority macro used both here and in interrupt config) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: empty by design; assigned into InterruptConfig */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */

/**
 * @brief Initialize the GTM unified PWM for three complementary pairs on TOM1.
 *        Center-aligned 20 kHz timebase, FXCLK0, 0.5 us dead-time, 1.0 us min pulse.
 */
void initGtmTomPwm(void)
{
    /* 1) Persistent module state declared above (g_gtmTom3Ph) */

    /* 2) Local configuration structures */
    IfxGtm_Pwm_Config              config;
    IfxGtm_Pwm_OutputConfig        output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig           dtmCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig     irqCfg;
    IfxGtm_Pwm_ChannelConfig       channelCfg[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Main configuration fields */
    config.cluster              = IfxGtm_Cluster_0;                        /* Use cluster 0 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;                /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;             /* Center-aligned */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;                  /* 3 complementary pairs */
    config.channels             = &channelCfg[0];
    config.frequency            = PWM_FREQUENCY_HZ;                        /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                      /* FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;        /* DTM clock */
    config.syncUpdateEnabled    = TRUE;                                     /* Shadow transfer at period */
    config.syncStart            = TRUE;                                     /* Synchronous start after init */

    /* 4) Output routing: map high-side and complementary low-side pins */
    /* U phase */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;
    output[0].complementaryPolarity   = Ifx_ActiveState_low;
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) Dead-time: 0.5 us for all pairs; store min pulse (1.0 us) in state */
    dtmCfg[0].deadTime.rising = DEAD_TIME_S;  dtmCfg[0].deadTime.falling = DEAD_TIME_S;
    dtmCfg[1].deadTime.rising = DEAD_TIME_S;  dtmCfg[1].deadTime.falling = DEAD_TIME_S;
    dtmCfg[2].deadTime.rising = DEAD_TIME_S;  dtmCfg[2].deadTime.falling = DEAD_TIME_S;

    g_gtmTom3Ph.minPulseS = MIN_PULSE_S; /* Stored for application usage if needed */

    /* Interrupt configuration (period event only; ISR toggles LED) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (uint8)ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* Channel configuration: logical indices 0..2, TOM1 timebase (CH0) inferred via pins) */
    channelCfg[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;  /* base/timebase channel */
    channelCfg[0].phase      = 0.0f;
    channelCfg[0].duty       = PHASE_U_DUTY_PCT;           /* percent */
    channelCfg[0].dtm        = &dtmCfg[0];
    channelCfg[0].output     = &output[0];
    channelCfg[0].mscOut     = NULL_PTR;
    channelCfg[0].interrupt  = &irqCfg;

    channelCfg[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelCfg[1].phase      = 0.0f;
    channelCfg[1].duty       = PHASE_V_DUTY_PCT;
    channelCfg[1].dtm        = &dtmCfg[1];
    channelCfg[1].output     = &output[1];
    channelCfg[1].mscOut     = NULL_PTR;
    channelCfg[1].interrupt  = &irqCfg;

    channelCfg[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelCfg[2].phase      = 0.0f;
    channelCfg[2].duty       = PHASE_W_DUTY_PCT;
    channelCfg[2].dtm        = &dtmCfg[2];
    channelCfg[2].output     = &output[2];
    channelCfg[2].mscOut     = NULL_PTR;
    channelCfg[2].interrupt  = &irqCfg;

    /* 6) GTM enable guard and CMU clock configuration (inside guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Initialize unified PWM (applies pin mux, alignment, timebase, shadows) */
    IfxGtm_Pwm_init(&g_gtmTom3Ph.pwm, &g_gtmTom3Ph.channels[0], &config);

    /* Store state for later runtime updates */
    g_gtmTom3Ph.dutyCycles[0] = PHASE_U_DUTY_PCT;
    g_gtmTom3Ph.dutyCycles[1] = PHASE_V_DUTY_PCT;
    g_gtmTom3Ph.dutyCycles[2] = PHASE_W_DUTY_PCT;

    g_gtmTom3Ph.phases[0] = 0.0f;
    g_gtmTom3Ph.phases[1] = 0.0f;
    g_gtmTom3Ph.phases[2] = 0.0f;

    g_gtmTom3Ph.deadTimes[0] = dtmCfg[0].deadTime;
    g_gtmTom3Ph.deadTimes[1] = dtmCfg[1].deadTime;
    g_gtmTom3Ph.deadTimes[2] = dtmCfg[2].deadTime;

    /* Debug LED after PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 8) Immediately program dead-time and base frequency */
    IfxGtm_Pwm_updateChannelsDeadTimeImmediate(&g_gtmTom3Ph.pwm, &g_gtmTom3Ph.deadTimes[0]);
    IfxGtm_Pwm_updateFrequencyImmediate(&g_gtmTom3Ph.pwm, PWM_FREQUENCY_HZ);

    /* 9) Program initial U/V/W duties using immediate multi-channel update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3Ph.pwm, (float32 *)&g_gtmTom3Ph.dutyCycles[0]);

    /* 10) PWM outputs start in sync automatically due to config.syncStart = TRUE */
}
