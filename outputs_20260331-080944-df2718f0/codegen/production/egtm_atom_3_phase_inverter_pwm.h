/**
 * @file egtm_atom_3_phase_inverter_pwm.h
 * @brief EGTM ATOM 3-Phase Inverter PWM driver (TC4xx)
 *
 * Public API for initializing the EGTM-based 3‑phase complementary, center‑aligned PWM.
 *
 * Note:
 * - Watchdog disable/configuration must be done only in CpuX_Main.c (IfxWtu_* APIs).
 * - Timing/scheduling (e.g., periodic duty updates) must be handled in application code.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize EGTM ATOM0 Cluster 0 for 3‑phase complementary, center‑aligned PWM at 20 kHz.
 */
void initEgtmAtom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
