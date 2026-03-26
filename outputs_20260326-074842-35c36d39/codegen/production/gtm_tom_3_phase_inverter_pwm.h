/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief GTM TOM 3-Phase Inverter PWM driver (TC3xx, unified IfxGtm_Pwm)
 *
 * Production-ready module implementing a two-level 3-phase inverter PWM using
 * GTM TOM submodule via the unified IfxGtm_Pwm driver.
 *
 * Public API (from SW Detailed Design):
 *  - void GTM_TOM_3PhaseInverterPWM_init(void)
 *  - void GTM_TOM_3PhaseInverterPWM_updateDuties(void)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the GTM TOM unified PWM for a 3‑phase inverter (6 complementary outputs).
 *
 * Behavior (per SW Detailed Design):
 *  - Enable GTM and configure CMU fixed clock derived from GCLK as PWM time base
 *  - Initialize unified IfxGtm_Pwm with center-aligned mode at requested frequency
 *  - Use TOM1 CH0 as master time base; configure 3 complementary output pairs
 *  - Active-high outputs, specified dead-time and min pulse constraints
 *  - Assign pre‑verified output pins, push‑pull mode, requested pad driver
 *  - Initialize the PWM handle with internal channels array
 *  - Stage initial duties (25%, 50%, 75%) with synchronized update semantics
 *  - Enable channel outputs
 */
void GTM_TOM_3PhaseInverterPWM_init(void);

/**
 * @brief Runtime duty update following ramp-and-wrap behavior for all three phases.
 *
 * Behavior (per SW Detailed Design):
 *  - Compute duty step as a fraction of PWM (expressed here as percent step)
 *  - Add to each phase's duty; if duty reaches/exceeds upper threshold, wrap to lower threshold
 *  - Apply the three updated phase duties synchronously to all six complementary outputs
 */
void GTM_TOM_3PhaseInverterPWM_updateDuties(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
