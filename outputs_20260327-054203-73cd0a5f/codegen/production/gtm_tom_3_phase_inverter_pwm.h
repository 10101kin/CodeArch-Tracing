/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM driver (TC3xx).
 *
 * Note:
 *  - No includes, typedefs, or macros here per project rules.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

void initGtmTomPwm(void);
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
