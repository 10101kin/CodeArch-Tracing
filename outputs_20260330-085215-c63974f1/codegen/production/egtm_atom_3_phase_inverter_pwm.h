/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-Phase Inverter PWM driver (TC4xx)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize eGTM ATOM0 (Cluster 0) 3-phase complementary PWM at 20 kHz */
void initEgtmAtom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
