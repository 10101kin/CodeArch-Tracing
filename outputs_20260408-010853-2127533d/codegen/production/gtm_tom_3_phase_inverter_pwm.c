/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm
 *
 * Notes:
 * - Follows iLLD mandatory initialization patterns for IfxGtm_Pwm
 * - No watchdog handling here (must be in CpuX_Main.c only)
 * - No STM timing/delay logic here
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =======================================================================
 * Configuration Macros (numeric values from requirements/migration)
 * ======================================================================= */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY_HZ           (20000.0f)
#define ISR_PRIORITY_ATOM          (20)

/* LED: P13.0 as diagnostic toggle pin (compound port,pin macro) */
#define LED                        &MODULE_P13, 0

/* Phase output pin mappings (user-requested, TC3xx PinMap symbols) */
#define PHASE_U_HS                 (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                 (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                 (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                 (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS                 (&IfxGtm_TOM1_5_TOUT2_P02_2_OUT)
#define PHASE_W_LS                 (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* Initial duties (percent) and step (percent) */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* Dead-time and min-pulse requirements (seconds) */
#define PWM_DEADTIME_RISING_S      (1.0e-6f) /* user requirement: 1 us rising */
#define PWM_DEADTIME_FALLING_S     (1.0e-6f) /* user requirement: 1 us falling */
#define PWM_MIN_PULSE_S            (1.0e-6f) /* cannot be set explicitly in IfxGtm_Pwm_DtmConfig */

/* =======================================================================
 * Module State
 * ======================================================================= */
#ifndef IFX_STATIC
#define IFX_STATIC static
#endif

typedef struct
{
    IfxGtm_Pwm                pwm;                              /* PWM driver handle */
    IfxGtm_Pwm_Channel        channels[NUM_OF_CHANNELS];        /* Persistent channels array */
    float32                    dutyCycles[NUM_OF_CHANNELS];      /* Duty in percent */
    float32                    phases[NUM_OF_CHANNELS];          /* Phase in degrees (or fraction) */
    IfxGtm_Pwm_DeadTime       deadTimes[NUM_OF_CHANNELS];       /* Dead-time per channel */
} GtmTom3PhInv_State;

IFX_STATIC GtmTom3PhInv_State g_gtmTom3phInv = {0};

/* =======================================================================
 * ISR and Period-Event Callback (declared before init per rules)
 * ======================================================================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* intentionally empty */
}

/* =======================================================================
 * Public API Implementations
 * ======================================================================= */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for three complementary channels */
    /* Phase U */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;  /* HS active-high */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;   /* LS active-low (complementary) */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM dead-time configuration (per channel). min-pulse is tracked in state only. */
    dtmConfig[0].deadTime.rising = PWM_DEADTIME_RISING_S;
    dtmConfig[0].deadTime.falling = PWM_DEADTIME_FALLING_S;
    dtmConfig[1].deadTime.rising = PWM_DEADTIME_RISING_S;
    dtmConfig[1].deadTime.falling = PWM_DEADTIME_FALLING_S;
    dtmConfig[2].deadTime.rising = PWM_DEADTIME_RISING_S;
    dtmConfig[2].deadTime.falling = PWM_DEADTIME_FALLING_S;

    /* 5) Base channel interrupt configuration */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;   /* period notification */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices start at 0) */
    /* Channel 0 → Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;   /* only base channel uses interrupt */

    /* Channel 1 → Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2 → Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main configuration */
    config.cluster              = IfxGtm_Cluster_1;                    /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;            /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;         /* center-aligned */
    config.syncStart            = TRUE;                                 /* synchronized start */
    config.syncUpdateEnabled    = TRUE;                                 /* synchronized update */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                     /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                   /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;   /* DTM clock source */

    /* 8) Enable guard: enable GTM and clocks only if not already enabled */
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

    /* 9) Initialize the PWM (persistent handle and channels in module state) */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 10) Store persistent runtime state for updates */
    g_gtmTom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_gtmTom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_gtmTom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure diagnostic LED GPIO as push-pull output (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateGtmTom3phInvDuty(void)
{
    /* Duty update rule: if (duty + step) >= 100 then duty = 0; then duty += step; */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply synchronized immediate update (percent units) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
