/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public APIs for EGTM ATOM 3-phase inverter PWM driver (TC4xx).
 *
 * Note:
 *  - Cpu0_Main.c must handle watchdog configuration.
 *  - This header intentionally contains only public function prototypes.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize 3-phase complementary center-aligned PWM on EGTM ATOM0 (cluster 0). */
void initEgtmAtom3phInv(void);

/** Update duty of all three phases with fixed step (wrap at 100%). */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
