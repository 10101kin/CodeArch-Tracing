/*
 * File: gtm_tom_3_phase_inverter_pwm.h
 * Description: GTM TOM 3-Phase Inverter PWM - Public API
 * Target: TC3xx (TC387)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize GTM TOM 3-phase inverter PWM.
 *        Follows SW Detailed Design: enables GTM and clocks, configures unified PWM
 *        for center-aligned, complementary outputs, maps pins, programs frequency,
 *        sets dead-time, applies initial duties, and starts synchronized outputs.
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);

/**
 * \brief Runtime duty update for U/V/W phases.
 *        On each call, increments duty by a fixed step and wraps between min/max
 *        bounds, then performs a single synchronous update across all channels.
 */
void GTM_TOM_3_Phase_Inverter_PWM_updateUVW(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
