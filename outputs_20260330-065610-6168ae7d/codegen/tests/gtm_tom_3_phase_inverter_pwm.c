/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm (unified PWM).
 * - Center-aligned complementary PWM at 20 kHz
 * - 1 us rising/falling dead-time via TOM DTM
 * - Synchronous start and synchronous update enabled
 * - LED P13.0 toggled in ISR
 *
 * Notes:
 * - Followed authoritative iLLD initialization pattern (see IfxGtm_Pwm doxygen examples).
 * - No watchdog control in this file (watchdog handling belongs to CpuX_Main.c).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ====================================================================== */
/* Macros and configuration constants                                      */
/* ====================================================================== */

/* Channel count for 3 complementary pairs (U,V,W) */
#define NUM_OF_CHANNELS            (3u)

/* PWM frequency (Hz) */
#define PWM_FREQUENCY_HZ           (20000.0f)

/* Interrupt priority for GTM PWM base channel */
#define ISR_PRIORITY_ATOM          (20u)

/* Phase initial duties in PERCENT (0..100) */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)

/* Duty step in PERCENT (wrap rule applies) */
#define PHASE_DUTY_STEP            (10.0f)

/* LED pin macro (compound: port, pin) */
#define LED                        &MODULE_P13, 0

/*
 * Pin routing placeholders (TOUT mapping must be provided by integration).
 * No validated pin symbols were available in this context, so use NULL_PTR.
 * Replace NULL_PTR with proper &IfxGtm_TOM1_x_TOUTy_P02_z_OUT symbols matching:
 *   U: HS=P02.0, LS=P02.7
 *   V: HS=P02.1, LS=P02.4
 *   W: HS=P02.2, LS=P02.5
 */
#define PHASE_U_HS                 (NULL_PTR) /* -> &IfxGtm_TOM1_0_TOUTx_P02_0_OUT */
#define PHASE_U_LS                 (NULL_PTR) /* -> &IfxGtm_TOM1_1_TOUTy_P02_7_OUT */
#define PHASE_V_HS                 (NULL_PTR) /* -> &IfxGtm_TOM1_2_TOUTx_P02_1_OUT */
#define PHASE_V_LS                 (NULL_PTR) /* -> &IfxGtm_TOM1_3_TOUTy_P02_4_OUT */
#define PHASE_W_HS                 (NULL_PTR) /* -> &IfxGtm_TOM1_4_TOUTx_P02_2_OUT */
#define PHASE_W_LS                 (NULL_PTR) /* -> &IfxGtm_TOM1_5_TOUTy_P02_5_OUT */

/* ====================================================================== */
/* Module state                                                            */
/* ====================================================================== */

typedef struct
{
    IfxGtm_Pwm              pwm;                               /* PWM driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];         /* Persistent channels (owned by driver) */
    float32                 dutyCycles[NUM_OF_CHANNELS];        /* Duty in percent */
    float32                 phases[NUM_OF_CHANNELS];            /* Phase in degrees (or percent), as configured */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];         /* Dead-time per channel */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInv;

/* ====================================================================== */
/* ISR declaration and minimal body                                        */
/* ====================================================================== */

/* ISR declaration (provider CPU0, priority ISR_PRIORITY_ATOM) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/** ISR body: toggle LED and return. */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ====================================================================== */
/* Period-event callback (assigned in InterruptConfig)                     */
/* ====================================================================== */

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty: ISR toggles LED */
}

/* ====================================================================== */
/* Public API implementations                                              */
/* ====================================================================== */

/**
 * Initialize the GTM for a 3-phase inverter PWM using IfxGtm_Pwm on TOM in
 * center-aligned complementary mode at 20 kHz with 1 us dead-time.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* 2) GTM enable and CMU clocks (inside enable guard) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 3) Load safe defaults into main config */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 4) Output configuration: complementary pairs with DTM dead-time */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;            /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;             /* LS active low  */
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

    /* DTM: 1 us rising/falling dead-time on all channels */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Interrupt configuration for base channel only (channel 0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = (IfxGtm_Pwm_callBack)IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = (IfxGtm_Pwm_callBack)NULL_PTR;

    /* Per-channel configuration (logical indices 0..2) */
    /* Channel 0 -> Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;          /* base channel interrupt */

    /* Channel 1 -> Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;                  /* no ISR on this channel */

    /* Channel 2 -> Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;                  /* no ISR on this channel */

    /* 6) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                   /* target Cluster_1 per requirements */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;           /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;        /* center-aligned */
    config.syncStart            = TRUE;                                /* synchronous start */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;         /* TOM uses FXCLK_0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock; /* DTM clock */
    config.syncUpdateEnabled    = TRUE;                                /* synchronous update */

    /* Initialize the PWM with persistent channel storage */
    IfxGtm_Pwm_init(&g_tom3phInv.pwm, &g_tom3phInv.channels[0], &config);

    /* 7) Store initial duties, phases, and dead-times in module state */
    g_tom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_tom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_tom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_tom3phInv.phases[0] = channelConfig[0].phase;
    g_tom3phInv.phases[1] = channelConfig[1].phase;
    g_tom3phInv.phases[2] = channelConfig[2].phase;

    g_tom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure LED GPIO as push-pull output */
    IfxPort_setPinMode(LED, IfxPort_Mode_outputPushPullGeneral);
}

/**
 * Percentage-based duty update for 3 logical channels using wrap rule:
 * if ((duty+step) >= 100) duty=0; duty+=step;  (applied per channel)
 */
void updateGtmTom3phInvDuty(void)
{
    if ((g_tom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[2] = 0.0f; }

    g_tom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply new duties immediately; driver manages shadow-transfer/sync */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInv.pwm, (float32 *)g_tom3phInv.dutyCycles);
}
