/*
 * gtm_tom_3_phase_inverter_pwmgg.h
 *
 * Production PWM driver for TC387 (TC3xx) using IfxGtm_Pwm unified driver.
 *
 * Implements the API contract from SW Detailed Design:
 *   void updateGtmTomPwmDutyCycles(void)
 *
 * Behavior:
 *   - On first call, fully initializes GTM clocks and a 3-channel TOM1-based
 *     center-aligned PWM at 20 kHz, mapped to single-ended outputs.
 *   - Applies initial duties 25%, 50%, 75% and starts synchronized channels.
 *   - On subsequent calls, increments each duty by +10 percentage points and wraps at 100% -> 0%.
 *   - Updates duties synchronously via IfxGtm_Pwm_updateChannelsDuty().
 *
 * Notes:
 *   - Watchdog control must NOT be placed in this module (CpuN_Main.c only).
 *   - Uses only generic iLLD headers (no family-suffixed pinmaps).
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWMGG_H
#define GTM_TOM_3_PHASE_INVERTER_PWMGG_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public API (exact signature from SW Detailed Design) */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWMGG_H */
