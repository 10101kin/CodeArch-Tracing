/**
 * @file gtm_tom_3_phase_inverter_pwm.c
 * @brief GTM TOM three-phase inverter PWM driver (TC3xx)
 *
 * Production-ready implementation based on iLLD IfxGtm_Tom_Timer and IfxGtm_Tom_PwmHl.
 *
 * Notes:
 * - No watchdog control is performed here (CPU startup files only).
 * - Uses only iLLD APIs specified in the provided mock signature list.
 */

/* iLLD core types */
#include "Ifx_Types.h"
/* GTM base and CMU control */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
/* TOM Timer and PwmHl drivers */
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
/* Pin mapping for TOM TOUT routing */
#include "IfxGtm_PinMap.h"
/* Optional GPIO access (not used for PWM but kept for completeness) */
#include "IfxPort.h"

/* ====================================================================== */
/* Configuration Macros (from requirements)                               */
/* ====================================================================== */
#define PWM_NUM_CHANNELS               (3u)
#define PWM_FREQUENCY_HZ               (20000u)          /* 20 kHz */
#define PWM_DEADTIME_TICKS             (50u)             /* 0.5 us at 100 MHz FXCLK0 assumption; actual FXCLK read at runtime */
#define PWM_MINPULSE_TICKS             (100u)            /* 1.0 us at 100 MHz */

/* Initial duty cycle percentages */
#define PHASE_U_INIT_DUTY_PCT          (25.0f)
#define PHASE_V_INIT_DUTY_PCT          (50.0f)
#define PHASE_W_INIT_DUTY_PCT          (75.0f)

/* Fixed fraction step for duty/on-time increment: period/20 (5%) */
#define PWM_STEP_DIVISOR               (20u)

/* ====================================================================== */
/* Validated TOUT pin macros (TOM1, channels/pins per requirements)        */
/* ====================================================================== */
/* Phase U: HS TOM1 CH2 -> P00.3 ; LS TOM1 CH1 -> P00.2 */
#define PHASE_U_HS    (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS    (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
/* Phase V: HS TOM1 CH4 -> P00.5 ; LS TOM1 CH3 -> P00.4 */
#define PHASE_V_HS    (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS    (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
/* Phase W: HS TOM1 CH6 -> P00.7 ; LS TOM1 CH5 -> P00.6 */
#define PHASE_W_HS    (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS    (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ====================================================================== */
/* Module persistent state                                                 */
/* ====================================================================== */
typedef struct
{
    IfxGtm_Tom_Timer   timer;                    /* TOM timer handle */
    IfxGtm_Tom_PwmHl   pwm;                      /* Complementary PWM HL driver */
    uint32             periodTicks;              /* Current timer period ticks (derived from FXCLK0 / target freq) */
    uint32             onTime[PWM_NUM_CHANNELS]; /* On-time ticks per phase [U,V,W] */
    boolean            initialized;              /* Driver init flag */
} GtmTom3Ph_State;

/* IFX_STATIC is used per production requirements for module state */
IFX_STATIC GtmTom3Ph_State g_gtmTom3Ph = {0};

/* Static pin arrays kept at file scope to ensure lifetime > driver init */
static const IfxGtm_Tom_ToutMap *s_phaseHigh[PWM_NUM_CHANNELS] =
{
    PHASE_U_HS, PHASE_V_HS, PHASE_W_HS
};
static const IfxGtm_Tom_ToutMap *s_phaseLow[PWM_NUM_CHANNELS] =
{
    PHASE_U_LS, PHASE_V_LS, PHASE_W_LS
};

/* ====================================================================== */
/* Local helpers                                                           */
/* ====================================================================== */
static uint32 gtm_getFxclk0FrequencyHz(void)
{
    /* Read current FXCLK0 frequency from CMU at runtime (no hardcoding) */
    return (uint32)IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0);
}

/* ====================================================================== */
/* Public API implementation                                              */
/* ====================================================================== */
/**
 * @brief Initialize GTM TOM1 timer and 3-phase complementary PWM HL.
 */
void initGtmTomPwm(void)
{
    /* 1) Enable GTM and configure CMU clocks (FXCLK and CLK0), following iLLD pattern */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        {
            float32 modFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, modFreq);
            /* Set CLK0 as well per recommended pattern (ATOM users), safe for TOM */
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, modFreq);
            /* Enable FXCLK and CLK0 domains */
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 2) Configure TOM timer (TOM1, base channel 0, FXCLK0, 20 kHz) */
    IfxGtm_Tom_Timer_Config timerCfg;
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

    /* Assign TOM instance and base channel per requirements */
    timerCfg.tom                 = IfxGtm_Tom_1;                /* TOM1 */
    timerCfg.timerChannel        = IfxGtm_Tom_Ch_0;             /* CH0 as time base */
    timerCfg.base.frequency      = (float32)PWM_FREQUENCY_HZ;   /* 20 kHz */
    timerCfg.base.minResolution  = 0.0f;                        /* use default resolution */
    timerCfg.base.isrPriority    = 0;                           /* no ISR for this driver */
    timerCfg.base.isrProvider    = (IfxSrc_Tos)0;               /* not used here */
    timerCfg.clock               = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0; /* FXCLK0 source */

    if (FALSE == IfxGtm_Tom_Timer_init(&g_gtmTom3Ph.timer, &timerCfg))
    {
        g_gtmTom3Ph.initialized = FALSE;
        return; /* Timer init failed */
    }

    /* 3) Configure complementary PWM HL (3 phases, push-pull, pad driver, polarity) */
    IfxGtm_Tom_PwmHl_Config pwmCfg;
    IfxGtm_Tom_PwmHl_initConfig(&pwmCfg);

    pwmCfg.timer                         = &g_gtmTom3Ph.timer;          /* Link to timer */
    pwmCfg.tom                           = IfxGtm_Tom_1;                 /* TOM1 */
    pwmCfg.base.channelCount             = PWM_NUM_CHANNELS;             /* 3 phases */
    pwmCfg.base.deadtime                 = PWM_DEADTIME_TICKS;           /* ticks */
    pwmCfg.base.minPulse                 = PWM_MINPULSE_TICKS;           /* ticks */
    pwmCfg.base.outputMode               = IfxPort_OutputMode_pushPull;  /* push-pull */
    pwmCfg.base.padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    pwmCfg.base.polarity                 = Ifx_ActiveState_high;         /* high-side active high */
    pwmCfg.base.complementaryPolarity    = Ifx_ActiveState_low;          /* low-side active low (dead-time enabled) */

    /* Assign high/low TOUT pin maps (parallel arrays for complementary outputs) */
    pwmCfg.ccx                           = &s_phaseHigh[0];
    pwmCfg.coutx                         = &s_phaseLow[0];

    if (FALSE == IfxGtm_Tom_PwmHl_init(&g_gtmTom3Ph.pwm, &pwmCfg))
    {
        g_gtmTom3Ph.initialized = FALSE;
        return; /* PWM HL init failed */
    }

    /* 4) Center-aligned PWM mode and start timer */
    (void)IfxGtm_Tom_PwmHl_setMode(&g_gtmTom3Ph.pwm, Ifx_Pwm_Mode_centerAligned);
    IfxGtm_Tom_Timer_run(&g_gtmTom3Ph.timer);

    /* 5) Compute period in ticks from actual FXCLK0 and target frequency */
    {
        uint32 fxclk0Hz = gtm_getFxclk0FrequencyHz();
        /* Period ticks for up-down center-aligned base is derived from timer driver; use FXCLK0/frequency as effective period. */
        g_gtmTom3Ph.periodTicks = (fxclk0Hz / (uint32)PWM_FREQUENCY_HZ);
        if (g_gtmTom3Ph.periodTicks == 0u)
        {
            g_gtmTom3Ph.periodTicks = 1u; /* avoid zero period */
        }
    }

    /* 6) Initial on-time ticks from required initial duties: 25%, 50%, 75% */
    g_gtmTom3Ph.onTime[0] = (uint32)((float32)g_gtmTom3Ph.periodTicks * (PHASE_U_INIT_DUTY_PCT / 100.0f));
    g_gtmTom3Ph.onTime[1] = (uint32)((float32)g_gtmTom3Ph.periodTicks * (PHASE_V_INIT_DUTY_PCT / 100.0f));
    g_gtmTom3Ph.onTime[2] = (uint32)((float32)g_gtmTom3Ph.periodTicks * (PHASE_W_INIT_DUTY_PCT / 100.0f));

    /* 7) Apply initial on-times using a shadow-update sequence for synchronous update */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3Ph.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3Ph.pwm, &g_gtmTom3Ph.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3Ph.timer);

    g_gtmTom3Ph.initialized = TRUE;
}

/**
 * @brief Periodic duty update: advance on-times by a fixed fraction of the period and wrap.
 */
void updateGtmTomPwmDutyCycles(void)
{
    if (!g_gtmTom3Ph.initialized)
    {
        return; /* Not initialized */
    }

    /* Read current period from persistent state (established at init from FXCLK0) */
    uint32 period = g_gtmTom3Ph.periodTicks;
    if (period == 0u)
    {
        period = 1u; /* safety */
    }

    /* Compute step as a fixed fraction of period (period / PWM_STEP_DIVISOR) */
    uint32 step = period / PWM_STEP_DIVISOR;
    if (step == 0u)
    {
        step = 1u; /* guarantee progress */
    }

    /* Calculate allowable range [min .. max] based on minPulse and upper bound */
    const uint32 minTicks = PWM_MINPULSE_TICKS;
    uint32 maxTicks = (period > PWM_MINPULSE_TICKS) ? (period - PWM_MINPULSE_TICKS) : period;
    if (maxTicks < minTicks)
    {
        maxTicks = minTicks; /* guard against misconfiguration */
    }

    /* Update each phase on-time with wrap */
    {
        uint32 t;
        t = g_gtmTom3Ph.onTime[0] + step; g_gtmTom3Ph.onTime[0] = (t > maxTicks) ? minTicks : t;
        t = g_gtmTom3Ph.onTime[1] + step; g_gtmTom3Ph.onTime[1] = (t > maxTicks) ? minTicks : t;
        t = g_gtmTom3Ph.onTime[2] + step; g_gtmTom3Ph.onTime[2] = (t > maxTicks) ? minTicks : t;
    }

    /* Apply updated on-times synchronously at next period event */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3Ph.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gtmTom3Ph.pwm, &g_gtmTom3Ph.onTime[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3Ph.timer);
}
