/**
 * @file egtm_atom_3_phase_inverter_pwm.h
 * @brief Public API for eGTM ATOM 3-phase inverter PWM driver (TC4xx).
 *
 * This header exposes initialization and periodic update functions only.
 * No includes, types, or macros are defined here by design.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize 3-phase inverter PWM on eGTM ATOM using unified IfxEgtm_Pwm driver.
 */
void initEgtmAtom3phInv(void);

/**
 * @brief Duty-cycle ramp update for 3-phase inverter PWM (percent-based).
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
