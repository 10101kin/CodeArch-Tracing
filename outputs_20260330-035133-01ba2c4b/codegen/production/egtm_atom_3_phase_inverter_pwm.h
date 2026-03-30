/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for eGTM ATOM 3-phase complementary PWM driver (TC4xx).
 *
 * Do not place includes, typedefs, or macros in this header.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
