/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Public API for GTM TOM three-phase inverter PWM driver (TC3xx)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

/*
 * Initialize GTM TOM1 three-phase complementary PWM (20 kHz, center-aligned).
 */
void initGtmTomPwm(void);

/*
 * Runtime update of three-phase PWM on-times using a fixed-step ramp with
 * synchronous shadow transfer at period boundary.
 */
void updateGtmTomPwmDutyCycles(void);

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
