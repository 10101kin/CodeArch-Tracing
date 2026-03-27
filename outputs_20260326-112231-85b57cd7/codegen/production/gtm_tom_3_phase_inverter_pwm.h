/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for 3-phase complementary PWM on GTM TOM using unified IfxGtm_Pwm driver.
 *
 * Note: This header intentionally contains only the public function prototypes
 * as per module architecture rules.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public APIs */
void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
