/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM (single-ended) using IfxGtm_Tom_Timer timebase
 *
 * NOTE: Do not place watchdog disable code here. Follow AURIX standard: watchdog handling only in CpuX_Main.c
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public API */
void initGtmTomPwm(void);
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
