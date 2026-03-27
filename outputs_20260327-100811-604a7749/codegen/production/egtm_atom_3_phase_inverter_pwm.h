/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM_ATOM_3_Phase_Inverter_PWM driver (TC4xx, eGTM ATOM, unified IfxEgtm_Pwm)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize eGTM Cluster 1 ATOM1 3-phase inverter PWM (complementary outputs, center-aligned, 20 kHz) */
void initEgtmAtom3phInv(void);

/** Update duty cycles synchronously across all 3 channels (wrap and increment by PHASE_DUTY_STEP) */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
