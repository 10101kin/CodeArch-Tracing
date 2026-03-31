/*
 * egtm_atom_3_phase_inverter_pwm.h
 * Public API for EGTM ATOM 3-Phase Inverter PWM driver (TC4xx)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize a 3-channel complementary, center-aligned PWM using eGTM ATOM0 Cluster 0.
 * - Synchronous start and update enabled
 * - 20 kHz switching frequency
 * - 1 us rising/falling dead-time
 * - Period-event interrupt on channel 0 (provider cpu0, priority 20) toggles LED P03.9
 */
void initEgtmAtom3phInv(void);

/**
 * Update the three logical PWM channels' duty cycles by a fixed step with wrap-around at 100%.
 * Applies changes immediately and synchronously to all channels.
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
