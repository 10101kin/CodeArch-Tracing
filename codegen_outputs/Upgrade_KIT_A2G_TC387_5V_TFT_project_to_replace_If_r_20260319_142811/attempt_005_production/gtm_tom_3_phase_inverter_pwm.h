/*
 * Module: GTM_TOM_3_Phase_Inverter_PWM
 * File: gtm_tom_3_phase_inverter_pwm.h
 *
 * Production-ready 3-phase inverter PWM using GTM TOM with IfxGtm_Tom_PwmHl.
 * Target: TC3xx (TC387)
 *
 * Notes:
 * - Uses TOM1 with CH0 as master timer and complementary pairs for U/V/W phases.
 * - Center-aligned PWM at 20 kHz, DTM-based dead-time, and min pulse enforcement.
 * - Public API names preserved exactly per SW Detailed Design contract.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CRITICAL: API CONTRACT — preserve function signatures EXACTLY
 * These functions implement the behavior specified in the SW Detailed Design.
 */

/* Initialize GTM TOM 3-phase complementary PWM (20 kHz, center-aligned). */
void IfxGtm_Tom_PwmHl_init(void);

/*
 * Runtime duty update per behavior description:
 * - Increment each phase duty by +10 percentage points (0.10 normalized)
 * - Wrap at 1.0 -> 0.0
 * - Clamp to safe range considering minPulse and 2*deadtime
 * - Apply synchronously at the next timer boundary
 */
void IfxGtm_Tom_PwmHl_setDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
