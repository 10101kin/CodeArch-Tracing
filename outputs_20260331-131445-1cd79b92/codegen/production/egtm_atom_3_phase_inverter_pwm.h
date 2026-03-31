/*
 * egtm_atom_3_phase_inverter_pwm.h
 * Public API for EGTM ATOM 3-phase inverter PWM driver (TC4xx)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Public function prototypes (no includes in header) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);
void resultISR(void);

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
