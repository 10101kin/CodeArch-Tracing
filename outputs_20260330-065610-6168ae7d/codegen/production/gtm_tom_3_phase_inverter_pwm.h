/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Public API for GTM TOM 3-Phase Inverter PWM driver (IfxGtm_Pwm unified).
 *
 * Do not add includes, typedefs, or macros here. Only public prototypes.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize GTM TOM1 PWM for 3-phase inverter */
void initGtmTom3phInv(void);

/* Update PWM duty cycles (percentage-based, synchronous update) */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
