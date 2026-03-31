/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief GTM TOM 3-phase complementary PWM driver (unified IfxGtm_Pwm API)
 *
 * Public API: initialization and duty update.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a persistent 3-channel complementary PWM on TOM using IfxGtm_Pwm.
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);

/**
 * @brief Update the three phase duties in percent and apply them immediately.
 */
void GTM_TOM_3_Phase_Inverter_PWM_updateDuties(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
