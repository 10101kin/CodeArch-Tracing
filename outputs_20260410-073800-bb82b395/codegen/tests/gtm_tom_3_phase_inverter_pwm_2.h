/*
 * gtm_tom_3_phase_inverter_pwm_2.h
 *
 * Public API for 3-phase complementary PWM using GTM TOM (IfxGtm_Pwm unified driver)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_2_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_2_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public functions */
void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);
void interruptGtmAtom(void);
void IfxGtm_periodEventFunction(void *data);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_2_H */
