/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm (TC3xx)
 *
 * Notes:
 * - Follows mandatory initialization patterns from iLLD unified PWM driver
 * - No watchdog API is used here (must be in CpuX_Main.c only)
 * - No STM-based timing; scheduling belongs to application loop
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ====================================================================== */
/* Configuration Macros (from requirements)                                */
/* ====================================================================== */
#define NUM_OF_CHANNELS          (3)
#define PWM_FREQUENCY_HZ         (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM        (20)

/* Initial duty (percent) */
#define PHASE_U_DUTY_INIT        (25.0f)
#define PHASE_V_DUTY_INIT        (50.0f)
#define PHASE_W_DUTY_INIT        (75.0f)

/* Duty update step (percent) */
#define PHASE_DUTY_STEP          (10.0f)

/* LED macro (port, pin) */
#define LED                      &MODULE_P13, 0

/*
 * Pin routing macros:
 * For migration safety (pin-map TBD), keep pins unassigned here. The unified driver
 * supports NULL_PTR pins; routing can be completed once final TOUT mapping is confirmed.
 * Do not invent pin symbols. Update these macros with validated IfxGtm TOM1 pin symbols
 * when available (e.g., &IfxGtm_TOM1_8_TOUTxx_Pyy_z_OUT and its complementary _N_ pad).
 */
#define PHASE_U_HS               (NULL_PTR)
#define PHASE_U_LS               (NULL_PTR)
#define PHASE_V_HS               (NULL_PTR)
#define PHASE_V_LS               (NULL_PTR)
#define PHASE_W_HS               (NULL_PTR)
#define PHASE_W_LS               (NULL_PTR)

/* ====================================================================== */
/* Module State                                                           */
/* ====================================================================== */
typedef struct
{
    IfxGtm_Pwm           pwm;                                 /* Unified PWM driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];           /* Persistent channels (required by driver) */
    float32              dutyCycles[NUM_OF_CHANNELS];          /* Duty in percent */
    float32              phases[NUM_OF_CHANNELS];              /* Phase in percent of period */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];           /* Dead-time per channel */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInv;                     /* Persistent module-state */

/* ====================================================================== */
/* Private ISR and Period Callback                                        */
/* ====================================================================== */
/*
 * ISR: GTM ATOM ISR stub for LED toggle (priority 20 on CPU0).
 * Note: Source routing/timer setup is not performed here by design
 * (interrupt linkage is handled by application or driver configuration).
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Empty period-event callback registered via IfxGtm_Pwm_InterruptConfig.
 * Intentionally performs no work.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ====================================================================== */
/* Public API Implementations                                             */
/* ====================================================================== */
/*
 * Initialize the GTM-based 3-phase complementary PWM using IfxGtm_Pwm.
 * - 3 complementary HS/LS pairs (center-aligned, 20 kHz)
 * - 1 us dead-time (rising and falling)
 * - synchronous start and sync shadow-update
 * - TOM1 time base (Cluster_1 assumed for migration), base channel is first entry
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures as locals */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* 2) Load defaults and fill output/dead-time */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output configuration: complementary HS/LS per phase */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;   /* HS */
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;   /* LS */
    output[0].polarity              = Ifx_ActiveState_high;               /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;                /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;   /* HS */
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;   /* LS */
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;   /* HS */
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;   /* LS */
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: 1 us on both edges for all phases */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 3) Prepare base-channel interrupt configuration */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 4) Channel configurations: TOM1 base with center-aligned PWM */
    /* Base/master channel for sync group: place interrupt only on channel 0 */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_8;  /* TOM1 Cluster_1 assumed */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_10;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_12;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Complete main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                    /* Cluster_1 */
    config.subModule           = IfxGtm_Pwm_SubModule_tom;            /* TOM */
    config.alignment           = IfxGtm_Pwm_Alignment_center;         /* Center-aligned */
    config.syncStart           = TRUE;                                 /* Synchronous start */
    config.syncUpdateEnabled   = TRUE;                                 /* Synchronous shadow-update */
    config.numChannels         = NUM_OF_CHANNELS;
    config.channels            = &channelConfig[0];
    config.frequency           = PWM_FREQUENCY_HZ;                     /* Hz */
    config.clockSource.atom    = IfxGtm_Cmu_Fxclk_0;                   /* FXCLK0 for TOM/ATOM */
    config.dtmClockSource      = IfxGtm_Dtm_ClockSource_cmuClock0;     /* DTM clock */

    /* 5) GTM enable guard + CMU configuration inside guard */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_tom3phInv.pwm, &g_tom3phInv.channels[0], &config);

    /* Store initial state (duties, dead-times, phases) for runtime updates */
    g_tom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_tom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_tom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_tom3phInv.phases[0] = channelConfig[0].phase;
    g_tom3phInv.phases[1] = channelConfig[1].phase;
    g_tom3phInv.phases[2] = channelConfig[2].phase;

    g_tom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure LED GPIO as push-pull output (initial state per board default) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Duty update (+10% with wrap) applied immediately and synchronously.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 -> duty = 0, then always add step */
    if ((g_tom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[2] = 0.0f; }

    g_tom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately; driver handles synchronized application across channels */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInv.pwm, (float32 *)g_tom3phInv.dutyCycles);
}
