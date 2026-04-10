/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Public API for GTM TOM 3-Phase Inverter PWM driver
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);
void interruptGtmAtom(void);
void IfxGtm_periodEventFunction(void *data);
void values(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
