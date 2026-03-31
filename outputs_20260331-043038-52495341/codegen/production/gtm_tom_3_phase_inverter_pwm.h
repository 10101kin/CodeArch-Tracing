/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief Public API for the 3-phase complementary PWM on GTM TOM
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize persistent 3-channel complementary PWM on TOM using unified IfxGtm_Pwm API.
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);

/**
 * @brief Update the three phase duties and apply immediately.
 */
void GTM_TOM_3_Phase_Inverter_PWM_updateDuties(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
