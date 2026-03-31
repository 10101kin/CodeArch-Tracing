/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production 3-phase complementary PWM driver using IfxGtm_Pwm unified API on TOM.
 *
 * Initialization strictly follows iLLD patterns:
 *  - IfxGtm_Pwm_initConfig / IfxGtm_Pwm_init sequence
 *  - OutputConfig and DtmConfig arrays per channel
 *  - InterruptConfig with periodEvent callback
 *  - GTM enable-guard with CMU clock configuration
 *
 * Watchdog: Do NOT disable any watchdog here. That belongs in Cpu0_Main.c only.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ======================== Macros (configuration constants) ======================== */
#define NUM_OF_CHANNELS                 (3u)

/* PWM switching frequency (Hz) */
#define PWM_FREQUENCY                   (20000.0f)

/* Initial duties (percent) */
#define PHASE_U_DUTY_INIT               (25.0f)
#define PHASE_V_DUTY_INIT               (50.0f)
#define PHASE_W_DUTY_INIT               (75.0f)

/* Duty increment step (percent) */
#define PHASE_DUTY_STEP                 (10.0f)

/* Dead-time and minimum pulse (seconds) — used by DTM configuration */
#define PWM_DEAD_TIME                   (1e-06f)   /* user-confirmed */
#define PWM_MIN_PULSE_TIME              (1e-06f)   /* retained as design reference */

/* ISR priority for TOM/ATOM routed PWM interrupt */
#define ISR_PRIORITY_ATOM               (3u)

/* LED/debug pin (compound macro: expands to two arguments: port, pin) */
#define LED                             &MODULE_P13, 0

/* ======================== Pin routing (validated TOUT symbols) ======================== */
/* Phase U: CH2(high)/CH1(low) -> P00.3 / P00.2 */
#define PHASE_U_HS                      (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS                      (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)

/* Phase V: CH4(high)/CH3(low) -> P00.5 / P00.4 */
#define PHASE_V_HS                      (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS                      (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)

/* Phase W: CH6(high)/CH5(low) -> P00.7 / P00.6 */
#define PHASE_W_HS                      (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS                      (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ======================== Module state ======================== */
typedef struct
{
    IfxGtm_Pwm              pwm;                                   /* driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];             /* persistent channel handles */
    float32                 dutyCycles[NUM_OF_CHANNELS];            /* percent */
    float32                 phases[NUM_OF_CHANNELS];                /* degrees or fractional 0..360 mapping (design stores 0.0) */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];            /* per-pair dead-time */
} GtmTom3PhInv_State;

IFX_STATIC GtmTom3PhInv_State g_gtmTom3PhInv;  /* persistent module state */

/* ======================== ISR and callback (declared before init) ======================== */
/* ISR declaration with priority. ISR body must only toggle LED. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback used by InterruptConfig; body intentionally empty. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ======================== Public API ======================== */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* 1) Local configuration objects (do not allocate handle/channels locally) */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   interruptConfig;

    /* 2) Populate defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Per-channel output configuration (complementary pairs) */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;          /* high-side */
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;          /* low-side  */
    output[0].polarity                = Ifx_ActiveState_high;                      /* HS active HIGH */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;                       /* LS active LOW  */
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

    /* 4) Dead-time configuration per pair */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME;

    /* 5) Interrupt configuration (period event on base channel) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical channel indices 0..2 */
    /* Base channel (index 0) with interrupt */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    /* Channel 1 */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2 */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;              /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;/* DTM clock source */

    /* 8) Enable-guard and CMU clocks (all operations inside the guard) */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM (applies initial duties atomically with sync) */
    IfxGtm_Pwm_init(&g_gtmTom3PhInv.pwm, &g_gtmTom3PhInv.channels[0], &config);

    /* 10) Store initial state for later updates */
    g_gtmTom3PhInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3PhInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3PhInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3PhInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3PhInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3PhInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3PhInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3PhInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3PhInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED/debug GPIO for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void GTM_TOM_3_Phase_Inverter_PWM_updateDuties(void)
{
    /* Duty wrap rule (separate checks; no loop): if (duty + step) >= 100 then reset to 0, then add step */
    if ((g_gtmTom3PhInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3PhInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3PhInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3PhInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3PhInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3PhInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate synchronous update of all complementary pairs */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3PhInv.pwm, (float32 *)g_gtmTom3PhInv.dutyCycles);
}
