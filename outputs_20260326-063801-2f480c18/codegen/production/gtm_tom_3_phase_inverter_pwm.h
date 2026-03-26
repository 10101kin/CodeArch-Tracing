/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm driver
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

/* Public API */
void initGtmTomPwm(void);
void updateGtmTomPwmDutyCycles(void);

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
