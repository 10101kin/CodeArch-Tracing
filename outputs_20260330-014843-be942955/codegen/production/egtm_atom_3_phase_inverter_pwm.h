/******************************************************************************
 * @file    egtm_atom_3_phase_inverter_pwm.h
 * @brief   TC4xx eGTM/ATOM 3-phase complementary center-aligned PWM driver API
 * @version 1.0
 ******************************************************************************/
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
