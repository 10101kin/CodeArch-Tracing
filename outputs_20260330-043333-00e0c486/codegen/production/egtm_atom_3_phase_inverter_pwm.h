/*
 * egtm_atom_3_phase_inverter_pwm.h
 * Public API for EGTM ATOM 3-Phase Inverter PWM driver
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize 3-channel complementary, center-aligned PWM on eGTM ATOM0 Cluster 0 */
void initEgtmAtom3phInv(void);

/** Update three phase duties in 10% steps with wrap-around and apply immediately */
void updateEgtmAtom3phInvDuty(void);

/* Note: period-event callback is internal and not declared in the public header */

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
