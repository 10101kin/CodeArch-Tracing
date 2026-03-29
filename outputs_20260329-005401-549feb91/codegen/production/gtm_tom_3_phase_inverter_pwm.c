/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief GTM TOM 3-Phase Inverter PWM driver using unified IfxGtm_Pwm API
 *
 * This module configures TOM1 for three complementary PWM pairs (U, V, W) with:
 * - Center-aligned operation at 20 kHz (timebase on TOM1 CH0)
 * - Dead-time: 0.5 us (rising/falling)
 * - Min pulse: 1.0 us (stored in state for application enforcement)
 * - Pin routing per user request (placeholders provided if pinmap symbols are not available)
 * - Initial duties: U=25%, V=50%, W=75%
 *
 * Notes:
 * - Watchdog handling is NOT performed here (must be in CpuX_Main.c only).
 * - All CMU/clock configuration is inside the enable guard as mandated.
 * - Unified PWM API is used (IfxGtm_Pwm). No separate TOM/ATOM timer setup.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================== Macros =============================== */

/* Channel count: three complementary pairs => three unified PWM channels */
#define NUM_OF_CHANNELS                 (3u)

/* PWM frequency (Hz) */
#define PWM_FREQUENCY_HZ                (20000.0f)

/* Dead-time and min-pulse (seconds) */
#define PWM_DEADTIME_S                  (0.5e-6f)   /* 0.5 us */
#define PWM_MIN_PULSE_S                 (1.0e-6f)   /* 1.0 us */

/* ISR priority macro (project-specific value) */
#define ISR_PRIORITY_ATOM               (3)

/* Debug LED pin: P13.0 (compound macro usage in IfxPort_* calls) */
#define LED &MODULE_P13, 0

/*
 * Pin mapping macros (user-requested pins). If validated pin symbols are not
 * available in the project's PinMap headers, keep these as NULL_PTR and
 * replace during integration with the appropriate iLLD symbols, for example:
 *   &IfxGtm_TOM1_2_TOUT12_P00_3_OUT, &IfxGtm_TOM1_1_TOUT11_P00_2_OUT, ...
 */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUT12_P00_3_OUT */
#define PHASE_U_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUT11_P00_2_OUT */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUT14_P00_5_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUT13_P00_4_OUT */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_6_TOUT16_P00_7_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUT15_P00_6_OUT */

/* Initial duties in percent (U, V, W) */
#define PHASE_U_INIT_DUTY_PCT           (25.0f)
#define PHASE_V_INIT_DUTY_PCT           (50.0f)
#define PHASE_W_INIT_DUTY_PCT           (75.0f)

/* ========================== Module State ============================= */

typedef struct
{
    IfxGtm_Pwm              pwm;                                  /* unified PWM handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];            /* persistent channels array */
    float32                 dutyCycles[NUM_OF_CHANNELS];           /* duty in percent per sync index */
    float32                 phases[NUM_OF_CHANNELS];               /* phase offsets (deg or fraction) */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];            /* per-channel dead-time record */
    float32                 minPulse;                              /* minimum on-time (seconds) */
} GtmTom3PhPwm_StateT;

/* IFX_STATIC is required by project coding rules (provided by Compilers.h via Ifx_Types.h) */
IFX_STATIC GtmTom3PhPwm_StateT g_gtmTom3PhPwm;

/* ============================ ISR/Callback =========================== */

/* ISR name matches driver convention. The ISR only toggles the LED (minimal ISR). */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback assigned to InterruptConfig; must be an empty body. */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================== API ================================= */

/**
 * @brief Initialize the GTM unified PWM (TOM) for a 3-phase inverter.
 *
 * Algorithm per SW Detailed Design:
 * 1) Declare persistent state (already present at file scope).
 * 2) Create and initialize configuration structures.
 * 3) Configure TOM submodule, center alignment, FXCLK0 clock source, sync start, 20 kHz.
 * 4) Configure output routing for six complementary outputs.
 * 5) Populate dead-time (0.5 us) and min-pulse (1.0 us) into state and dtm configs.
 * 6) Inside enable guard: enable GTM, set GCLK to module frequency, enable FXCLK/CLK0.
 * 7) Initialize unified PWM with persistent handle and channels.
 * 8) Ensure base frequency via immediate update (20 kHz). Dead-time from dtmConfig is applied by init.
 * 9) Initial duties are already provided in channelConfig; preserved as atomic shadow values.
 * 10) Sync start occurs via config.syncStart = TRUE; channels run in lockstep.
 */
void initGtmTomPwm(void)
{
    /* 2) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* Initialize defaults from driver */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 4) Output routing: high-side pin + complementary low-side pin per phase */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;   /* high-side active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;    /* low-side  active LOW  */
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

    /* 5) DTM configuration: 0.5 us rising/falling for each complementary pair */
    dtmConfig[0].deadTime.rising = PWM_DEADTIME_S;
    dtmConfig[0].deadTime.falling = PWM_DEADTIME_S;
    dtmConfig[1].deadTime.rising = PWM_DEADTIME_S;
    dtmConfig[1].deadTime.falling = PWM_DEADTIME_S;
    dtmConfig[2].deadTime.rising = PWM_DEADTIME_S;
    dtmConfig[2].deadTime.falling = PWM_DEADTIME_S;

    /* Interrupt configuration for base channel (period event only) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 3) Channel configuration using logical indices (0..N-1) */
    /* CH0 → Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;   /* timebase channel */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INIT_DUTY_PCT;       /* 25% */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;                     /* base channel interrupt */

    /* CH1 → Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INIT_DUTY_PCT;       /* 50% */
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = &irqCfg;

    /* CH2 → Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INIT_DUTY_PCT;       /* 75% */
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = &irqCfg;

    /* 3) Main PWM configuration */
    /* config.cluster: default from initConfig is used */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;             /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;          /* center-aligned */
    config.syncStart            = TRUE;                                  /* synchronized start */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                      /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                    /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;      /* DTM clock */
    config.syncUpdateEnabled    = TRUE;                                  /* enable shadow updates */

    /* 6) Enable guard: enable GTM and configure CMU clocks (inside guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Initialize unified PWM (applies pin mux, alignment, timebase, shadow regs) */
    IfxGtm_Pwm_init(&g_gtmTom3PhPwm.pwm, &g_gtmTom3PhPwm.channels[0], &config);

    /* 10) Persistent state capture (duties, phases, dead-times, minPulse) */
    g_gtmTom3PhPwm.dutyCycles[0] = PHASE_U_INIT_DUTY_PCT;
    g_gtmTom3PhPwm.dutyCycles[1] = PHASE_V_INIT_DUTY_PCT;
    g_gtmTom3PhPwm.dutyCycles[2] = PHASE_W_INIT_DUTY_PCT;

    g_gtmTom3PhPwm.phases[0] = 0.0f;
    g_gtmTom3PhPwm.phases[1] = 0.0f;
    g_gtmTom3PhPwm.phases[2] = 0.0f;

    g_gtmTom3PhPwm.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3PhPwm.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3PhPwm.deadTimes[2] = dtmConfig[2].deadTime;

    g_gtmTom3PhPwm.minPulse = PWM_MIN_PULSE_S;

    /* 8) Ensure base frequency is programmed to 20 kHz immediately */
    IfxGtm_Pwm_updateFrequencyImmediate(&g_gtmTom3PhPwm.pwm, PWM_FREQUENCY_HZ);

    /* LED GPIO init (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}
