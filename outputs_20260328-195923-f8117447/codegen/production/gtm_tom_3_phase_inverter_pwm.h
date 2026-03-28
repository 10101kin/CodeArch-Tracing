/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for 3-phase complementary, center-aligned PWM on GTM TOM (TC3xx).
 *
 * NOTE: Do not include any iLLD headers here. Implementations and includes are in the .c file.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the 3-phase complementary PWM (TOM backend) */
void initGtmTom3phInv(void);

/* Update the three phase duties in percent (+10% step, wrap-to-0-then-add-step) */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
