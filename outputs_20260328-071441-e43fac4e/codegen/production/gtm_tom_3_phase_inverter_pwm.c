/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-phase complementary inverter PWM using unified IfxGtm_Pwm
 *
 * Notes:
 * - No watchdog disable here (must be in CpuX_Main.c only)
 * - No STM timing logic here (the scheduler belongs to CpuX_Main.c)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ========================= Macros and Constants ========================= */

/* Channel and timing configuration */
#define NUM_OF_CHANNELS            (3U)           /* 3 complementary pairs: U, V, W */
#define PWM_FREQUENCY_HZ           (20000.0f)     /* 20 kHz */
#define PHASE_U_DUTY_INIT          (25.0f)        /* percent */
#define PHASE_V_DUTY_INIT          (50.0f)        /* percent */
#define PHASE_W_DUTY_INIT          (75.0f)        /* percent */
#define PHASE_DUTY_STEP            (1.0f)         /* percent per update call */

/* LED for ISR debug toggle (port, pin as a compound macro) */
#define LED                        &MODULE_P13, 0

/* ISR priority */
#define ISR_PRIORITY_ATOM          (3)

/* Validated pin symbols (TOM1 on P00.x as requested) */
#define PHASE_U_HS                 (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)  /* U high: TOM1 CH2 -> P00.3 */
#define PHASE_U_LS                 (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)  /* U low : TOM1 CH1 -> P00.2 */
#define PHASE_V_HS                 (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)  /* V high: TOM1 CH4 -> P00.5 */
#define PHASE_V_LS                 (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)  /* V low : TOM1 CH3 -> P00.4 */
#define PHASE_W_HS                 (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)  /* W high: TOM1 CH6 -> P00.7 */
#define PHASE_W_LS                 (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)  /* W low : TOM1 CH5 -> P00.6 */

/* ========================= Local Types and State ========================= */

typedef struct
{
    IfxGtm_Pwm                 pwm;                               /* unified PWM handle */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];         /* persistent channels array */
    float32                    dutyCycles[NUM_OF_CHANNELS];       /* phase U,V,W duties in percent */
    float32                    phases[NUM_OF_CHANNELS];           /* optional phase offsets (deg or %) */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];        /* applied dead-times per phase */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3phPwmState;

/* ========================= ISR and Callback (file scope) ========================= */

/* Period-event callback for unified PWM driver (empty by design) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Interrupt service routine: minimal body as required */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API ========================= */

/**
 * Initialize GTM TOM for 3-phase complementary PWM using unified IfxGtm_Pwm.
 * - Center-aligned, complementary outputs with dead-time and min pulse constraints
 * - TOM1 pairs: U(H/L)=CH2/P00.3 & CH1/P00.2, V(H/L)=CH4/P00.5 & CH3/P00.4, W(H/L)=CH6/P00.7 & CH5/P00.6
 * - FXCLK0 as time base, 20 kHz, sync start and sync update enabled
 */
void initGtmTomPwm(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config              config;
    IfxGtm_Pwm_ChannelConfig       channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig        output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig           dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig     irqCfg;

    /* 2) Load defaults into main config */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Alignment and submodule selection */
    config.cluster            = IfxGtm_Cluster_1;                   /* TOM1 cluster */
    config.subModule          = IfxGtm_Pwm_SubModule_tom;           /* use TOM */
    config.alignment          = IfxGtm_Pwm_Alignment_center;        /* center-aligned */

    /* Synchronous start and shadow transfer updates */
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;

    /* Clocking: request FXCLK0 as the PWM time base and DTM CMU clock0 */
    config.clockSource        = IfxGtm_Pwm_ClockSource_fxclk0;      /* FXCLK0 as PWM time base */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;   /* DTM uses CMU CLK0 */

    /* Frequency */
    config.frequency          = PWM_FREQUENCY_HZ;

    /* 3) Per-channel output routing and complementary polarity */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;       /* HS active high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;        /* LS active low */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (seconds) and min pulse constraint note */
    /* Use 0.5 us rising/falling dead-time for all complementary pairs */
    dtmConfig[0].deadTime.rising = 0.5e-6f;  dtmConfig[0].deadTime.falling = 0.5e-6f;
    dtmConfig[1].deadTime.rising = 0.5e-6f;  dtmConfig[1].deadTime.falling = 0.5e-6f;
    dtmConfig[2].deadTime.rising = 0.5e-6f;  dtmConfig[2].deadTime.falling = 0.5e-6f;

    /* 5) Interrupt configuration (base channel only) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2) */
    /* Phase U (channel 0) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;  /* logical index */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;          /* percent */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;                    /* base interrupt on ch0 */

    /* Phase V (channel 1) */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Phase W (channel 2) */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Main config channels linkage */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];

    /* 7) Enable-guard: bring GTM and CMU clocks up only if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize the unified PWM driver with persistent state */
    IfxGtm_Pwm_init(&g_gtmTom3phPwmState.pwm, &g_gtmTom3phPwmState.channels[0], &config);

    /* 9) Store initial duties and dead-times into module state (no update call here) */
    g_gtmTom3phPwmState.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3phPwmState.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3phPwmState.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtmTom3phPwmState.phases[0] = channelConfig[0].phase;
    g_gtmTom3phPwmState.phases[1] = channelConfig[1].phase;
    g_gtmTom3phPwmState.phases[2] = channelConfig[2].phase;

    g_gtmTom3phPwmState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phPwmState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phPwmState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Optionally apply frequency immediately (shadow-gated) */
    IfxGtm_Pwm_updateFrequencyImmediate(&g_gtmTom3phPwmState.pwm, PWM_FREQUENCY_HZ);

    /* 11) Configure LED pin for ISR debug after PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Atomically ramp/update the 3-phase PWM duty cycles under shadow-transfer gating.
 * Duty values are kept in percent [0..100].
 * Wrap rule (per phase): if (duty + step) >= 100 then duty = 0; duty += step; (always add)
 */
void updateGtmTomPwmDutyCycles(void)
{
    /* Wrap and increment for each phase (explicit, not in a loop) */
    if ((g_gtmTom3phPwmState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phPwmState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phPwmState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phPwmState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phPwmState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phPwmState.dutyCycles[2] = 0.0f; }

    g_gtmTom3phPwmState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phPwmState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phPwmState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Unified bulk immediate update (applies to all 3 complementary pairs atomically) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phPwmState.pwm, (float32 *)g_gtmTom3phPwmState.dutyCycles);
}
