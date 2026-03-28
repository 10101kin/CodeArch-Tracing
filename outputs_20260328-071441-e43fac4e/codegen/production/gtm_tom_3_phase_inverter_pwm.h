/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief Public API for GTM TOM 3-phase complementary inverter PWM using unified IfxGtm_Pwm
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the GTM PWM for a 3-phase complementary inverter using unified IfxGtm_Pwm
 */
void initGtmTomPwm(void);

/**
 * @brief Atomically update all three complementary PWM pairs under shadow gating
 */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
