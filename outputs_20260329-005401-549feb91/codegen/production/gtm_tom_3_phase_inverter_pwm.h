/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM driver based on unified IfxGtm_Pwm.
 *
 * Note: This header intentionally contains only function prototypes, as required.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the GTM unified PWM for 3 complementary TOM pairs (TOM1 base) */
void initGtmTomPwm(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
