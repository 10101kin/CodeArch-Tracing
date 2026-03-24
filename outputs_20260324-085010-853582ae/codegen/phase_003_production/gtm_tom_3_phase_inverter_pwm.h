/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief GTM TOM 3-Phase Inverter PWM driver public API (TC3xx / TC387)
 *
 * This module configures a 3-phase complementary center-aligned PWM using GTM TOM
 * and the iLLD IfxGtm_Tom_PwmHl driver, following the SW Detailed Design exactly.
 *
 * Notes:
 * - Watchdog disable is not handled here (must be in CpuN_Main.c)
 * - Pin mapping uses generic IfxGtm_PinMap.h symbols
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the GTM TOM-based 3-phase complementary center-aligned PWM.
 *
 * Behavior (from SW Detailed Design):
 * 1) Enable GTM and required clocks
 * 2) Configure TOM timer (TOM instance, base channel, frequency, cmuFxclk0)
 * 3) Configure six PWM pins via GTM PinMap with push-pull + cmosAutomotiveSpeed1
 * 4) Configure and initialize IfxGtm_Tom_PwmHl for 3 complementary pairs, set dead-time and min pulse
 * 5) Select center-aligned mode
 * 6) Compute initial on-times from period (25%/50%/75%), atomically apply via disable/update/apply
 */
void initGtmTomPwm(void);

/**
 * @brief Runtime duty-cycle ramp with wrap-around and coherent update.
 *
 * Behavior (from SW Detailed Design):
 * 1) Read current period
 * 2) Compute duty increment step and min/max boundaries from period
 * 3) For each channel: add step; if >= max, wrap to min
 * 4) Disable update, write on-time array, apply update atomically
 */
void IfxGtm_Tom_PwmHl_setOnTime(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
