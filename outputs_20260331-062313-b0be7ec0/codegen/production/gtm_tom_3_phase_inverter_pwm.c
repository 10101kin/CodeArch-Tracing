/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm unified driver
 *
 * - Submodule: TOM
 * - Alignment: center-aligned
 * - Frequency: 20 kHz
 * - Channels: 3 (U, V, W) on TOM1 high-side pins (single-ended)
 * - Synchronous start and synchronous update enabled
 * - Clock source: CMU FXCLK0 for TOM
 *
 * Notes:
 * - Watchdogs must be controlled only in CpuX_Main.c (not here)
 * - No STM timing in this driver; scheduling performed in CpuX_Main.c
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu.h"

/* ========================= Macros and Configuration Constants ========================= */

#define NUM_OF_CHANNELS               (3U)
#define PWM_FREQUENCY_HZ              (20000.0f)

/* Initial duties in percent */
#define PHASE_U_DUTY_INIT             (25.0f)
#define PHASE_V_DUTY_INIT             (50.0f)
#define PHASE_W_DUTY_INIT             (75.0f)

/* Update step and wrap thresholds in percent */
#define PHASE_DUTY_STEP               (10.0f)
#define DUTY_MIN_THRESHOLD            (10.0f)
#define DUTY_MAX_THRESHOLD            (90.0f)

/* ISR priority (used by IFX_INTERRUPT declaration, ISR not wired in this module) */
#define ISR_PRIORITY_ATOM             (3)

/* Debug LED: Port/Pin pair macro */
#define LED                           &MODULE_P13, 0

/* Preserved TOM1 high-side TOUT mappings for P00.3 / P00.5 / P00.7 */
#define PHASE_U_HS                    &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_V_HS                    &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_W_HS                    &IfxGtm_TOM1_6_TOUT16_P00_7_OUT

/* ========================= Internal State ========================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                  /* unified PWM handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];            /* persistent channels array */
    float32                 dutyCycles[NUM_OF_CHANNELS];           /* percent [0..100] */
    float32                 phases[NUM_OF_CHANNELS];               /* degrees or percent phase (design uses 0.0f) */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];           /* stored DTM dead times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;                     /* zero-initialized */

/* ========================= ISR and Period Callback (internal) ========================= */

/* ISR: declared per structural rules; not wired via InterruptConfig in this module */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback placeholder; not assigned in this module by design */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API Implementations ========================= */

/*
 * Initialize 3-phase center-aligned PWM on TOM using IfxGtm_Pwm unified driver.
 *
 * Behavior (per design):
 *  - Configure with local config structs first
 *  - Use TOM1 HS pins only (single-ended), polarity active-high
 *  - No interrupt config assignment
 *  - Center alignment, syncStart, syncUpdateEnabled
 *  - Frequency 20 kHz, 3 channels, initial duties U/V/W = 25/50/75 %
 *  - Inside enable-guard: enable GTM, program GCLK and CLK0, enable FXCLK|CLK0
 *  - Initialize PWM with persistent handle and channels array; store state
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: bind TOM1 HS pins, active-high, push-pull, automotive speed */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = NULL_PTR;                       /* single-ended */
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = NULL_PTR;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = NULL_PTR;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: zero dead-time (single-ended) */
    dtmConfig[0].deadTime.rising = 0.0f;  dtmConfig[0].deadTime.falling = 0.0f;
    dtmConfig[1].deadTime.rising = 0.0f;  dtmConfig[1].deadTime.falling = 0.0f;
    dtmConfig[2].deadTime.rising = 0.0f;  dtmConfig[2].deadTime.falling = 0.0f;

    /* 5) Channel configuration: logical indices 0..2; phase=0; duties 25/50/75; no ISR */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = NULL_PTR;   /* no ISR used per design */

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

    /* 6) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_0;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;  /* TOM uses FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;

    /* 7) GTM enable-guard and CMU clocks setup (MANDATORY) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 9) Store persistent state from configuration */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Configure LED GPIO (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update the three phase duty cycles and apply synchronously.
 * Algorithm per design: +10% step; if new value >= 90%, wrap to 10%.
 * Implementation uses pre-check and unconditional add (no loop) for deterministic updates.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Pre-wrap then unconditional add; ensures wrap-to-10 when threshold reached */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= DUTY_MAX_THRESHOLD)
    {
        g_gtmTom3phInv.dutyCycles[0] = (DUTY_MIN_THRESHOLD - PHASE_DUTY_STEP);
    }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= DUTY_MAX_THRESHOLD)
    {
        g_gtmTom3phInv.dutyCycles[1] = (DUTY_MIN_THRESHOLD - PHASE_DUTY_STEP);
    }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= DUTY_MAX_THRESHOLD)
    {
        g_gtmTom3phInv.dutyCycles[2] = (DUTY_MIN_THRESHOLD - PHASE_DUTY_STEP);
    }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply synchronously to all channels using the immediate multi-channel API */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32 *)g_gtmTom3phInv.dutyCycles);
}
