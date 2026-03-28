/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief Public API for 3-phase complementary center-aligned PWM on GTM TOM using IfxGtm_Pwm
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

/**
 * @brief Initialize 3-phase complementary center-aligned PWM (TOM1, Cluster_1) at 20 kHz with 1 us deadtime.
 */
void initGtmTom3phInv(void);

/**
 * @brief Update the three phase duties: +10% step, wrap-to-0-then-add-step, apply immediately.
 */
void updateGtmTom3phInvDuty(void);

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
