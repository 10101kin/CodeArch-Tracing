/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM-based 3-phase inverter PWM using IfxGtm_Pwm.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initializes GTM TOM1 for 3-phase complementary PWM (center-aligned, 20 kHz) */
void initGtmTom3phInv(void);

/* Updates duty cycles using percentage logic and applies immediately */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
