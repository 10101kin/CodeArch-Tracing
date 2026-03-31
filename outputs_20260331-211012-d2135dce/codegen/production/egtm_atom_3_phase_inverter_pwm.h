/*
 * egtm_atom_3_phase_inverter_pwm.h
 * Public interface for EGTM ATOM 3-Phase Inverter PWM driver (TC4xx)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initializes 3-channel complementary, center-aligned PWM using EGTM ATOM */
void initEgtmAtom3phInv(void);

/* Updates the three channel duties with fixed step and applies immediately */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
