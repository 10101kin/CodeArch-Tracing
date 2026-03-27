/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief GTM TOM three-phase complementary PWM driver public API
 *
 * This header exposes the initialization and runtime update APIs for
 * a 3-phase inverter PWM built on GTM TOM1 using a TOM timer as base
 * and the TOM PwmHl helper for complementary outputs with dead-time
 * and minimum pulse enforcement.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize GTM TOM1 for 3-phase complementary PWM (U,V,W pairs) using PwmHl.
 */
void initGtmTom3phInv(void);

/**
 * @brief Update phase on-times in a cyclic ramp and apply synchronously at period boundary.
 */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
