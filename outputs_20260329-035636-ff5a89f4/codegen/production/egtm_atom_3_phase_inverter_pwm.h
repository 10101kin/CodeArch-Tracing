/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM three-phase inverter PWM driver (TC4xx).
 *
 * Note:
 *  - Do NOT include headers or macros here. Keep this header minimal per guidelines.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize eGTM Cluster 1 ATOM1 3-phase complementary center-aligned PWM
 *  - 20 kHz switching, 1.0 us dead-time
 *  - Synchronous start and update
 *  - Period ISR on CPU0 with priority 20
 */
void initEgtmAtom3phInv(void);

/** Update three phase duties by a fixed percent step and apply immediately */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
