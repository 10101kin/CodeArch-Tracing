/*==========================================================================
 *  File: gtm_tom_3_phase_inverter_pwm.h
 *  Brief: Public API for GTM TOM 3-Phase Inverter PWM driver (IfxGtm_Pwm)
 *==========================================================================*/
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize 3-phase center-aligned PWM on TOM1 (U,V,W = 25/50/75%). */
void initGtmTom3phInv(void);

/** Update U,V,W duty cycles (+10% step with wrap to [10%,90%) and apply immediately). */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
