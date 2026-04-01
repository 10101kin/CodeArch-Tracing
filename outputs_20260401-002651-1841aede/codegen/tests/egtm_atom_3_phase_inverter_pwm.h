/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-Phase Inverter PWM driver (TC4xx).
 *
 * Note:
 *  - Do not include headers or macros here. Prototypes only.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize EGTM ATOM0 Cluster 0 center-aligned complementary PWM (3 channels). */
void initEgtmAtom3phInv(void);

/** Update all three phase duties with a fixed step and wrap at 100%. */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
