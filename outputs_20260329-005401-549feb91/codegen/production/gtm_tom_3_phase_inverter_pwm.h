/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM three-phase inverter PWM (unified IfxGtm_Pwm driver)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

/*
 * Initialize the GTM unified PWM for three complementary pairs on TOM1 with
 * a 20 kHz center-aligned timebase, 0.5 us dead-time, and 1.0 us min pulse.
 */
void initGtmTomPwm(void);

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
