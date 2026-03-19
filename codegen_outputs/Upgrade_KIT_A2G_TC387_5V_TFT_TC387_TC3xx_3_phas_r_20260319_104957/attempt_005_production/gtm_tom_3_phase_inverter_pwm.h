#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the GTM-based three-phase PWM inverter using TOM1 channels.
 * Implements the SW Detailed Design behavior: GTM enable, TOM timer @20kHz
 * center-aligned on Fxclk0, six PWM outputs on TOM1 CH1..CH6 with existing
 * board pin mapping, and software dead-time applied to low-side duties.
 */
void initGtmTomPwm(void);

/**
 * Periodic duty update per SW Detailed Design: every call increments U/V/W
 * high-side duty by +10% with wrap in 0..100%; computes software dead-time
 * (0.5us) fraction using Fxclk0 and period ticks, then sets low-side duty to
 * (100 - HS - deadTimePct*100) clamped to [0,100]. Applies all six duties in
 * a single atomic update.
 */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
