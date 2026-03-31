/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-Phase Inverter PWM driver (TC4xx).
 *
 * This header exposes only the initialization and runtime update functions.
 * ISR and internal callback are private to the driver implementation.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the eGTM ATOM-based 3-phase complementary, center-aligned PWM.
 * - Synchronous start and update enabled
 * - 20 kHz switching frequency
 * - 1 us rising/falling dead-time
 * - Period-event interrupt on base channel (channel 0)
 */
void initEgtmAtom3phInv(void);

/**
 * Update duty cycles of all three phases by a fixed step with wrap-around, and
 * apply immediately in a single synchronous update.
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
