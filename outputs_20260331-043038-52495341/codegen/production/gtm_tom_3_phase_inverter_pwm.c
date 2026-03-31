/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for a 3-phase complementary PWM using IfxGtm_Pwm (TOM sub-module).
 *
 * - Persistent driver handle and channels are stored in module state
 * - Center-aligned, synchronized start/update, FXCLK0 clock source
 * - Complementary outputs with dead-time insertion
 * - Period interrupt routed by driver; ISR toggles a debug LED
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define NUM_OF_CHANNELS           (3u)
#define PWM_FREQUENCY_HZ          (20000.0f)      /* 20 kHz */
#define ISR_PRIORITY_ATOM         (3)

/* Initial duties (percent) and step */
#define PHASE_U_DUTY_INIT         (25.0f)
#define PHASE_V_DUTY_INIT         (50.0f)
#define PHASE_W_DUTY_INIT         (75.0f)
#define PHASE_DUTY_STEP           (10.0f)

/* LED pin (compound macro used as two arguments) */
#define LED                       &MODULE_P13, 0

/* GTM TOM1 complementary output pin selections (user-specified) */
#define PHASE_U_HS                (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)  /* U high-side:  TOM1 CH2 -> P00.3 */
#define PHASE_U_LS                (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)  /* U low-side:   TOM1 CH1 -> P00.2 */
#define PHASE_V_HS                (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)  /* V high-side:  TOM1 CH4 -> P00.5 */
#define PHASE_V_LS                (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)  /* V low-side:   TOM1 CH3 -> P00.4 */
#define PHASE_W_HS                (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)  /* W high-side:  TOM1 CH6 -> P00.7 */
#define PHASE_W_LS                (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)  /* W low-side:   TOM1 CH5 -> P00.6 */

/* =============================
 * Module State
 * ============================= */
typedef struct
{
    IfxGtm_Pwm               pwm;                                 /* unified PWM handle */
    IfxGtm_Pwm_Channel       channels[NUM_OF_CHANNELS];           /* persistent channels (driver stores pointers) */
    float32                  dutyCycles[NUM_OF_CHANNELS];         /* percent */
    float32                  phases[NUM_OF_CHANNELS];             /* degrees or percent of period (as configured) */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];          /* dead-time per complementary pair */
} GtmTom3Ph_State;

IFX_STATIC GtmTom3Ph_State g_gtmTom3Ph = {0};

/* =============================
 * ISR and Callback Declarations
 * ============================= */
extern void IfxGtm_periodEventFunction(void *data);  /* forward - must be visible */

/* ISR: declared with vector macro and minimal body (toggle LED) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback: empty body (assigned via InterruptConfig) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Public API Implementations
 * ============================= */

/**
 * Initialize a persistent 3-channel complementary PWM on TOM using IfxGtm_Pwm.
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Populate defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: high + complementary (active-high HS, active-low LS), push-pull, cmosAutomotiveSpeed1 */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;        /* HS active-high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;         /* LS active-low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM (dead-time) configuration per complementary pair: use 1.0e-6 s (user-confirmed) */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 5) Interrupt configuration: period event, pulse notify, CPU0 provider, valid priority; ISR is declared separately */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;    /* base/time channel */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;             /* base period event on channel 0 */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

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
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;     /* TOM uses FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;  /* DTM clock source */

    /* 8) Enable-guard for GTM and CMU clocks (all inside the guard) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        {
            float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize PWM (applies initial duties atomically with sync updates) */
    IfxGtm_Pwm_init(&g_gtmTom3Ph.pwm, &g_gtmTom3Ph.channels[0], &config);

    /* 10) Store initial duties, phases, and dead-times into state */
    g_gtmTom3Ph.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3Ph.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3Ph.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtmTom3Ph.phases[0] = 0.0f;
    g_gtmTom3Ph.phases[1] = 0.0f;
    g_gtmTom3Ph.phases[2] = 0.0f;

    g_gtmTom3Ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3Ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3Ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED/debug pin after PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duties in percent and apply them immediately.
 */
void GTM_TOM_3_Phase_Inverter_PWM_updateDuties(void)
{
    /* Duty wrap rule: check against 100, reset to 0, then always add step */
    if ((g_gtmTom3Ph.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3Ph.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3Ph.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3Ph.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3Ph.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3Ph.dutyCycles[2] = 0.0f; }

    g_gtmTom3Ph.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3Ph.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3Ph.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate synchronous duty update for all channels */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3Ph.pwm, (float32*)g_gtmTom3Ph.dutyCycles);
}
