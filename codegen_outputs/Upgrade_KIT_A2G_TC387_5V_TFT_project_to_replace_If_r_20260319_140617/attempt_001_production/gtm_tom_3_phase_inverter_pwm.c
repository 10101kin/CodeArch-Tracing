#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"  // Generic pin map header
#include "IfxPort.h"

// =============================
// Requirements-derived constants
// =============================
#define TIMING_PWM_FREQUENCY_HZ                 (20000.0f)   // 20 kHz
#define TIMING_DEADTIME_US                      (0.5f)       // 0.5 us
#define TIMING_MIN_PULSE_US                     (1.0f)       // 1.0 us
#define CLOCK_GTM_FXCLK0_FREQ_HZ                (100000000.0f) // 100 MHz FXCLK0

#define INITIAL_DUTY_PERCENT_U                  (25.0f)
#define INITIAL_DUTY_PERCENT_V                  (50.0f)
#define INITIAL_DUTY_PERCENT_W                  (75.0f)
#define DUTY_UPDATE_POLICY_INCREMENT_PERCENT    (10.0f)

// Convert percent to normalized fraction
#define PERCENT_TO_FRAC(pct)   ((float32)((pct) * (1.0f / 100.0f)))

// Number of complementary phase pairs
#define NUM_OF_CHANNELS                         (3U)

// =============================
// Pin assignments (TOM1, KIT_A2G_TC387_5V_TFT style)
// U: TOM1 CH2 (P00.3) HS, CH1 (P00.2) LS
// V: TOM1 CH4 (P00.5) HS, CH3 (P00.4) LS
// W: TOM1 CH6 (P00.7) HS, CH5 (P00.6) LS
// =============================
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

// =============================
// Internal driver state
// =============================
typedef struct
{
    IfxGtm_Tom_Timer   timer;                    // Master TOM1 timer (CH0)
    IfxGtm_Tom_PwmHl   pwm;                      // Complementary PWM driver
    Ifx_TimerValue     pwmOnTimes[NUM_OF_CHANNELS];
    float32            dutyFrac[NUM_OF_CHANNELS]; // Normalized [0..1]
    float32            deadtime_s;
    float32            minPulse_s;
    float32            frequency_Hz;
} GtmTom3PhaseInv_Driver;

static GtmTom3PhaseInv_Driver s_drv;
static boolean                s_initialized = FALSE;

// =============================
// Local helpers
// =============================
static inline float32 clampf(float32 v, float32 lo, float32 hi)
{
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

// =============================
// Public API implementations
// =============================
void IfxGtm_Tom_PwmHl_init(void)
{
    s_initialized = FALSE;

    // 1) Enable GTM module
    IfxGtm_enable(&MODULE_GTM);

    // 2) Configure CMU clocks: set GCLK to module frequency, FXCLK0 to 100 MHz, enable FXCLK
    IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM));
    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, CLOCK_GTM_FXCLK0_FREQ_HZ);
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    // 3) Master timer on TOM1 CH0 @ 20 kHz using FXCLK0
    IfxGtm_Tom_Timer_Config timerCfg;
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
    timerCfg.base.frequency = TIMING_PWM_FREQUENCY_HZ;
    timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
    timerCfg.tom            = IfxGtm_Tom_1;
    timerCfg.timerChannel   = IfxGtm_Tom_Ch_0;

    if (IfxGtm_Tom_Timer_init(&s_drv.timer, &timerCfg) == FALSE)
    {
        return; // HW init failed; remain not initialized
    }

    // 4) Complementary PWM HL setup (3 pairs U/V/W)
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
    pwmHlCfg.base.channelCount     = (uint8)(sizeof(ccx) / sizeof(ccx[0]));
    pwmHlCfg.base.deadtime         = (float32)(TIMING_DEADTIME_US * 1.0e-6f); // seconds
    pwmHlCfg.base.minPulse         = (float32)(TIMING_MIN_PULSE_US * 1.0e-6f); // seconds
    pwmHlCfg.base.outputMode       = IfxPort_OutputMode_pushPull;
    pwmHlCfg.base.outputDriver     = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    pwmHlCfg.base.ccxActiveState   = Ifx_ActiveState_high;
    pwmHlCfg.base.coutxActiveState = Ifx_ActiveState_high;
    pwmHlCfg.ccx                   = ccx;
    pwmHlCfg.coutx                 = coutx;
    pwmHlCfg.timer                 = &s_drv.timer;
    pwmHlCfg.tom                   = timerCfg.tom;

    if (IfxGtm_Tom_PwmHl_init(&s_drv.pwm, &pwmHlCfg) == FALSE)
    {
        return; // PWM HL init failed
    }

    if (IfxGtm_Tom_PwmHl_setMode(&s_drv.pwm, Ifx_Pwm_Mode_centerAligned) == FALSE)
    {
        return; // Mode configuration failed
    }

    // Update internal timing info and run timer
    IfxGtm_Tom_Timer_updateInputFrequency(&s_drv.timer);
    IfxGtm_Tom_Timer_run(&s_drv.timer);

    // 5) Store configuration values and initialize duties (normalized fractions)
    s_drv.deadtime_s   = (float32)(TIMING_DEADTIME_US * 1.0e-6f);
    s_drv.minPulse_s   = (float32)(TIMING_MIN_PULSE_US * 1.0e-6f);
    s_drv.frequency_Hz = TIMING_PWM_FREQUENCY_HZ;

    s_drv.dutyFrac[0] = PERCENT_TO_FRAC(INITIAL_DUTY_PERCENT_U);
    s_drv.dutyFrac[1] = PERCENT_TO_FRAC(INITIAL_DUTY_PERCENT_V);
    s_drv.dutyFrac[2] = PERCENT_TO_FRAC(INITIAL_DUTY_PERCENT_W);

    // 6) Program initial on-times and apply synchronously
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
        s_drv.pwmOnTimes[0] = (Ifx_TimerValue)((float32)period * s_drv.dutyFrac[0]);
        s_drv.pwmOnTimes[1] = (Ifx_TimerValue)((float32)period * s_drv.dutyFrac[1]);
        s_drv.pwmOnTimes[2] = (Ifx_TimerValue)((float32)period * s_drv.dutyFrac[2]);

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
        return; // Early exit if init failed/not done
    }

    // 1) Increment each duty by +0.10 with wrap at 1.0
    const float32 step = PERCENT_TO_FRAC(DUTY_UPDATE_POLICY_INCREMENT_PERCENT);
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        s_drv.dutyFrac[i] += step;
        if (s_drv.dutyFrac[i] >= 1.0f)
        {
            s_drv.dutyFrac[i] = 0.0f;
        }
    }

    // 2) Compute safe clamping bounds from minPulse and deadtime
    const float32 period_s = (1.0f / s_drv.frequency_Hz);
    const float32 safeMin  = (s_drv.minPulse_s + (2.0f * s_drv.deadtime_s)) / period_s;
    const float32 safeMax  = 1.0f - safeMin;

    // 3) Clamp duties and convert to on-times (ticks)
    const Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&s_drv.timer);
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        const float32 dutyClamped = clampf(s_drv.dutyFrac[i], safeMin, safeMax);
        s_drv.dutyFrac[i]         = dutyClamped; // persist clamped value
        s_drv.pwmOnTimes[i]       = (Ifx_TimerValue)((float32)period * dutyClamped);
    }

    // 4) Apply synchronously at next timer boundary
    IfxGtm_Tom_Timer_disableUpdate(&s_drv.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&s_drv.pwm, s_drv.pwmOnTimes);
    IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);
}
