/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Public API for GTM TOM 3-phase complementary PWM using IfxGtm_Pwm
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the GTM-based 3-phase complementary PWM using the unified IfxGtm_Pwm driver
 * on TOM1 Cluster_1 with center-aligned mode and synchronized start/updates.
 * This configures six outputs (U/V/W high+low), a shared TOM timer for update gating,
 * and a minimal ISR to toggle a debug LED.
 */
void initGtmTom3phInv(void);

/**
 * Update three phase duty cycles with wrap and clamp, then apply them atomically
 * using TOM timer update gating (disable/apply update sequence).
 */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
