/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief GTM TOM 3-Phase inverter PWM driver public API
 *
 * This header exposes the public initialization and runtime update APIs for the
 * 3-phase complementary PWM using the unified IfxGtm_Pwm driver on TOM1 Cluster_1.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize 3-phase complementary PWM on TOM1 Cluster_1 (center-aligned, 20 kHz, 1 us dead-time).
 */
void initGtmTom3phInv(void);

/**
 * @brief Step duty cycles (U,V,W) by +10% with wrap rule and update coherently.
 */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
