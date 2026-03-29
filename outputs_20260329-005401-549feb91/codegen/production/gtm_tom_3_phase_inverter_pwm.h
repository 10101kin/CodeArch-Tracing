/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief Public API for GTM TOM 3-Phase Inverter PWM driver (unified IfxGtm_Pwm)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the GTM unified PWM for three complementary pairs on TOM1 */
void initGtmTomPwm(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
