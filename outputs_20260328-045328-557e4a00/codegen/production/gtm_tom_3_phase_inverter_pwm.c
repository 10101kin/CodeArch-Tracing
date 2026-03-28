/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM on AURIX TC3xx using unified IfxGtm_Pwm
 * - Center-aligned PWM, complementary outputs with DTM 1us dead-time
 * - Frequency: 20 kHz, TOM1 Cluster_1
 * - Synchronous start and synchronous (buffered) update enabled
 * - Period-event callback stub provided; ISR toggles LED on P13.0
 *
 * Mandatory patterns:
 *  - Follow iLLD initialization sequence
 *  - GTM enable guard with dynamic CMU configuration
 *  - No watchdog handling in this module
 *  - No STM timing logic in this module
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ===========================
 * Configuration Macros
 * =========================== */
#define NUM_OF_CHANNELS        (3U)
#define PWM_FREQUENCY          (20000.0f)
#define ISR_PRIORITY_ATOM      (20)

/* Initial duties in percent */
#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)
#define PHASE_DUTY_STEP        (10.0f)

/* LED pin macro (port, pin) */
#define LED                    &MODULE_P13, 0

/*
 * Pin routing macros for TOM1 complementary outputs (Phase U/V/W).
 * Note: No validated pin symbols were provided by the template. To map outputs
 *       to the requested pads (U: P02.0/P02.7, V: P02.1/P02.4, W: P02.2/P02.5)
 *       on TOM1, Cluster_1, replace the NULL_PTRs below with the corresponding
 *       IfxGtm_TOM1_x_TOUTy_P02_z_OUT symbols from the device pinmap headers.
 *       The unified IfxGtm_Pwm driver consumes these via the OutputConfig.
 */
#define PHASE_U_HS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_0_TOUTxx_P02_0_OUT */
#define PHASE_U_LS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_1_TOUTyy_P02_7_OUT */
#define PHASE_V_HS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_2_TOUTxx_P02_1_OUT */
#define PHASE_V_LS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_3_TOUTyy_P02_4_OUT */
#define PHASE_W_HS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_4_TOUTxx_P02_2_OUT */
#define PHASE_W_LS   ((IfxGtm_Pwm_ToutMap*)NULL_PTR) /* e.g., &IfxGtm_TOM1_5_TOUTyy_P02_5_OUT */

/* ===========================
 * Module State
 * =========================== */

typedef struct
{
    IfxGtm_Pwm                 pwm;                                 /* PWM handle */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];           /* Persistent channel handles (driver stores pointer) */
    float32                    dutyCycles[NUM_OF_CHANNELS];         /* Duty cycle state (percent) */
    float32                    phases[NUM_OF_CHANNELS];             /* Phase state (fraction 0..1) */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];          /* Dead-time state */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInvState;

/* ===========================
 * Private Declarations
 * =========================== */

/* Period-event callback: empty body as per design */
void IfxGtm_periodEventFunction(void *data);

/* ISR: toggle LED on P13.0 at priority ISR_PRIORITY_ATOM */
IFX_INTERRUPT(interruptGtmAtom0Ch0, 0, ISR_PRIORITY_ATOM);

/* ===========================
 * Private Implementations
 * =========================== */

void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

void interruptGtmAtom0Ch0(void)
{
    IfxPort_togglePin(LED);
}

/* ===========================
 * Public API Implementations
 * =========================== */

void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects (setup-only) */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Populate output pin routing for three logical complementary channels */
    /* Phase U */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;             /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;              /* LS active low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM/dead-time configuration: 1 us on rising and falling */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 5) Interrupt configuration for base channel (period event) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2, initial duties) */
    /* Ch 0 - Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;           /* percent */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;       /* Base channel gets period event */

    /* Ch 1 - Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;           /* percent */
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;               /* No ISR on sync channels */

    /* Ch 2 - Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;           /* percent */
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;               /* No ISR on sync channels */

    /* 7) Complete main config for TOM1 Cluster_1, center-aligned, sync start/update, FXCLK0 */
    config.gtmSFR             = &MODULE_GTM;
    config.cluster            = IfxGtm_Cluster_1;
    config.subModule          = IfxGtm_Pwm_SubModule_tom;
    config.alignment          = IfxGtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource        = IfxGtm_Cmu_Clk_0;                 /* Use first FXCLK (CLK0) */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock from FXCLK0 */

    /* 8) GTM enable guard and CMU clock configuration */
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

    /* 9) Initialize the PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInvState.pwm, &g_gtmTom3phInvState.channels[0], &config);

    /* 10) Store configured duties, phases and dead-times into module state */
    g_gtmTom3phInvState.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInvState.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInvState.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInvState.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInvState.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInvState.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure debug LED GPIO as push-pull output (P13.0) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateGtmTom3phInvDuty(void)
{
    /* Apply wrap rule: if (duty + step) >= 100 then duty = 0; duty += step; */
    if ((g_gtmTom3phInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInvState.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate update: pass duty array (percent) directly */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInvState.pwm, (float32 *)g_gtmTom3phInvState.dutyCycles);
}
