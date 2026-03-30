/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief Public API for GTM TOM 3-phase inverter PWM (IfxGtm_Pwm unified driver)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a 3-phase, center-aligned PWM on TOM1 (cluster 1) with complementary outputs.
 */
void initGtmTom3phInv(void);

/**
 * @brief Update U/V/W duties with step-and-wrap, then apply immediately.
 */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
