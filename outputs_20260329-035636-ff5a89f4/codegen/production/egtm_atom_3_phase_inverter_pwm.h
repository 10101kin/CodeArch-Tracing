/**
 * @file egtm_atom_3_phase_inverter_pwm.h
 * @brief eGTM ATOM three-phase inverter PWM driver (TC4xx)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize eGTM Cluster1 ATOM1 three-phase complementary, center-aligned PWM at 20 kHz with 1.0 us dead-time.
 *        Synchronous start and update are enabled; a period ISR is configured on CPU0 with priority 20.
 */
void initEgtmAtom3phInv(void);

/**
 * @brief Increment U/V/W duty cycles by a fixed percentage step and apply immediately to all channels.
 *        Wraps each duty to 0 before increment when (duty + STEP) >= 100, then adds STEP.
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
