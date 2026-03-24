/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief GTM TOM 3-Phase Inverter PWM driver (TC387)
 *
 * Production-ready module implementing a 3-phase complementary PWM using GTM TOM and PwmHl.
 * Follows iLLD reference initialization and update patterns with proper error handling.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Requirements-driven configuration macros
 */
#define TIMING_PWM_FREQUENCY_HZ                 (20000.0f)   /* 20 kHz */
#define TIMING_DUTY_STEP_PERCENT                (10.0f)      /* +10% per update call */
#define TIMING_STEP_INTERVAL_MS                 (500U)       /* Caller handles timing */
#define TIMING_APPLY_UPDATES_AT_PERIOD_BOUNDARY (1)          /* Shadow/update sequence */
#define CLOCK_REQUIRES_XTAL                     (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ          (300U)

/* Derived helper for on-time computations (fraction) */
#define DUTY_STEP                               (0.10f)      /* 10% as fraction */
#define DUTY_25_PERCENT                         (0.25f)
#define DUTY_50_PERCENT                         (0.50f)
#define DUTY_75_PERCENT                         (0.75f)

/* Dead-time and minimum pulse (microseconds as in reference usage) */
#define PWM_DEAD_TIME                           (0.5f)       /* 0.5 us */
#define PWM_MIN_PULSE_TIME                      (1.0f)       /* 1.0 us */

/* Public API as per SW Detailed Design */
void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
