/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM driver (IfxGtm_Pwm-based)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize 3-phase center-aligned PWM (U, V, W) on TOM using IfxGtm_Pwm */
void initGtmTom3phInv(void);

/* Update U/V/W duty cycles with +10% step and wrap in [10%, 90%], then apply immediately */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
