/*
 * GTM TOM 3-Phase Inverter PWM - Production Header
 * Target: AURIX TC3xx (TC387)
 * Peripheral: GTM TOM PWM (PwmHl)
 *
 * This module initializes and updates a 3-phase complementary, center-aligned
 * PWM using the documented IfxGtm_Tom_PwmHl driver. It follows the SW Detailed
 * Design API contract exactly:
 *   - IfxGtm_Tom_PwmHl_init()
 *   - IfxGtm_Tom_PwmHl_setDuty()
 *
 * Watchdog handling must NOT be placed in this driver (CpuN_Main.c only).
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================
 * Requirements-driven constants
 * ============================= */
#define PWM_FREQUENCY_HZ                          (20000.0f)   /* TIMING_PWM_FREQUENCY_HZ */
#define PWM_DEADTIME_US                           (0.5f)       /* TIMING_DEADTIME_US */
#define PWM_MIN_PULSE_US                          (1.0f)       /* TIMING_MIN_PULSE_US */
#define DUTY_UPDATE_INTERVAL_MS                   (500u)       /* DUTY_UPDATE_POLICY_UPDATE_INTERVAL_MS - handled by caller timing */
#define DUTY_INCREMENT_PERCENT                    (10.0f)      /* DUTY_UPDATE_POLICY_INCREMENT_PERCENT */
#define DUTY_WRAP_0_TO_100                        (1u)         /* DUTY_UPDATE_POLICY_WRAP_0_TO_100 */
#define CLOCK_GTM_FXCLK0_FREQ_HZ                  (100000000.0f) /* CLOCK_GTM_FXCLK0_FREQ_MHZ */

/* Initial duties (percent) from requirements */
#define INITIAL_DUTY_PERCENT_U                    (25.0f)
#define INITIAL_DUTY_PERCENT_V                    (50.0f)
#define INITIAL_DUTY_PERCENT_W                    (75.0f)

/* Convenience fractional macros used internally (0.0..1.0) */
#define DUTY_25_PERCENT                           (0.25f)
#define DUTY_50_PERCENT                           (0.50f)
#define DUTY_75_PERCENT                           (0.75f)

/* Minimum/maximum duty placeholders for reference logic (not hard limits) */
#define DUTY_STEP                                 (0.1f)   /* +10 percentage points as fraction */
#define DUTY_MIN                                  (0.1f)
#define DUTY_MAX                                  (0.9f)

/* =============================
 * Public API (Exact Signatures)
 * ============================= */

/**
 * Initialize the GTM for 3-phase complementary, center-aligned PWM using TOM PwmHl.
 * Behavior per SW Detailed Design (init sequence and configuration).
 *
 * Pre-conditions:
 *  - GTM module clock available; watchdog handling is performed in CpuN_Main.c.
 *  - Caller ensures single-initialization at startup.
 */
void IfxGtm_Tom_PwmHl_init(void);

/**
 * Runtime duty update function. On each call:
 *  - Increment U/V/W duties by +10 percentage points, wrap at 100% to 0%
 *  - Compute safeMin = (minPulse + 2*deadtime)/period, safeMax = 1 - safeMin
 *  - Clamp duties to [safeMin, safeMax]
 *  - Apply synchronously using shadow-update at the next timer event
 *
 * Note: Caller controls the call period (e.g., every 500 ms) in Cpu0_Main.c.
 */
void IfxGtm_Tom_PwmHl_setDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
