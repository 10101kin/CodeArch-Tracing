/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief GTM TOM three-phase inverter PWM driver (TC3xx) — public API
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize GTM TOM1 three-phase complementary PWM (20 kHz, center-aligned). */
void initGtmTomPwm(void);

/** Periodically update the three-phase on-times with a small ramp step and apply synchronously. */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
