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
 * Initialize EGTM ATOM0 Cluster0 3-phase complementary, center-aligned PWM
 * - 3 channels (U,V,W)
 * - 20 kHz switching frequency, syncStart + syncUpdate enabled
 * - 1 us dead-time (rising/falling)
 * - Initial duties: U=25%, V=50%, W=75%
 * - Period-event interrupt on channel 0 with priority 20 (CPU0)
 */
void initEgtmAtom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
