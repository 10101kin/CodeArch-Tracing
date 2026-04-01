/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-Phase Inverter PWM driver (TC4xx).
 *
 * Note:
 *  - Cpu0_Main.c is responsible for watchdog handling and scheduling.
 *  - This header exposes only initialization and runtime update entry points.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize EGTM ATOM0 Cluster 0 complementary 3-phase PWM (20 kHz, center-aligned). */
void initEgtmAtom3phInv(void);

/** Update all three PWM channels' duty by a fixed step with wrap at 100%. */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
