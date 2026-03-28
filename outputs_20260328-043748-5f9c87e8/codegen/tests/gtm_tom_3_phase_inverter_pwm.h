/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief Public API for GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm driver.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize 3-phase center-aligned complementary PWM on TOM1 using IfxGtm_Pwm */
void initGtmTomPwm(void);

/** Update U/V/W duties with fixed 10% step and wrap rule; apply immediately */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
