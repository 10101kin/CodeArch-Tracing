/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-phase inverter PWM + ADC trigger (TC4xx).
 *
 * Note:
 *  - Watchdog disable must be done only in CpuX main files (IfxWtu_* APIs).
 *  - This header exposes only the public functions; ISR and callbacks are internal.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize EGTM PWM for 3-phase inverter (ATOM0 CH0..2) and ADC trigger (ATOM0 CH3). */
void initEgtmAtom3phInv(void);

/** Update the three inverter phase duties immediately and synchronously (percent 0..100). */
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
