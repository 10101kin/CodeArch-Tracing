/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: 3-phase complementary, center-aligned PWM using unified IfxGtm_Pwm on TOM backend.
 * Target: TC3xx (e.g., TC387), TOM1, Cluster 1, 20 kHz, 1 us dead-time, CMU FXCLK0.
 *
 * Mandatory patterns and APIs are taken from the authoritative iLLD documentation.
 *
 * CRITICAL RULES FOLLOWED:
 * - Initialization sequence strictly follows iLLD unified IfxGtm_Pwm patterns
 * - Enable-guard for GTM + CMU clocks inside guard block only
 * - Complementary outputs configured via IfxGtm_Pwm_OutputConfig array
 * - Dead-time via IfxGtm_Pwm_DtmConfig array
 * - Channel config via IfxGtm_Pwm_ChannelConfig array
 * - Persistent channels array stored in module-state (IFX_STATIC)
 * - Period callback and ISR are separate; ISR only toggles LED
 * - No watchdog handling here; must be in CpuX_Main.c
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ============================= Macros ============================= */
#define NUM_OF_CHANNELS          (3u)
#define PWM_FREQUENCY            (20000.0f)    /* 20 kHz */
#define ISR_PRIORITY_ATOM        (20)

/* Initial duty cycles in percent */
#define PHASE_U_INIT_DUTY        (25.0f)
#define PHASE_V_INIT_DUTY        (50.0f)
#define PHASE_W_INIT_DUTY        (75.0f)

/* Duty update step in percent */
#define PHASE_DUTY_STEP          (10.0f)

/* Debug LED: P13.0 */
#define LED                      &MODULE_P13, 0

/*
 * Pin mappings (user-requested):
 *   U: P02.0 (high) / P02.7 (low)
 *   V: P02.1 (high) / P02.4 (low)
 *   W: P02.2 (high) / P02.5 (low)
 *
 * No validated TOUT pin symbols were provided by the PinMap validation context.
 * Use NULL_PTR placeholders and replace during board integration with the proper
 * IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols for TC38x 516-pin package.
 */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_0_OUT for P02.0 */
#define PHASE_U_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_7_OUT for P02.7 */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_1_OUT for P02.1 */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_4_OUT for P02.4 */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_2_OUT for P02.2 */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_5_OUT for P02.5 */

/* ========================= Module state ========================== */

typedef struct
{
    IfxGtm_Pwm              pwm;                              /* Driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];        /* Persistent channels (driver writes here) */
    float32                 dutyCycles[NUM_OF_CHANNELS];      /* Duty in percent [0..100] per phase */
    float32                 phases[NUM_OF_CHANNELS];          /* Phase offset in seconds (if used) */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];       /* Stored dead-time settings */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInvState;  /* Zero-initialized */

/* ====================== ISR and callbacks ======================== */

/* ISR: priority defined by ISR_PRIORITY_ATOM; provider CPU0; body must only toggle LED */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: assigned in InterruptConfig; must be a no-op */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ===================== Public API functions ====================== */

/**
 * Initialize a 3-channel complementary, center-aligned PWM on TOM backend using unified IfxGtm_Pwm.
 * Configuration: TOM1, Cluster 1, 20 kHz, complementary with 1 us rising/falling dead-times, sync start and sync shadow updates.
 * - Base channel (index 0) gets the period-event interrupt; provider CPU0; priority ISR_PRIORITY_ATOM.
 * - Initial duties: U=25%, V=50%, W=75%.
 * - Persistent channels array from module state is passed to the driver init.
 * - LED P13.0 configured as output for ISR toggling.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare ALL config structs as local variables */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Initialize main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (complementary pairs) */
    /* Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;  /* high-side active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;   /* low-side  active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (1 us rising/falling per pair) */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Interrupt configuration (base channel only) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;   /* period mode */
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;              /* provider CPU0 */
    irqCfg.priority    = ISR_PRIORITY_ATOM;            /* priority 20 */
    irqCfg.periodEvent = IfxGtm_periodEventFunction;   /* callback */
    irqCfg.dutyEvent   = NULL_PTR;                     /* none */

    /* 6) Channel configuration (logical indices 0..2) */
    /* CH0 → Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;              /* base channel interrupt */

    /* CH1 → Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;             /* only base channel has interrupt */

    /* CH2 → Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;             /* only base channel has interrupt */

    /* 7) Main PWM configuration */
    config.cluster              = (IfxGtm_Cluster)1;                    /* Cluster 1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;             /* TOM backend */
    config.alignment            = IfxGtm_Pwm_Alignment_center;          /* center-aligned */
    config.syncStart            = TRUE;                                  /* synchronized start */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                         /* 20 kHz */
    config.clockSource.atom     = (uint32)IfxGtm_Cmu_Fxclk_0;            /* CMU FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;      /* DTM clock source */
    config.syncUpdateEnabled    = TRUE;                                  /* synchronized shadow updates */

    /* 8) Enable guard: enable GTM + CMU clocks if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM (pass persistent channels array from module state) */
    IfxGtm_Pwm_init(&g_tom3phInvState.pwm, &g_tom3phInvState.channels[0], &config);

    /* 10) Store initial duties, phases, and dead-times into module state */
    g_tom3phInvState.dutyCycles[0] = PHASE_U_INIT_DUTY;
    g_tom3phInvState.dutyCycles[1] = PHASE_V_INIT_DUTY;
    g_tom3phInvState.dutyCycles[2] = PHASE_W_INIT_DUTY;

    g_tom3phInvState.phases[0] = channelConfig[0].phase;
    g_tom3phInvState.phases[1] = channelConfig[1].phase;
    g_tom3phInvState.phases[2] = channelConfig[2].phase;

    g_tom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO (no explicit level set) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three phase duties in percent using a fixed +10% step.
 * Wrap-to-0-then-add-step behavior is applied independently per phase.
 * Applies the change immediately and atomically using synchronized shadow updates.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap-to-0-then-add-step behavior: three independent checks, then unconditional adds */
    if ((g_tom3phInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInvState.dutyCycles[2] = 0.0f; }

    g_tom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate atomic update of all three complementary pairs */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInvState.pwm, (float32 *)g_tom3phInvState.dutyCycles);
}
