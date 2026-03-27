/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for 3-phase complementary PWM on GTM TOM using IfxGtm_Pwm
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public functions */
void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
