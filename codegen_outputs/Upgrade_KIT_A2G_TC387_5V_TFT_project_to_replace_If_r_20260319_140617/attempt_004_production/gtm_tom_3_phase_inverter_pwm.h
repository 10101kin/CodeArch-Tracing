/*
 * gtm_tom_3_phase_inverter_pwm.h
 * GTM TOM 3-Phase Inverter PWM - production interface
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =====================================================================
 * Requirements-derived configuration macros (must match requirements)
 * ===================================================================== */
#define INITIAL_DUTY_PERCENT_U                     (25.0f)
#define INITIAL_DUTY_PERCENT_V                     (50.0f)
#define INITIAL_DUTY_PERCENT_W                     (75.0f)

#define DUTY_UPDATE_POLICY_UPDATE_INTERVAL_MS       (500u)
#define DUTY_UPDATE_POLICY_INCREMENT_PERCENT        (10.0f)
#define DUTY_UPDATE_POLICY_WRAP_0_TO_100            (1u) /* boolean TRUE */

/* Extreme duty handling policy: clamp to minPulse and 2*deadtime */
#define DUTY_UPDATE_POLICY_EXTREME_DUTY_HANDLING_POLICY_clamp_to_minPulse_and_deadtime   (1u)

#define TIMING_PWM_FREQUENCY_HZ                    (20000.0f)
#define TIMING_DEADTIME_US                         (0.5f)
#define TIMING_MIN_PULSE_US                        (1.0f)
#define TIMING_USE_HARDWARE_DTM                    (1u)
#define TIMING_UPDATE_SYNCHRONOUS_TGC              (1u)

#define CLOCK_REQUIRES_XTAL                        (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ             (300.0f)
#define CLOCK_GTM_FXCLK0_FREQ_MHZ                  (100.0f)

/* =====================================================================
 * Public API (EXACT names/signatures from SW Detailed Design)
 * ===================================================================== */

/**
 * Initialize the GTM for 3-phase complementary, center-aligned PWM using TOM.
 * Sequence per SW Detailed Design and iLLD reference:
 * 1) IfxGtm_enable
 * 2) IfxGtm_Cmu_enableClocks / set GCLK and CLK0 (fxclk0) to required frequencies
 * 3) Configure and initialize TOM timer and PWMHL with dead-time and min-pulse
 * 4) Program initial duties 25/50/75 as a synchronous update
 * 5) Start timer so updates apply at the next boundary
 */
void IfxGtm_Tom_PwmHl_init(void);

/**
 * Runtime duty update per behavior_description:
 * - Increment U/V/W duties by +10 percentage points (normalized 0..1 add +0.10)
 * - Wrap values that reach/exceed 1.0 to 0.0
 * - Compute safeMinDuty = (minPulse + 2*deadtime)/period and safeMaxDuty = 1 - safeMinDuty
 * - Clamp each requested duty into [safeMinDuty, safeMaxDuty]
 * - Apply synchronously (non-immediate) so all phases update together
 */
void IfxGtm_Tom_PwmHl_setDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
