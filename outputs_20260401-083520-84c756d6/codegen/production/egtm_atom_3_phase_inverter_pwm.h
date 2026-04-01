/*
 * egtm_atom_3_phase_inverter_pwm.h
 * Public API for EGTM ATOM 3-Phase Inverter PWM driver (TC4xx)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Public function prototypes (no includes, typedefs, or macros here) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
