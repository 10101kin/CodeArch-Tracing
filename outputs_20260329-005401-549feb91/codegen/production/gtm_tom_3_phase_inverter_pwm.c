/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief GTM TOM unified PWM driver for three-phase inverter (TC3xx)
 *
 * This module configures the GTM unified IfxGtm_Pwm driver for TOM to generate
 * three complementary, center-aligned PWM pairs with 0.5 us dead-time and
 * 1.0 us minimum pulse target. Timebase is TOM1 with a 20 kHz center-aligned
 * operation derived from FXCLK0.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Constants ========================= */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY_HZ           (20000.0f)           /* 20 kHz */
#define DEAD_TIME_S                (0.5e-6f)            /* 0.5 us in seconds */
#define MIN_PULSE_S                (1.0e-6f)            /* 1.0 us in seconds */
#define ISR_PRIORITY_ATOM          (3)

/* LED debug pin (port, pin as macro-arguments) */
#define LED                        &MODULE_P13, 0

/*
 * User-requested pin assignments (TOM1):
 *  U HS = CH2 -> P00.3 (TOUT12), U LS = CH1 -> P00.2 (TOUT11)
 *  V HS = CH4 -> P00.5 (TOUT14), V LS = CH3 -> P00.4 (TOUT13)
 *  W HS = CH6 -> P00.7 (TOUT16), W LS = CH5 -> P00.6 (TOUT15)
 *
 * Pin symbol resolution policy:
 *  - No validated pin symbols were provided in the current template context.
 *  - Use NULL_PTR placeholders and provide the exact symbol names to be
 *    substituted during project integration.
 */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUT12_P00_3_OUT */
#define PHASE_U_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUT11_P00_2_OUT */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUT14_P00_5_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUT13_P00_4_OUT */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_6_TOUT16_P00_7_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUT15_P00_6_OUT */

/* ========================= Local ISR and Callback ========================= */
/* ISR must be declared with IFX_INTERRUPT and toggle LED only */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: empty body by design */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Module State ========================= */
typedef struct
{
    IfxGtm_Pwm              pwm;                              /* PWM driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];        /* persistent channels array */
    float32                 dutyCycles[NUM_OF_CHANNELS];
    float32                 phases[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];
    float32                 minPulse;                         /* stored as seconds */
} GtmTom3PhPwm_State;

IFX_STATIC GtmTom3PhPwm_State g_gtmTom3phPwm;

/* ========================= Public API ========================= */
/**
 * Initialize the GTM unified PWM for three complementary pairs on TOM with:
 *  - 20 kHz center-aligned timebase (FXCLK0)
 *  - Dead-time = 0.5 us (rising and falling)
 *  - Minimum pulse target = 1.0 us (stored in state)
 *  - Initial duties: U=25%, V=50%, W=75%
 *  - Synchronized start enabled
 */
void initGtmTomPwm(void)
{
    /* 1) Persistent state already declared above (g_gtmTom3phPwm) */

    /* 2) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* Initialize defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Configure main PWM settings (done after channel/output config below) */

    /* 4) Output routing: map requested TOUT pins (complementary pair per channel) */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) Dead-time and min-pulse configuration data (dead-time in seconds) */
    dtmConfig[0].deadTime.rising = DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = DEAD_TIME_S;
    dtmConfig[1].deadTime.rising = DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = DEAD_TIME_S;
    dtmConfig[2].deadTime.rising = DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = DEAD_TIME_S;

    /* Store min pulse target in state for future use */
    g_gtmTom3phPwm.minPulse = MIN_PULSE_S;

    /* 6) Interrupt configuration (base channel assignment) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;  /* empty callback */
    irqCfg.dutyEvent   = NULL_PTR;

    /* 7) Channel configuration: logical indices 0..(N-1) map to U, V, W */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = 25.0f;                 /* U = 25% */
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;               /* base channel interrupt */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = 50.0f;                 /* V = 50% */
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = 75.0f;                 /* W = 75% */
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 8) Main config fields */
    config.subModule         = IfxGtm_Pwm_SubModule_tom;
    config.alignment         = IfxGtm_Pwm_Alignment_center;
    config.syncStart         = TRUE;
    config.syncUpdateEnabled = TRUE;
    config.numChannels       = (uint8)NUM_OF_CHANNELS;
    config.channels          = &channelConfig[0];
    config.frequency         = PWM_FREQUENCY_HZ;
    /* Clock sources: TOM from FXCLK0, DTM from CMU Clock0 */
    config.clockSource.tom   = IfxGtm_Cmu_Fxclk_0;      /* FXCLK0 */
    config.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 9) Enable guard for GTM + CMU (MANDATORY block) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 10) Initialize PWM (applies pin mux, alignment, timebase, shadow regs) */
    IfxGtm_Pwm_init(&g_gtmTom3phPwm.pwm, &g_gtmTom3phPwm.channels[0], &config);

    /* 11) Persist initial duty/phase/dead-time into module state */
    g_gtmTom3phPwm.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phPwm.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phPwm.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phPwm.phases[0] = channelConfig[0].phase;
    g_gtmTom3phPwm.phases[1] = channelConfig[1].phase;
    g_gtmTom3phPwm.phases[2] = channelConfig[2].phase;

    g_gtmTom3phPwm.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phPwm.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phPwm.deadTimes[2] = dtmConfig[2].deadTime;

    /* 12) Immediately program dead-time for all channels (safety before start) */
    {
        float32 dtRise[NUM_OF_CHANNELS] = { DEAD_TIME_S, DEAD_TIME_S, DEAD_TIME_S };
        float32 dtFall[NUM_OF_CHANNELS] = { DEAD_TIME_S, DEAD_TIME_S, DEAD_TIME_S };
        IfxGtm_Pwm_updateChannelsDeadTimeImmediate(&g_gtmTom3phPwm.pwm, dtRise, dtFall);
    }

    /* 13) Ensure the base frequency is exactly 20 kHz (immediate) */
    IfxGtm_Pwm_updateFrequencyImmediate(&g_gtmTom3phPwm.pwm, PWM_FREQUENCY_HZ);

    /* 14) Program initial duties using immediate multi-channel update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phPwm.pwm, (float32*)g_gtmTom3phPwm.dutyCycles);

    /* 15) Configure LED GPIO after PWM init (debug ISR toggle) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* Synchronized start is enabled via config.syncStart = TRUE. */
}
