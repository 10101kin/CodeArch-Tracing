/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Public API for GTM TOM 3-Phase Inverter PWM driver (IfxGtm_Pwm unified driver)
 *
 * Notes:
 * - This header intentionally contains only public prototypes and an include guard.
 * - All iLLD includes and private definitions are in the .c file per project rules.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize GTM TOM 3-phase inverter PWM */
void initGtmTom3phInv(void);

/* Periodic duty update (step +10% with wrap rule, applied immediately) */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
