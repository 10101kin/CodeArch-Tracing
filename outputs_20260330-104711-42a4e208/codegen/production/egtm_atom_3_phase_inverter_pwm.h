/*
 * egtm_atom_3_phase_inverter_pwm.h
 * Public API for EGTM ATOM 3-Phase Inverter PWM driver
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize a 3-channel complementary, center-aligned PWM for a three-phase inverter
 * on eGTM ATOM0, Cluster 0 using the unified high-level driver. See source for details.
 */
void initEgtmAtom3phInv(void);

/**
 * Periodic duty-cycle stepper for all three logical PWM channels (U, V, W).
 * Implements 10% step with wrap-at-100 behavior, then applies new duty immediately.
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
