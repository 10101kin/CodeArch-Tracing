/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Public API for 3-phase complementary PWM generation on GTM TOM
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

void GTM_TOM_3_Phase_Inverter_PWM_init(void);
void GTM_TOM_3_Phase_Inverter_PWM_updateDuties(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
