/*==========================================================================
 *  File: gtm_tom_3_phase_inverter_pwm.c
 *  Brief: Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm
 *
 *  Notes:
 *   - Follows iLLD high-level IfxGtm_Pwm initialization pattern.
 *   - Center-aligned 20 kHz TOM1 PWM on single-ended HS pins P00.3/P00.5/P00.7.
 *   - Synchronous start and synchronous update enabled.
 *   - No watchdog handling here (must be in CpuX main per AURIX standard).
 *==========================================================================*/

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* =============================== Macros ================================== */
/* Number of logical PWM channels (U, V, W) */
#define GTM_TOM_3PH_NUM_CHANNELS      (3U)
/* PWM switching frequency (Hz) */
#define GTM_TOM_3PH_PWM_FREQUENCY     (20000.0f)
/* ISR priority macro (used by IFX_INTERRUPT) */
#define ISR_PRIORITY_ATOM             (3)

/* Phase pin selection (HS only, single-ended) */
#define PHASE_U_HS                    (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_V_HS                    (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_W_HS                    (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)

/* Initial duty cycles (%) */
#define PHASE_U_DUTY                  (25.0f)
#define PHASE_V_DUTY                  (50.0f)
#define PHASE_W_DUTY                  (75.0f)

/* Duty update step and wrap thresholds (%) */
#define PHASE_DUTY_STEP               (10.0f)
#define PHASE_DUTY_MIN                (10.0f)
#define PHASE_DUTY_MAX                (90.0f)

/* LED pin used by ISR for debug toggle */
#define LED                           &MODULE_P13, 0

/* ============================ Module State =============================== */

typedef struct
{
    IfxGtm_Pwm             pwm;                                         /* PWM driver handle */
    IfxGtm_Pwm_Channel     channels[GTM_TOM_3PH_NUM_CHANNELS];          /* Persistent channel handles */
    float32                dutyCycles[GTM_TOM_3PH_NUM_CHANNELS];        /* Duty in percent */
    float32                phases[GTM_TOM_3PH_NUM_CHANNELS];            /* Phase (deg or percent domain) */
    IfxGtm_Pwm_DeadTime    deadTimes[GTM_TOM_3PH_NUM_CHANNELS];         /* Stored dead-times */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3ph;

/* ============================== ISR/CBK ================================= */
/* ISR: minimal body as per guidelines (toggle LED). */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback (assigned via InterruptConfig in other designs). Empty body by rule. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================== API ===================================== */
/**
 * Initialize 3-phase center-aligned PWM on TOM using IfxGtm_Pwm.
 * Configuration is performed with local structures, then HW is touched within
 * a GTM enable guard. Channels are U,V,W mapped to TOM1 HS pins P00.3/5/7.
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[GTM_TOM_3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[GTM_TOM_3PH_NUM_CHANNELS];

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (HS only, single-ended; complementaryPin = NULL) */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = NULL_PTR;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = NULL_PTR;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = NULL_PTR;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time: zero for single-ended mode */
    dtmConfig[0].deadTime.rising = 0.0f; dtmConfig[0].deadTime.falling = 0.0f;
    dtmConfig[1].deadTime.rising = 0.0f; dtmConfig[1].deadTime.falling = 0.0f;
    dtmConfig[2].deadTime.rising = 0.0f; dtmConfig[2].deadTime.falling = 0.0f;

    /* Channel configuration: logical ordinal indices Ch_0..Ch_2 with initial duties */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = NULL_PTR; /* No ISR used */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR; /* No ISR used */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR; /* No ISR used */

    /* 4) Main configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM_3PH_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_3PH_PWM_FREQUENCY;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;         /* TOM uses FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;

    /* 5) GTM enable-guard with mandatory CMU configuration */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent state handle and channels array */
    IfxGtm_Pwm_init(&g_gtmTom3ph.pwm, &g_gtmTom3ph.channels[0], &config);

    /* 7) Persist initial state for runtime updates */
    g_gtmTom3ph.dutyCycles[0] = PHASE_U_DUTY;
    g_gtmTom3ph.dutyCycles[1] = PHASE_V_DUTY;
    g_gtmTom3ph.dutyCycles[2] = PHASE_W_DUTY;

    g_gtmTom3ph.phases[0] = channelConfig[0].phase;
    g_gtmTom3ph.phases[1] = channelConfig[1].phase;
    g_gtmTom3ph.phases[2] = channelConfig[2].phase;

    g_gtmTom3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* Debug LED pin configuration (after PWM init) */
    IfxPort_setPinModeOutput(&MODULE_P13, 0, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/**
 * Update the three phase duty cycles and apply synchronously via immediate update.
 * Algorithm per design: add +10.0 to each; if result >= 90.0 wrap to 10.0.
 */
void updateGtmTom3phInvDuty(void)
{
    float32 tmp;

    tmp = g_gtmTom3ph.dutyCycles[0] + PHASE_DUTY_STEP;
    g_gtmTom3ph.dutyCycles[0] = (tmp >= PHASE_DUTY_MAX) ? PHASE_DUTY_MIN : tmp;

    tmp = g_gtmTom3ph.dutyCycles[1] + PHASE_DUTY_STEP;
    g_gtmTom3ph.dutyCycles[1] = (tmp >= PHASE_DUTY_MAX) ? PHASE_DUTY_MIN : tmp;

    tmp = g_gtmTom3ph.dutyCycles[2] + PHASE_DUTY_STEP;
    g_gtmTom3ph.dutyCycles[2] = (tmp >= PHASE_DUTY_MAX) ? PHASE_DUTY_MIN : tmp;

    /* Apply all three updates coherently (single shadow transfer) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3ph.pwm, (float32 *)g_gtmTom3ph.dutyCycles);
}
