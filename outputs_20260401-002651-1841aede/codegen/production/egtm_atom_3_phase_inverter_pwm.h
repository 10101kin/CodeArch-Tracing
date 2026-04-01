/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-Phase Inverter PWM driver (TC4xx).
 *
 * Note: Do not include watchdog or CPU start-up code here.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize a 3-channel center-aligned complementary PWM on EGTM ATOM0 (Cluster 0).
 */
void initEgtmAtom3phInv(void);

/**
 * Step and update duty cycles for all three PWM channels.
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
