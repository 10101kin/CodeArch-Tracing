/* ============================================================================
 *  File: egtm_atom_3_phase_inverter_pwm.h
 *  Brief: Public API for eGTM ATOM 3-phase inverter PWM driver (TC4xx)
 * ============================================================================ */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize eGTM Cluster1 ATOM1 3-phase complementary, center-aligned PWM */
void initEgtmAtom3phInv(void);

/** Update the three phase duty cycles with a fixed step (immediate update) */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
