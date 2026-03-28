/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for 3-phase complementary PWM on GTM TOM (TC3xx).
 *
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize 3-phase complementary PWM using IfxGtm_Pwm high-level driver */
void initGtmTomPwm(void);

/* Update duty cycles for phases U, V, W (percent-based) and apply immediately */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
