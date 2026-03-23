/*
 * gtm_tom_3_phase_inverter_pwm.h
 * TC3xx (TC387) - 3-Phase Inverter PWM using GTM TOM unified high-level driver (IfxGtm_Pwm)
 *
 * This module initializes six single-output PWM channels on a shared TOM time base at 20 kHz,
 * center-aligned, with synchronized shadow updates. High-side duties start at 25/50/75 percent.
 * Low-sides use inverted polarity with the same numeric duty values to yield complementary
 * behavior without hardware dead-time.
 *
 * NOTES:
 * - Watchdog control is NOT included here; it must be handled only in CpuN_Main.c files.
 * - Generic PinMap header is used; avoid family-specific pin map headers.
 */
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
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
