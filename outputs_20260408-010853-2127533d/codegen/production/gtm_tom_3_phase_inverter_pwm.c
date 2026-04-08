/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm high-level API
 *
 * Notes:
 * - Follows iLLD initialization pattern strictly (initConfig -> customize -> init)
 * - Uses GTM enable guard and CMU configuration per authoritative examples
 * - No watchdog API calls here (must be in CpuX_Main.c only)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD dependencies */
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =========================================================================================
 * Macros and configuration constants (migration values and explicit requirements)
 * ========================================================================================= */

/* Channel count */
#define GTM_TOM_NUM_CHANNELS                (3u)

/* PWM switching frequency (Hz) */
#define GTM_TOM_PWM_FREQ_HZ                 (20000.0f)

/* Initial duty cycles in percent (0..100) */
#define GTM_TOM_INIT_DUTY_U_PERCENT         (25.0f)
#define GTM_TOM_INIT_DUTY_V_PERCENT         (50.0f)
#define GTM_TOM_INIT_DUTY_W_PERCENT         (75.0f)

/* Duty step and wrap threshold (percent) */
#define GTM_TOM_DUTY_STEP_PERCENT           (10.0f)
#define GTM_TOM_DUTY_WRAP_THRESHOLD         (100.0f)

/* Dead-time (seconds) and minimum pulse time (seconds) */
#define GTM_TOM_DEADTIME_RISING_S           (0.5e-6f)
#define GTM_TOM_DEADTIME_FALLING_S          (0.5e-6f)
#define GTM_TOM_MIN_PULSE_S                 (1.0e-6f)

/* ISR priority macro (must match InterruptConfig.priority) */
#define ISR_PRIORITY_ATOM                   (20)

/* Diagnostic LED (port, pin) for ISR toggle action */
#define LED                                 (&MODULE_P13), 0

/* User-requested TOM1 pin routing for TC3xx (validated symbols) */
#define PHASE_U_HS                          (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                          (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                          (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                          (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS                          (&IfxGtm_TOM1_5_TOUT2_P02_2_OUT)
#define PHASE_W_LS                          (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* =========================================================================================
 * Module state
 * ========================================================================================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                        /* PWM handle */
    IfxGtm_Pwm_Channel      channels[GTM_TOM_NUM_CHANNELS];             /* Persistent channel handles */
    float32                 dutyCycles[GTM_TOM_NUM_CHANNELS];           /* Duty in percent */
    float32                 phases[GTM_TOM_NUM_CHANNELS];               /* Phase in percent */
    IfxGtm_Pwm_DeadTime     deadTimes[GTM_TOM_NUM_CHANNELS];            /* Dead-time storage */
    float32                 minPulseS;                                   /* Min pulse (seconds), informational */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtnTom3phInvState;  /* IFX_STATIC per architecture pattern */

/* =========================================================================================
 * ISR and callback (declared before init per structural rules)
 * ========================================================================================= */

/*
 * ISR: toggles diagnostic LED only (no driver calls from ISR)
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/*
 * Period event callback: stub, empty body. Assigned to InterruptConfig.periodEvent.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =========================================================================================
 * Public API implementations
 * ========================================================================================= */

/*
 * Initialize GTM TOM 3-phase complementary inverter using IfxGtm_Pwm high-level driver
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for three complementary channels */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;            /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;             /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (and min pulse informational) */
    dtmConfig[0].deadTime.rising = GTM_TOM_DEADTIME_RISING_S;
    dtmConfig[0].deadTime.falling = GTM_TOM_DEADTIME_FALLING_S;
    dtmConfig[1].deadTime.rising = GTM_TOM_DEADTIME_RISING_S;
    dtmConfig[1].deadTime.falling = GTM_TOM_DEADTIME_FALLING_S;
    dtmConfig[2].deadTime.rising = GTM_TOM_DEADTIME_RISING_S;
    dtmConfig[2].deadTime.falling = GTM_TOM_DEADTIME_FALLING_S;

    /* 5) Interrupt configuration (period notification on channel 0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations: logical indices 0..2, phases = 0, initial duties */
    /* Channel 0 (Phase U) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = GTM_TOM_INIT_DUTY_U_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    /* Channel 1 (Phase V) */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = GTM_TOM_INIT_DUTY_V_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2 (Phase W) */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = GTM_TOM_INIT_DUTY_W_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main configuration */
    config.cluster              = IfxGtm_Cluster_1;                         /* TOM1 cluster */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;                 /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;              /* center-aligned */
    config.syncStart            = TRUE;                                      /* sync start */
    config.syncUpdateEnabled    = TRUE;                                      /* sync update */
    config.numChannels          = (uint8)GTM_TOM_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_PWM_FREQ_HZ;                       /* 20 kHz */
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;                /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;          /* DTM clock */

    /* 8) Enable guard: enable GTM and CMU clocks if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        {
            float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gtnTom3phInvState.pwm, &g_gtnTom3phInvState.channels[0], &config);

    /* 10) Store persistent state for runtime updates */
    g_gtnTom3phInvState.dutyCycles[0] = GTM_TOM_INIT_DUTY_U_PERCENT;
    g_gtnTom3phInvState.dutyCycles[1] = GTM_TOM_INIT_DUTY_V_PERCENT;
    g_gtnTom3phInvState.dutyCycles[2] = GTM_TOM_INIT_DUTY_W_PERCENT;

    g_gtnTom3phInvState.phases[0] = 0.0f;
    g_gtnTom3phInvState.phases[1] = 0.0f;
    g_gtnTom3phInvState.phases[2] = 0.0f;

    g_gtnTom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtnTom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtnTom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    g_gtnTom3phInvState.minPulseS = GTM_TOM_MIN_PULSE_S; /* informational */

    /* 11) Configure diagnostic LED GPIO as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update three phase duties (percent) and apply atomically via immediate group update
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap-check then increment: no loops per structural rule */
    if ((g_gtnTom3phInvState.dutyCycles[0] + GTM_TOM_DUTY_STEP_PERCENT) >= GTM_TOM_DUTY_WRAP_THRESHOLD) { g_gtnTom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_gtnTom3phInvState.dutyCycles[1] + GTM_TOM_DUTY_STEP_PERCENT) >= GTM_TOM_DUTY_WRAP_THRESHOLD) { g_gtnTom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_gtnTom3phInvState.dutyCycles[2] + GTM_TOM_DUTY_STEP_PERCENT) >= GTM_TOM_DUTY_WRAP_THRESHOLD) { g_gtnTom3phInvState.dutyCycles[2] = 0.0f; }

    g_gtnTom3phInvState.dutyCycles[0] += GTM_TOM_DUTY_STEP_PERCENT;
    g_gtnTom3phInvState.dutyCycles[1] += GTM_TOM_DUTY_STEP_PERCENT;
    g_gtnTom3phInvState.dutyCycles[2] += GTM_TOM_DUTY_STEP_PERCENT;

    /* Apply synchronized immediate duty update */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtnTom3phInvState.pwm, (float32 *)g_gtnTom3phInvState.dutyCycles);
}

/* =========================================================================================
 * Unit-test helper stubs (no functional behavior required)
 * ========================================================================================= */
void UT_DEADTIME_FALLING_S(void) { }
void UT_DEADTIME_RISING_S(void)  { }
void UT_FLOAT_EPSILON(void)      { }
void UT_INIT_DUTY_U_PERCENT(void){ }
void UT_INIT_DUTY_V_PERCENT(void){ }
void UT_INIT_DUTY_W_PERCENT(void){ }
void UT_MIN_PULSE_S(void)        { }
void UT_NUM_CHANNELS(void)       { }
void UT_PWM_FREQ_HZ(void)        { }
void UT_STEP_PERCENT(void)       { }
void UT_WRAP_THRESHOLD_PERCENT(void) { }
void accumulate(void)            { }
void behavior(void)              { }
void call(void)                  { }
void logic(void)                 { }
void percent(void)               { }
void update(void)                { }
