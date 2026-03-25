/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Production PWM driver for TC3xx (TC387) using unified IfxGtm_Pwm (TOM) driver.
 *
 * Do not place watchdog disable code here. CpuN_Main.c handles watchdog policy.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public API (preserve reference names/signatures) */
void initGtmTomPwm(void);
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
