/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for eGTM ATOM 3-Phase Inverter PWM driver (TC4xx).
 *
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the 3-phase complementary, center-aligned PWM on eGTM ATOM0, Cluster 0 */
void initEgtmAtom3phInv(void);

/* Periodic duty-cycle stepper that updates all channels synchronously */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
