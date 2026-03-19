/*
 * GTM TOM 3-Phase Inverter PWM - Production Source
 * Target: AURIX TC3xx (TC387)
 * Peripheral: GTM TOM PWM (PwmHl)
 *
 * This implementation follows the SW Detailed Design and authoritative iLLD
 * initialization patterns. It uses real iLLD drivers and checks all return values.
 *
 * Forbidden: Watchdog disable calls in this file. These belong only in CpuN_Main.c.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"  /* Generic header - family-specific headers are forbidden */
#include "IfxPort.h"

/* =============================
 * Pin assignments (KIT_A2G_TC387_5V_TFT request)
 *  - TOM1 channels for 3-phase complementary outputs
 *  - U: TOM1 CH2 (HS)/P00.3 & CH1 (LS)/P00.2
 *  - V: TOM1 CH4 (HS)/P00.5 & CH3 (LS)/P00.4
 *  - W: TOM1 CH6 (HS)/P00.7 & CH5 (LS)/P00.6
 * ============================= */
#define PHASE_U_HS   &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS   &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS   &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS   &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS   &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS   &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* Master TOM and channel (TOM1 CH0 as master timer, per requirements) */
#define GTM_TOM_MASTER              (IfxGtm_Tom_1)
#define GTM_TOM_MASTER_TIMER_CH     (IfxGtm_Tom_Ch_0)

/* Channel count */
#define NUM_OF_CHANNELS             (3u)

/* =============================
 * Internal driver state
 * ============================= */

typedef struct
{
    IfxGtm_Tom_Timer   timer;                     /* TOM master timer */
    IfxGtm_Tom_PwmHl   pwm;                       /* TOM PwmHl driver */
    Ifx_TimerValue     pwmOnTimes[NUM_OF_CHANNELS];
    float32            dutyNorm[NUM_OF_CHANNELS];  /* 0.0 .. 1.0 normalized duties */
} GtmTom3PhasePwm_Driver;

static GtmTom3PhasePwm_Driver s_drv;
static boolean                s_initialized = FALSE;

/* =============================
 * Local utilities
 * ============================= */
static inline float32 gtm_sec_from_us(float32 us)
{
    return (us * 1.0e-6f);
}

static inline float32 gtm_clampf(float32 v, float32 lo, float32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

/* =============================
 * Public API implementations
 * ============================= */

void IfxGtm_Tom_PwmHl_init(void)
{
    /* Do not proceed twice */
    s_initialized = FALSE;

    /* 1) Enable GTM and set CMU clocks (FXCLK0 from CLK0) */
    IfxGtm_enable(&MODULE_GTM);

    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        /* Use module cluster frequency for GCLK, set CLK0 to required 100 MHz */
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, CLOCK_GTM_FXCLK0_FREQ_HZ);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Configure TOM master timer (TOM1 CH0, center-aligned base @ 20 kHz) */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

        timerCfg.base.frequency  = PWM_FREQUENCY_HZ;
        timerCfg.clock           = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
        timerCfg.tom             = GTM_TOM_MASTER;
        timerCfg.timerChannel    = GTM_TOM_MASTER_TIMER_CH;

        if (IfxGtm_Tom_Timer_init(&s_drv.timer, &timerCfg) == FALSE)
        {
            return; /* Early exit on failure */
        }
    }

    /* 3) Configure PwmHl (complementary outputs, dead-time, min pulse, pins) */
    {
        IfxGtm_Tom_PwmHl_Config pwmHlCfg;
        IfxGtm_Tom_ToutMapP ccx[NUM_OF_CHANNELS] =
        {
            PHASE_U_HS,
            PHASE_V_HS,
            PHASE_W_HS
        };
        IfxGtm_Tom_ToutMapP coutx[NUM_OF_CHANNELS] =
        {
            PHASE_U_LS,
            PHASE_V_LS,
            PHASE_W_LS
        };

        IfxGtm_Tom_PwmHl_initConfig(&pwmHlCfg);

        pwmHlCfg.base.channelCount    = NUM_OF_CHANNELS;
        pwmHlCfg.base.deadtime        = gtm_sec_from_us(PWM_DEADTIME_US); /* seconds */
        pwmHlCfg.base.minPulse        = gtm_sec_from_us(PWM_MIN_PULSE_US); /* seconds */
        pwmHlCfg.base.outputMode      = IfxPort_OutputMode_pushPull;
        pwmHlCfg.base.outputDriver    = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        pwmHlCfg.base.ccxActiveState  = Ifx_ActiveState_high;
        pwmHlCfg.base.coutxActiveState= Ifx_ActiveState_high;

        pwmHlCfg.ccx                  = ccx;
        pwmHlCfg.coutx                = coutx;
        pwmHlCfg.timer                = &s_drv.timer;
        pwmHlCfg.tom                  = GTM_TOM_MASTER;

        if (IfxGtm_Tom_PwmHl_init(&s_drv.pwm, &pwmHlCfg) == FALSE)
        {
            return; /* Early exit on failure */
        }

        if (IfxGtm_Tom_PwmHl_setMode(&s_drv.pwm, Ifx_Pwm_Mode_centerAligned) == FALSE)
        {
            return; /* Early exit on failure */
        }

        /* Reflect input frequency changes into driver (recommended pattern) */
        IfxGtm_Tom_Timer_updateInputFrequency(&s_drv.timer);
    }

    /* 4) Start timer */
    IfxGtm_Tom_Timer_run(&s_drv.timer);

    /* 5) Initialize duty states and program initial on-times (25%, 50%, 75%) */
    s_drv.dutyNorm[0] = INITIAL_DUTY_PERCENT_U / 100.0f;
    s_drv.dutyNorm[1] = INITIAL_DUTY_PERCENT_V / 100.0f;
    s_drv.dutyNorm[2] = INITIAL_DUTY_PERCENT_W / 100.0f;

    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
        s_drv.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[0]);
        s_drv.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[1]);
        s_drv.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[2]);

        /* Apply synchronously at next timer boundary */
        IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
    }

    s_initialized = TRUE;
}

void IfxGtm_Tom_PwmHl_setDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit if init failed or not called yet */
    }

    /* 1) Increment each duty by +10 percentage points and wrap at 100% -> 0% */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        s_drv.dutyNorm[i] += (DUTY_INCREMENT_PERCENT / 100.0f);
        if (s_drv.dutyNorm[i] >= 1.0f)
        {
            s_drv.dutyNorm[i] = 0.0f;
        }
    }

    /* 2) Compute safe clamping range from requirements */
    const float32 period_s   = 1.0f / PWM_FREQUENCY_HZ; /* From configured 20 kHz */
    const float32 deadtime_s = gtm_sec_from_us(PWM_DEADTIME_US);
    const float32 minPulse_s = gtm_sec_from_us(PWM_MIN_PULSE_US);

    const float32 safeMin = gtm_clampf((minPulse_s + (2.0f * deadtime_s)) / period_s, 0.0f, 0.5f);
    const float32 safeMax = 1.0f - safeMin;

    /* 3) Clamp duties to [safeMin, safeMax] */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        s_drv.dutyNorm[i] = gtm_clampf(s_drv.dutyNorm[i], safeMin, safeMax);
    }

    /* 4) Convert to on-times (ticks) and apply synchronously */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
        s_drv.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[0]);
        s_drv.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[1]);
        s_drv.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * s_drv.dutyNorm[2]);

        IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.pwmOnTimes);
        IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
    }
}
