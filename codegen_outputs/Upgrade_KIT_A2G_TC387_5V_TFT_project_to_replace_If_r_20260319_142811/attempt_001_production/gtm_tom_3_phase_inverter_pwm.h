/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Production 3-phase inverter PWM using GTM TOM + PwmHl (TC3xx)
 *
 * Implements the SW Detailed Design API contract:
 *   - void IfxGtm_Tom_PwmHl_init(void);
 *   - void IfxGtm_Tom_PwmHl_setDuty(void);
 *
 * Notes:
 * - Uses IfxGtm_Tom_Timer and IfxGtm_Tom_PwmHl iLLD drivers (real APIs)
 * - 3 complementary channels (U, V, W) on TOM1 per user requirement
 * - 20 kHz center-aligned PWM, hardware DTM dead-time and min-pulse
 * - Initial duties: 25% / 50% / 75%
 * - Update function increments +10 percentage points per call with wrap and
 *   clamps to respect minimum pulse and 2*deadtime
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Requirement-derived configuration values */
#define TIMING_PWM_FREQUENCY_HZ                       (20000.0f)   /* 20 kHz */
#define TIMING_DEADTIME_US                            (0.5f)       /* 0.5 us */
#define TIMING_MIN_PULSE_US                           (1.0f)       /* 1.0 us */
#define TIMING_UPDATE_SYNCHRONOUS_TGC                 (1)          /* TRUE */
#define CLOCK_GTM_FXCLK0_FREQ_MHZ                     (100.0f)     /* Target FXCLK0 (informational) */

/* Initial duty settings in percent [0..100] as per requirement */
#define INITIAL_DUTY_PERCENT_U                        (25.0f)
#define INITIAL_DUTY_PERCENT_V                        (50.0f)
#define INITIAL_DUTY_PERCENT_W                        (75.0f)

/* Duty update policy (caller controls timing at 500 ms period) */
#define DUTY_UPDATE_POLICY_UPDATE_INTERVAL_MS         (500u)
#define DUTY_UPDATE_POLICY_INCREMENT_PERCENT          (10.0f)      /* +10 percentage points */
#define DUTY_UPDATE_POLICY_WRAP_0_TO_100              (1)          /* TRUE */

/* Extreme duty handling policy: clamp to minPulse/2*deadtime */
#define DUTY_UPDATE_POLICY_EXTREME_DUTY_HANDLING_POLICY_CLAMP       (1)

/* Public API - EXACT signatures per SW Detailed Design */
/**
 * Initialize GTM TOM for 3-phase complementary center-aligned PWM using PwmHl.
 * Sequence (as described in SW Detailed Design):
 *  1) IfxGtm_enable; 2) IfxGtm_Cmu_enableClocks (FXCLK);
 *  3) Configure TOM1 master timer (CH0) @ 20 kHz on Fxclk0;
 *  4) Initialize PwmHl with complementary outputs and set deadtime/minPulse;
 *  5) Program initial duties (25/50/75%) using synchronous update; 6) Run timer.
 */
void IfxGtm_Tom_PwmHl_init(void);

/**
 * Runtime duty update per SW Detailed Design:
 *  - Increment each duty by +10 percentage points; if >= 100%, wrap to 0%
 *  - Compute safe min fraction as (minPulse + 2*deadtime) / period
 *  - Clamp requested duties to [safeMin, 1 - safeMin]
 *  - Apply via synchronous update so all phases update at the same boundary
 * Caller must invoke at 500 ms intervals (no internal timing/gating here).
 */
void IfxGtm_Tom_PwmHl_setDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
