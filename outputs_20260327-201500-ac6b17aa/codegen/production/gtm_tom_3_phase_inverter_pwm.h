/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for 3-phase inverter PWM (GTM TOM) using IfxGtm_Pwm unified driver.
 *
 * This header intentionally contains only prototypes (no includes/macros) per template rules.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
