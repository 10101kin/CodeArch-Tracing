/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-phase complementary PWM and ADC trigger routing.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize EGTM CLS0 ATOM resources for a 3-phase complementary PWM on logical channels 0..2
 *  at 20 kHz center-aligned with 1 us hardware dead-time, and set up ATOM0 logical channel 3 as a
 *  50% edge-aligned trigger output routed to the ADC trigger fabric.
 */
void initEgtmAtom3phInv(void);

/** Update the three logical PWM channel duties immediately.
 *  requestDuty[0..2] in percent [0..100]. Values are clamped and applied immediately.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

/** ADC conversion-complete interrupt service routine body (toggle LED). */
void resultISR(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
