#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the GTM for 3-phase complementary, center-aligned PWM using TOM1 with
 * hardware dead-time and minimum pulse enforcement.
 *
 * Behavior per SW Detailed Design:
 * 1) IfxGtm_enable
 * 2) IfxGtm_Cmu_enableClocks (FXCLK) and configure FXCLK0 to 100 MHz
 * 3) Prepare TOM master timer on TOM1 CH0 @ 20 kHz (fxclk0 as time base)
 * 4) Initialize IfxGtm_Tom_PwmHl with complementary outputs and DTM
 * 5) Configure dead-time = 0.5 us, minPulse = 1.0 us
 * 6) Program initial duties 25%/50%/75% (applied synchronously)
 * 7) Start PWM so updates are applied on next timer boundary
 */
void IfxGtm_Tom_PwmHl_init(void);

/**
 * Update phase duties by +10 percentage points each call with wrap and safe clamping.
 *
 * Behavior per SW Detailed Design:
 * - Maintain persistent duties (U/V/W) as normalized [0.0..1.0]
 * - Each call: duty += 0.10; if duty >= 1.0 -> wrap to 0.0
 * - Compute period_s from configured 20 kHz
 * - safeMin = (minPulse + 2*deadtime)/period; safeMax = (1.0 - safeMin)
 * - Clamp each duty into [safeMin, safeMax]
 * - Apply synchronously so all phases update together at next timer event
 */
void IfxGtm_Tom_PwmHl_setDuty(void);

#ifdef __cplusplus
}
#endif

#endif // GTM_TOM_3_PHASE_INVERTER_PWM_H
