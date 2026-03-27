/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM_ATOM_3_Phase_Inverter_PWM driver (TC4xx, eGTM unified PWM)
 *
 * Note: This header intentionally contains only include guards and function prototypes.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize a 3-phase inverter PWM using eGTM unified PWM on Cluster 1 ATOM1.
 * - 3 channels (0,1,2) with complementary outputs
 * - Center-aligned 20 kHz PWM
 * - 1.0 us rising/falling dead-time via DTM
 * - Synchronized start and update enabled
 * - Period-event interrupt configured on channel 0 (CPU0, prio 20)
 */
void initEgtmAtom3phInv(void);

/**
 * Perform a synchronous duty step on all three PWM channels and apply immediately.
 * Duty values wrap to 0 when (duty + step) >= 100, then the step is added.
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
