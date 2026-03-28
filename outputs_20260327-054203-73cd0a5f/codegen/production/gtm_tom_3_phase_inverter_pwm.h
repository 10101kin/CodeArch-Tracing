/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Public API for GTM TOM three-phase inverter PWM driver.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize GTM TOM PWM for a 3-phase inverter (center-aligned, complementary outputs). */
void initGtmTomPwm(void);

/** Periodically update the three-phase duty on-times with a fixed step and wrap. */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
