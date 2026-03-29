/*
 * GTM TOM 3-Phase Inverter PWM driver (unified IfxGtm_Pwm)
 * - TC3xx family, TOM submodule, center-aligned 20 kHz
 * - 3 complementary pairs with 0.5 us dead-time and 1.0 us min pulse
 * - Pin mapping per user requirement (TOM1 CH[1..6] on P00.[2..7])
 *
 * Notes:
 * - Watchdog handling must NOT be added here (only in Cpu0_Main.c per AURIX pattern)
 * - Uses enable-guard pattern for GTM/CMU
 * - Follows unified IfxGtm_Pwm configuration sequence from iLLD
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =========================== Macros (config values) =========================== */
#define NUM_OF_CHANNELS                 (3u)
#define PWM_FREQUENCY_HZ                (20000.0f)       /* 20 kHz */
#define DEAD_TIME_SEC                   (0.5e-6f)        /* 0.5 us in seconds */
#define MIN_PULSE_SEC                   (1.0e-6f)        /* 1.0 us in seconds */

#define PHASE_U_DUTY                    (25.0f)          /* percent */
#define PHASE_V_DUTY                    (50.0f)          /* percent */
#define PHASE_W_DUTY                    (75.0f)          /* percent */

/* Interrupt priority for GTM PWM (TOM/ATOM unified driver) */
#define ISR_PRIORITY_ATOM               (3u)

/* LED pin for ISR scope toggle (port, pin compound) */
#define LED                             &MODULE_P13, 0

/* =========================== Validated TOUT pin symbols =========================== */
/* User-requested routing: U: CH2->P00.3 (TOUT12), CH1->P00.2 (TOUT11)
 *                         V: CH4->P00.5 (TOUT14), CH3->P00.4 (TOUT13)
 *                         W: CH6->P00.7 (TOUT16), CH5->P00.6 (TOUT15)
 */
#define PHASE_U_HS                      (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS                      (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS                      (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS                      (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS                      (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS                      (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* =========================== Module state =========================== */
typedef struct
{
    IfxGtm_Pwm              pwm;                              /* unified PWM driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];        /* persistent channel handles (owned by driver) */
    float32                 dutyCycles[NUM_OF_CHANNELS];      /* duty in percent (0..100) per sync index */
    float32                 phases[NUM_OF_CHANNELS];          /* phase offsets in degrees or percent scale as per driver */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];       /* per-channel dead-time settings (s) */
    float32                 minPulse;                          /* minimum on-time (s) */
} Tom3ph_State;

IFX_STATIC Tom3ph_State g_tom3ph;

/* =========================== ISR and callback =========================== */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback (assigned via InterruptConfig). Must be visible and empty. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =========================== Public API =========================== */
void initGtmTomPwm(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Initialize config defaults from iLLD */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Configure main PWM behavior (center-aligned, TOM, FXCLK0, sync start, 20 kHz) */
    config.subModule         = IfxGtm_Pwm_SubModule_tom;
    config.alignment         = IfxGtm_Pwm_Alignment_center;
    config.clockSource.tom   = IfxGtm_Cmu_Fxclk_0;                 /* FXCLK0 for TOM */
    config.syncStart         = TRUE;                                /* start all in sync */
    config.syncUpdateEnabled = TRUE;                                /* synchronized shadow updates */
    config.numChannels       = (uint8)NUM_OF_CHANNELS;
    config.channels          = &channelConfig[0];
    config.frequency         = PWM_FREQUENCY_HZ;                    /* 20 kHz */
    config.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0;    /* DTM clock source */
    /* Cluster selection for TOM1 (cluster indices are device specific; TOM1 typically maps to Cluster_1) */
    config.cluster           = IfxGtm_Cluster_1;

    /* 4) Output routing and polarity for complementary pairs */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;   /* High-side */
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;   /* Low-side  */
    output[0].polarity               = Ifx_ActiveState_high;              /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;               /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 5) Dead-time and minimum pulse preparation */
    dtmConfig[0].deadTime.rising = DEAD_TIME_SEC;
    dtmConfig[0].deadTime.falling = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.rising = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.falling = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.rising = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.falling = DEAD_TIME_SEC;

    g_tom3ph.deadTimes[0].rising = DEAD_TIME_SEC; g_tom3ph.deadTimes[0].falling = DEAD_TIME_SEC;
    g_tom3ph.deadTimes[1].rising = DEAD_TIME_SEC; g_tom3ph.deadTimes[1].falling = DEAD_TIME_SEC;
    g_tom3ph.deadTimes[2].rising = DEAD_TIME_SEC; g_tom3ph.deadTimes[2].falling = DEAD_TIME_SEC;
    g_tom3ph.minPulse = MIN_PULSE_SEC;

    /* Interrupt (period event) configuration for base channel */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..N-1; base/timebase at Ch_0) */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;  /* timebase channel */
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;                   /* base channel drives ISR */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Enable guard: enable GTM and configure CMU clocks only if needed */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize the unified PWM driver */
    IfxGtm_Pwm_init(&g_tom3ph.pwm, &g_tom3ph.channels[0], &config);

    /* Store initial state (duties and phases) after init */
    g_tom3ph.dutyCycles[0] = PHASE_U_DUTY;
    g_tom3ph.dutyCycles[1] = PHASE_V_DUTY;
    g_tom3ph.dutyCycles[2] = PHASE_W_DUTY;
    g_tom3ph.phases[0] = 0.0f; g_tom3ph.phases[1] = 0.0f; g_tom3ph.phases[2] = 0.0f;

    /* 9) Immediately enforce dead-time before outputs start and set frequency */
    {
        float32 dtRising[NUM_OF_CHANNELS] = { DEAD_TIME_SEC, DEAD_TIME_SEC, DEAD_TIME_SEC };
        float32 dtFalling[NUM_OF_CHANNELS] = { DEAD_TIME_SEC, DEAD_TIME_SEC, DEAD_TIME_SEC };
        IfxGtm_Pwm_updateChannelsDeadTimeImmediate(&g_tom3ph.pwm, dtRising, dtFalling);
        IfxGtm_Pwm_updateFrequencyImmediate(&g_tom3ph.pwm, PWM_FREQUENCY_HZ);
    }

    /* 10) Queue initial duties using non-immediate multi-channel update (shadow transfer) */
    IfxGtm_Pwm_updateChannelsDuty(&g_tom3ph.pwm, (float32*)g_tom3ph.dutyCycles);

    /* 11) Start synchronized outputs (lockstep on configured timebase) */
    IfxGtm_Pwm_startSyncedChannels(&g_tom3ph.pwm);

    /* Debug LED as output (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}
