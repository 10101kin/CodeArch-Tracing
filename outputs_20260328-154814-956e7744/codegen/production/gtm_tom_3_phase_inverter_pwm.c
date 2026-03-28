/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver implementing a 3-phase complementary, center-aligned PWM
 * using the IfxGtm_Pwm unified driver on TOM1, Cluster_1 at 20 kHz with 1 us
 * dead time. Includes an ATOM ISR stub that toggles LED P13.0 and a period
 * callback with empty body.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* Required iLLD headers (selected) */
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =====================================================================================
 * Configuration macros (numeric constants from requirements)
 * ===================================================================================== */
#define NUM_OF_CHANNELS            (3u)           /* 3 complementary pairs => 3 logical channels */
#define PWM_FREQUENCY_HZ           (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM          (20)           /* ISR priority for ATOM ISR */

#define PHASE_U_DUTY_INIT          (25.0f)        /* percent */
#define PHASE_V_DUTY_INIT          (50.0f)        /* percent */
#define PHASE_W_DUTY_INIT          (75.0f)        /* percent */
#define PHASE_DUTY_STEP            (10.0f)        /* percent step */

/* LED on P13.0 for ISR toggling: compound macro expands to (port, pin) */
#define LED                         &MODULE_P13, 0

/* =====================================================================================
 * Pin routing macros (placeholders – integrator must replace with validated pin symbols)
 * Mapping per requirements (TOM1, TGC1 on P02.x):
 *   U: HS=P02.0, LS=P02.7
 *   V: HS=P02.1, LS=P02.4
 *   W: HS=P02.2, LS=P02.5
 *
 * NOTE:
 *   Use the exact IfxGtm_TOM1_*_TOUT*_P02_*_OUT symbols from the iLLD PinMap
 *   headers for your device/package. Until validated, leave as NULL_PTR so the
 *   build reminds to provide the correct symbols during integration.
 */
#define PHASE_U_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_0_OUT */
#define PHASE_U_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_7_OUT */
#define PHASE_V_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_1_OUT */
#define PHASE_V_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_4_OUT */
#define PHASE_W_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_2_OUT */
#define PHASE_W_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_5_OUT */

/* =====================================================================================
 * Module state (persistent) — MUST be static using IFX_STATIC
 * ===================================================================================== */
#ifndef IFX_STATIC
#define IFX_STATIC static
#endif

typedef struct
{
    IfxGtm_Pwm              pwm;                                  /* unified PWM driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];            /* persistent channels array */
    float32                 dutyCycles[NUM_OF_CHANNELS];           /* per-phase duties in percent */
    float32                 phases[NUM_OF_CHANNELS];               /* per-phase phase offsets (deg or %) */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];           /* per-phase dead-time (s) */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_tom3ph = {0};

/* =====================================================================================
 * Private ISR and callback declarations (must appear before init)
 * ===================================================================================== */
/* Period-event callback (empty body) used by unified PWM driver */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* empty by design */
}

/* ATOM ISR (prio 20) — minimal body: toggle LED. Not installed here. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* =====================================================================================
 * Public API implementations
 * ===================================================================================== */
/**
 * Initialize 3-phase complementary, center-aligned PWM on TOM1/Cluster_1 @ 20 kHz,
 * 1 us dead-time, with synchronous start/update. Prepares LED P13.0 for ISR toggling.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare configuration objects as locals */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Populate main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: bind pins, set polarity, output mode, pad driver */
    /* Phase U (channel 0) */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;  /* high-side active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;   /* low-side  active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (channel 1) */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (channel 2) */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration (seconds). Use 1.0e-6f as mandated literal. */
    dtmConfig[0].deadTime.rising = 1.0e-6f; dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f; dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f; dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 4) Unified PWM base interrupt (only for base channel 0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction; /* empty body */
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Per-channel configuration: TOM1 channels 0..2 (logical) mapped to U,V,W */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* base channel routes unified period callback */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Update other PWM config fields */
    config.gtmSFR               = &MODULE_GTM;
    config.cluster              = IfxGtm_Cluster_1;                      /* TGC1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;              /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;           /* center aligned */
    config.syncStart            = TRUE;                                   /* sync start */
    config.syncUpdateEnabled    = TRUE;                                   /* sync update */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.atom     = (uint32)IfxGtm_Cmu_Fxclk_0;             /* FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;      /* DTM clock */

    /* 5) GTM enable guard and CMU clock configuration (inside guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialise the unified PWM (applies pin routing, complementary, dead-time) */
    IfxGtm_Pwm_init(&g_tom3ph.pwm, &g_tom3ph.channels[0], &config);

    /* 7) Store initial duties/dead-times in module state */
    g_tom3ph.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_tom3ph.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_tom3ph.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_tom3ph.phases[0] = 0.0f;
    g_tom3ph.phases[1] = 0.0f;
    g_tom3ph.phases[2] = 0.0f;

    g_tom3ph.deadTimes[0].rising = 1.0e-6f; g_tom3ph.deadTimes[0].falling = 1.0e-6f;
    g_tom3ph.deadTimes[1].rising = 1.0e-6f; g_tom3ph.deadTimes[1].falling = 1.0e-6f;
    g_tom3ph.deadTimes[2].rising = 1.0e-6f; g_tom3ph.deadTimes[2].falling = 1.0e-6f;

    /* 8) Configure LED GPIO as push-pull output and drive LOW (known default) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(LED);
}

/**
 * Update U,V,W duty cycles with wrap rule and apply immediately to the PWM.
 * No timing/dwell performed here; higher-level scheduling handles periodicity.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 => duty = 0; then add step (always) */
    if ((g_tom3ph.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3ph.dutyCycles[0] = 0.0f; }
    if ((g_tom3ph.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3ph.dutyCycles[1] = 0.0f; }
    if ((g_tom3ph.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3ph.dutyCycles[2] = 0.0f; }

    g_tom3ph.dutyCycles[0] += PHASE_DUTY_STEP; /* U */
    g_tom3ph.dutyCycles[1] += PHASE_DUTY_STEP; /* V */
    g_tom3ph.dutyCycles[2] += PHASE_DUTY_STEP; /* W */

    /* Immediate synchronized update (array length = numChannels) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3ph.pwm, (float32*)g_tom3ph.dutyCycles);
}
