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
 * Initialize EGTM ATOM-based 3-phase complementary center-aligned PWM (CH0..CH2)
 * and route ATOM0 CH3 as ADC trigger source. Performs module clock enable/CMU setup,
 * configures PWM with dead-time and interrupt, and prepares internal state.
 */
void initEgtmAtom3phInv(void);

/**
 * Update logical duty cycles (percent 0..100) for the three PWM channels atomically.
 * Values are clamped to [0, 100] and take effect at the next period boundary via shadow update.
 *
 * @param requestDuty Pointer to array of three float32 duty-percent values: {U, V, W}.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
