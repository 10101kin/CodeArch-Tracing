/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM1 3-phase complementary PWM driver (KIT_A2G_TC387_5V_TFT)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize GTM for 3-phase complementary PWM on TOM1 using a TOM timer base
 * and PwmHl for paired outputs.
 */
void initGtmTom3phInv(void);

/**
 * Update three phase on-times in a cyclic ramp and apply them synchronously at
 * the PWM period boundary.
 */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
