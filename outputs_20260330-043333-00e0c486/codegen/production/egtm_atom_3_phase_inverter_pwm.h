/**
 * @file egtm_atom_3_phase_inverter_pwm.h
 * @brief EGTM ATOM 3-Phase Inverter PWM public API
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize eGTM ATOM0 Cluster 0 complementary, center-aligned 3-phase PWM */
void initEgtmAtom3phInv(void);

/** Update 3-phase duties in 10% steps with wrap-around and apply immediately */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
