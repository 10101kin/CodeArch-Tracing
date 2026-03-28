/**************************************************************************
 * @file    egtm_atom_3_phase_inverter_pwm.h
 * @brief   Public API for eGTM ATOM 3-phase inverter PWM driver (TC4xx)
 * @note    This header exposes only the public initialization and update APIs.
 **************************************************************************/
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the eGTM ATOM-based 3-phase inverter PWM (complementary outputs).
 *  - 3 complementary pairs (U, V, W)
 *  - Center-aligned, 20 kHz, ~1 us dead-time
 *  - Sync start and sync update enabled
 */
void initEgtmAtom3phInv(void);

/** Update (ramp) duty-cycles for U, V, W phases in percent using immediate update. */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
