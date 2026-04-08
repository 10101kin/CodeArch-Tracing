/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for 3-phase complementary inverter PWM using IfxGtm_Pwm on TOM1.
 *
 * Note: Cpu0_Main.c must handle watchdogs and scheduling. This module provides
 *       initialization and atomic duty update only.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public API */
void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);
void interruptGtmAtom(void);
void IfxGtm_periodEventFunction(void *data);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
