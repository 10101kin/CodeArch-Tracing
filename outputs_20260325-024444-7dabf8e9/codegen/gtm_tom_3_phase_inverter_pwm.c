/**
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * TC3xx (TC387) 3-phase inverter PWM using IfxGtm_Pwm (TOM) + IfxGtm_Tom_Timer timebase.
 * - Center-aligned 10 kHz
 * - Dead-time 500 ns via DTM (hardware dead-time)
 * - Atomic updates via TGC gating (disableUpdate/applyUpdate)
 * - Complementary phase legs within TOM0 TGC0 for atomicity
 *
 * Notes:
 * - Uses unified IfxGtm_Pwm high-level driver with OutputConfig for pin routing.
 * - Pins selected from validated PinMap for TC387 (generic IfxGtm_PinMap.h symbols).
 * - No watchdog handling here (must be in CpuN_Main.c).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD headers (generic) */
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* =========================
 * Configuration constants
 * ========================= */
#define NUM_OF_CHANNELS                 (3u)

/* Timing requirements */
#define PWM_FREQUENCY_HZ                (10000.0f)         /* 10 kHz center-aligned */
#define DEAD_TIME_SEC                   (5.0e-7f)          /* 500 ns dead-time */

/* Clocking (requirements) */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ  (300.0f)
#define CLOCK_GTM_GCLK_MHZ              (300.0f)
#define CLOCK_CMU_CLK0_MHZ              (100.0f)

/* Duty-cycle behavior (percent representation) */
#define INITIAL_DUTY_PERCENT_U          (25.0f)
#define INITIAL_DUTY_PERCENT_V          (50.0f)
#define INITIAL_DUTY_PERCENT_W          (75.0f)
#define DUTY_STEP_PERCENT               (10.0f)

/* Additional reference-style macros (percent domain) */
#define DUTY_25_PERCENT                 (25.0f)
#define DUTY_50_PERCENT                 (50.0f)
#define DUTY_75_PERCENT                 (75.0f)
#define DUTY_STEP                       (10.0f)
#define DUTY_MIN                        (0.0f)
#define DUTY_MAX                        (100.0f)

/* =========================
 * Pin assignments (validated examples for TC387)
 * Keep all channels within TOM0 TGC0 for atomic updates
 * ========================= */
/* Phase U: TOM0 CH0 */
#define PHASE_U_HS   (&IfxGtm_TOM0_0_TOUT53_P21_2_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM0_0N_TOUT51_P21_0_OUT)
/* Phase V: TOM0 CH1 */
#define PHASE_V_HS   (&IfxGtm_TOM0_1_TOUT54_P21_3_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM0_1N_TOUT52_P21_1_OUT)
/* Phase W: TOM0 CH4 */
#define PHASE_W_HS   (&IfxGtm_TOM0_4_TOUT64_P20_8_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM0_4N_TOUT65_P20_9_OUT)

/* =========================
 * Driver state
 * ========================= */
typedef struct {
    IfxGtm_Pwm           pwm;                              /* Unified PWM driver */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];        /* Channel runtime data */
    float32              dutyCycles[NUM_OF_CHANNELS];      /* High-side duties in percent */
    float32              phases[NUM_OF_CHANNELS];          /* Phase shift values (unused) */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];       /* Dead-time per channel */
    IfxGtm_Tom_Timer     timer;                            /* Shared TOM timebase for TGC gating */
} GtmTom3phInv;

static GtmTom3phInv s_drv;
static boolean      s_initialized = FALSE;

/* =========================
 * Initialization (SW Detailed Design contract)
 * ========================= */
void initGtmTom3phInv(void)
{
    /* 1) Enable GTM and required CMU clocks */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
    }
    /* Set GCLK and CLK0 per requirements */
    IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, (CLOCK_GTM_GCLK_MHZ * 1.0e6f));
    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, (CLOCK_CMU_CLK0_MHZ * 1.0e6f));
    /* Route TOM clock from GCLK for CLK0 */
    IfxGtm_Cmu_selectClkInput(&MODULE_GTM, IfxGtm_Cmu_Clk_0, TRUE);
    /* Enable FXCLK domain (reference pattern). CLK0 enable is implementation-defined in iLLD */
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* 2) Configure TOM timebase for center-aligned 10 kHz */
    IfxGtm_Tom_Timer_Config timerConfig;
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    /* Reference-style fields (as per iLLD patterns) */
    timerConfig.base.frequency = PWM_FREQUENCY_HZ;
    timerConfig.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;  /* Fxclk0 derived from CLK0 */
    timerConfig.tom            = IfxGtm_Tom_0;                    /* TOM0 timebase */
    timerConfig.timerChannel   = IfxGtm_Tom_Ch_0;                  /* Use CH0 as the master timer */

    if (IfxGtm_Tom_Timer_init(&s_drv.timer, &timerConfig) == FALSE)
    {
        /* Abort init on failure (critical error handling requirement) */
        return;
    }

    /* 3) Route six PWM outputs via unified IfxGtm_Pwm output config (no explicit PinMap calls) */
    IfxGtm_Pwm_Config          pwmConfig;
    IfxGtm_Pwm_ChannelConfig   chCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    outCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmCfg[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&pwmConfig, &MODULE_GTM);

    /* Output configuration for complementary pairs */
    outCfg[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    outCfg[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    outCfg[0].polarity               = Ifx_ActiveState_high;
    outCfg[0].complementaryPolarity  = Ifx_ActiveState_low;
    outCfg[0].outputMode             = IfxPort_OutputMode_pushPull;
    outCfg[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    outCfg[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    outCfg[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    outCfg[1].polarity               = Ifx_ActiveState_high;
    outCfg[1].complementaryPolarity  = Ifx_ActiveState_low;
    outCfg[1].outputMode             = IfxPort_OutputMode_pushPull;
    outCfg[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    outCfg[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    outCfg[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    outCfg[2].polarity               = Ifx_ActiveState_high;
    outCfg[2].complementaryPolarity  = Ifx_ActiveState_low;
    outCfg[2].outputMode             = IfxPort_OutputMode_pushPull;
    outCfg[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Program dead-time 500 ns via DTM */
    dtmCfg[0].deadTime.rising  = DEAD_TIME_SEC;
    dtmCfg[0].deadTime.falling = DEAD_TIME_SEC;
    dtmCfg[1].deadTime.rising  = DEAD_TIME_SEC;
    dtmCfg[1].deadTime.falling = DEAD_TIME_SEC;
    dtmCfg[2].deadTime.rising  = DEAD_TIME_SEC;
    dtmCfg[2].deadTime.falling = DEAD_TIME_SEC;

    /* 5) Build channel configurations (same TOM instance/TGC for atomic updates) */
    /* Initialize channel config defaults (pattern requirement) */
    IfxGtm_Pwm_initChannelConfig(&chCfg[0]);
    IfxGtm_Pwm_initChannelConfig(&chCfg[1]);
    IfxGtm_Pwm_initChannelConfig(&chCfg[2]);

    /* Phase U on TOM0 CH0 */
    chCfg[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    chCfg[0].phase      = 0.0f;
    chCfg[0].duty       = INITIAL_DUTY_PERCENT_U;
    chCfg[0].dtm        = &dtmCfg[0];
    chCfg[0].output     = &outCfg[0];
    chCfg[0].mscOut     = NULL_PTR;
    chCfg[0].interrupt  = NULL_PTR;   /* No ISR in this module */

    /* Phase V on TOM0 CH1 */
    chCfg[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    chCfg[1].phase      = 0.0f;
    chCfg[1].duty       = INITIAL_DUTY_PERCENT_V;
    chCfg[1].dtm        = &dtmCfg[1];
    chCfg[1].output     = &outCfg[1];
    chCfg[1].mscOut     = NULL_PTR;
    chCfg[1].interrupt  = NULL_PTR;

    /* Phase W on TOM0 CH4 */
    chCfg[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_4;
    chCfg[2].phase      = 0.0f;
    chCfg[2].duty       = INITIAL_DUTY_PERCENT_W;
    chCfg[2].dtm        = &dtmCfg[2];
    chCfg[2].output     = &outCfg[2];
    chCfg[2].mscOut     = NULL_PTR;
    chCfg[2].interrupt  = NULL_PTR;

    /* 6) Global PWM config (sync + center alignment + clock source) */
    pwmConfig.cluster            = IfxGtm_Cluster_0;
    pwmConfig.subModule          = IfxGtm_Pwm_SubModule_tom;
    pwmConfig.alignment          = IfxGtm_Pwm_Alignment_center;
    pwmConfig.syncStart          = TRUE;                      /* Start channels after init */
    pwmConfig.syncUpdateEnabled  = TRUE;                      /* Shadow register updates */
    pwmConfig.numChannels        = NUM_OF_CHANNELS;
    pwmConfig.channels           = &chCfg[0];
    pwmConfig.frequency          = PWM_FREQUENCY_HZ;
    pwmConfig.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;        /* TOM uses Fxclk enum */
    pwmConfig.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 7) Initialize PWM driver */
    IfxGtm_Pwm_init(&s_drv.pwm, &s_drv.channels[0], &pwmConfig);

    /* Set hardware dead-time immediately as well (explicit per design) */
    IfxGtm_Pwm_updateChannelDeadTimeImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)0, dtmCfg[0].deadTime);
    IfxGtm_Pwm_updateChannelDeadTimeImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)1, dtmCfg[1].deadTime);
    IfxGtm_Pwm_updateChannelDeadTimeImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)2, dtmCfg[2].deadTime);

    /* 8) Gate TGC updates, program initial duties for coherent start, then apply */
    s_drv.dutyCycles[0] = INITIAL_DUTY_PERCENT_U;
    s_drv.dutyCycles[1] = INITIAL_DUTY_PERCENT_V;
    s_drv.dutyCycles[2] = INITIAL_DUTY_PERCENT_W;

    IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)0, s_drv.dutyCycles[0]);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)1, s_drv.dutyCycles[1]);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)2, s_drv.dutyCycles[2]);
    IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);

    /* Ready for runtime updates */
    s_initialized = TRUE;
}

/* =========================
 * Runtime duty-step update (SW Detailed Design contract)
 * ========================= */
void updateGtmTom3phInvDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit on uninitialized state (tests expect no side-effects) */
    }

    /* 1) Increase each phase's high-side duty by +10% (percent domain) */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        s_drv.dutyCycles[i] += DUTY_STEP_PERCENT;
        /* 2) Wrap to 0..100% range */
        if (s_drv.dutyCycles[i] >= 100.0f)
        {
            s_drv.dutyCycles[i] -= 100.0f;
        }
    }

    /* 3) Complementary low-side duty conceptually = (100 - high-side duty)
       Unified driver handles complementary via polarity + DTM; no separate write needed. */

    /* 4) Disable TGC update to gate changes */
    IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);

    /* 5) Write new duties immediately to their PWM channels (high-side channels) */
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)0, s_drv.dutyCycles[0]);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)1, s_drv.dutyCycles[1]);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)2, s_drv.dutyCycles[2]);

    /* 6) Atomically apply update (all six outputs latch together within same TGC) */
    IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
}
