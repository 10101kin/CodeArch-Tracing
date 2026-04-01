/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public interface for EGTM ATOM 3-Phase Inverter PWM driver (TC4xx, unified IfxEgtm_Pwm)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize 3-phase complementary, center-aligned PWM with synchronous start/update */
void initEgtmAtom3phInv(void);

/** Update U/V/W duties with fixed step and apply immediately (bulk, synchronous) */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
