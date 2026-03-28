/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for 3-phase complementary center-aligned PWM using IfxGtm_Pwm on TOM backend.
 *
 * Note: iLLD headers must only be included in the .c file.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize 3-phase complementary center-aligned PWM on TOM1, Cluster 1 at 20 kHz with 1 us deadtime.
 */
void initGtmTom3phInv(void);

/**
 * Update duty cycles by +10% with wrap-to-0-then-add-step behavior and apply immediately.
 */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
