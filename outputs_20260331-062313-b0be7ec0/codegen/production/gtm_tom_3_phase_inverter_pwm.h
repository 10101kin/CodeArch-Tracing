/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Public API for GTM TOM 3-phase inverter PWM driver
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize 3-phase center-aligned PWM (TOM) with U/V/W channels */
void initGtmTom3phInv(void);

/** Update U/V/W duty cycles with +10% step and wrap [10..90]% and apply synchronously */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
