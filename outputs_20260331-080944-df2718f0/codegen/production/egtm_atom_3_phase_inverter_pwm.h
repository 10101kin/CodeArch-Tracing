/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-Phase Inverter PWM driver (TC4xx)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize EGTM ATOM0 Cluster 0 for 3-phase complementary, center-aligned PWM at 20 kHz
 * with 1 us dead-time and initial duties U/V/W = 25%/50%/75%.
 * - SyncStart and SyncUpdate enabled
 * - Fxclk_0 used as ATOM clock
 * - Period-event interrupt routed to CPU0, priority 20, ISR toggles LED
 */
void initEgtmAtom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
