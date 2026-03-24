/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Production module: GTM_TOM_3_Phase_Inverter_PWM
 * Target: AURIX TC3xx (TC387)
 * Peripheral: GTM TOM PWM (unified IfxGtm_Pwm driver, single-output)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= Requirements-driven configuration ========================= */
#define NUM_OF_CHANNELS                    (3U)

/* From requirements (highest priority) */
#define TIMEBASE_CHANNEL                   TOM1_CH0
#define TIMEBASE_ALIGNMENT                 center
#define TIMEBASE_FREQUENCY_HZ              (20000.0f)
#define TIMING_PWM_FREQUENCY_HZ            (20000.0f)
#define TIMING_DUTY_UPDATE_PERIOD_MS       (500U)
#define TIMING_DUTY_STEP_PERCENT           (10.0f)
#define TIMING_WRAPAROUND_AT_100_TO_0      (1U)
#define TIMING_CPU0_WAIT_TIME_MS           (500U)
#define CLOCK_REQUIRES_XTAL                (1U)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ     (300U)

/* Initial duties in percent (single-ended) */
#define DUTY_U_INIT_PERCENT                (25.0f)
#define DUTY_V_INIT_PERCENT                (50.0f)
#define DUTY_W_INIT_PERCENT                (75.0f)

/* ========================= Public API (from SW Detailed Design) ========================= */
/**
 * Initialize GTM TOM-based 3-channel PWM using unified IfxGtm_Pwm driver.
 * Behavior per SW Detailed Design:
 *  - Enable GTM and functional clocks
 *  - Configure 20 kHz, center-aligned, CMU Fxclk0, TOM1 timebase
 *  - Configure three synchronized channels: TOM1 CH2/CH4/CH6 routed to P02.0/P02.1/P02.2
 *  - Initialize PWM driver and start synchronized channels
 *  - Apply initial duties: U=25%, V=50%, W=75% via immediate multi-channel update
 */
void initGtmTom3PhaseInverterPwm(void);

/**
 * Runtime duty update. Behavior per SW Detailed Design:
 *  - Increase each duty by +10 percentage points
 *  - If any duty >= 100%, wrap to 0%
 *  - Apply immediately via multi-channel duty update
 * Intended to be called every 500 ms by the caller (Cpu0 main loop).
 */
void updateGtmTom3PhaseDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
