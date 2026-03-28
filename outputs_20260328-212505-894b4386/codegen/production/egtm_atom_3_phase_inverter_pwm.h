/**
 * @file egtm_atom_3_phase_inverter_pwm.h
 * @brief Public API for eGTM ATOM 3-phase inverter PWM driver (TC4xx, unified IfxEgtm_Pwm)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize eGTM ATOM 3-phase inverter PWM */
void initEgtmAtom3phInv(void);

/** Update duty cycles using immediate apply (percent-based ramp) */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
