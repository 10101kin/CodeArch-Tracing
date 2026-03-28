/**
 * @file egtm_atom_3_phase_inverter_pwm.h
 * @brief eGTM ATOM 3-phase inverter PWM driver (TC4xx, unified IfxEgtm_Pwm)
 *
 * Public API: initialization and duty update.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
