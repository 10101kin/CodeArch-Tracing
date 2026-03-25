/***************************************************************************
 * File: gtm_tom_3_phase_inverter_pwm.h
 * Description: GTM TOM 3-Phase Inverter PWM - Unified IfxGtm_Pwm driver API
 * Target: AURIX TC3xx
 * Notes:
 *  - Follows unified IfxGtm_Pwm configuration patterns with channel/output arrays
 *  - Center-aligned 20 kHz PWM on TOM1 using P00.2–P00.7 pinout
 *  - Initial U/V/W duty 25/50/75 percent; runtime +10% every 500 ms with wrap
 *  - Complementary outputs currently not used; configured as safe GPIO low
 *  - Watchdog handling is NOT present here (must be in CpuN_Main.c)
 ***************************************************************************/
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public API (from SW Detailed Design) */
void initGtmTomPwm(void);
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptGtmTom(void);
void IfxGtm_periodEventFunction(void *data);

}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
