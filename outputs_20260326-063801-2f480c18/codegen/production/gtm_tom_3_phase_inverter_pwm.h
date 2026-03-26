/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm (single-ended)
 *
 * This header intentionally contains only include guards and function prototypes,
 * as required by the production code rules.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize GTM, TOM timebase, and unified PWM for 3 single-ended channels */
void initGtmTomPwm(void);

/* Update duties (+10% step with 0..100% wrap) and apply synchronous shadow transfer */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
