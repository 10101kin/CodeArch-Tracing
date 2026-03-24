/*
 * egtm_atom_3_phase_inverter.h
 *
 * TC4xx eGTM ATOM 3-Phase Inverter PWM driver (production)
 *
 * Implements unified IfxEgtm_Pwm-based 3-phase complementary PWM with CDTM dead-time,
 * plus ADC trigger routing and LED GPIO per requirements.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_H
#define EGTM_ATOM_3_PHASE_INVERTER_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public API (exact signatures from SW Detailed Design) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxEgtm_periodEventFunction(void *data);
void interruptEgtmAtom(void);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_H */
