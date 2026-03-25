/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Production driver for TC3xx (TC387) 3-phase inverter PWM using IfxGtm_Pwm (TOM)
 * with IfxGtm_Tom_Timer as shared timebase and TGC atomic update gating.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public API (SW Detailed Design contract) */
void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
