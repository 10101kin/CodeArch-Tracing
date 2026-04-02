/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-phase inverter PWM driver (TC4xx).
 *
 * This header intentionally exposes only the public functions.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize EGTM ATOM PWM for 3-phase complementary inverter and ADC trigger channel */
void initEgtmAtom3phInv(void);

/* Update 3-phase duty cycles immediately and synchronously (percent 0..100) */
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
