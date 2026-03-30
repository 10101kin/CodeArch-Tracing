/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM driver (IfxGtm_Pwm unified driver).
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize GTM/TOM1 3-phase complementary PWM (20 kHz, center-aligned, 1 us dead-time). */
void initGtmTom3phInv(void);

/** Update 3-phase PWM duty cycles (percentage-based, step=10, wrap rule). */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
