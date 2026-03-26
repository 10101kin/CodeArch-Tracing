/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM-based 3-phase complementary PWM using IfxGtm_Pwm.
 *
 * NOTE: Do not include any other headers here. This header only exposes
 *       the public driver function prototypes.
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
