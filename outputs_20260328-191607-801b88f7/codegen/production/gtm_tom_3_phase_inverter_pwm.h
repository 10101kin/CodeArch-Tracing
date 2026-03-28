/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for 3-phase complementary PWM on GTM TOM1 Cluster_1 using IfxGtm_Pwm.
 *
 * Note:
 *  - Do NOT add includes here. Keep this header minimal per production rules.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H_
#define GTM_TOM_3_PHASE_INVERTER_PWM_H_

#ifdef __cplusplus
extern "C" {
#endif

void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H_ */
