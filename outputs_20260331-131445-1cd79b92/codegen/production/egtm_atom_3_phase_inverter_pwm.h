/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-phase inverter PWM driver (TC4xx).
 *
 * NOTE:
 *  - This header intentionally contains only the public function prototypes.
 *  - All configuration, macros, and internal state are defined in the .c file.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize EGTM CLS0 ATOM resources for 3-phase complementary PWM and ATOM0 CH3 ADC trigger. */
void initEgtmAtom3phInv(void);

/** Update three logical PWM channel duties immediately (percent values in [0..100]). */
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

/** ADC conversion-complete ISR hook: toggles LED pin and returns. */
void resultISR(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
