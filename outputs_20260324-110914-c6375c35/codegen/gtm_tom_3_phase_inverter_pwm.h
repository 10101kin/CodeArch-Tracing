/**********************************************************************************************************************
 * 
 *  File: gtm_tom_3_phase_inverter_pwm.h
 *  Brief: GTM TOM 3-Phase Inverter PWM - Unified IfxGtm_Pwm driver interface
 *  Target: TC3xx (TC387)
 *
 *********************************************************************************************************************/
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public API as required by SW Detailed Design */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);
void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
