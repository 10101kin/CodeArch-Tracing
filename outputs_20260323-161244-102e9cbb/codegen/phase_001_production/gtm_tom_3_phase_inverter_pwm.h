/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Production module: GTM_TOM_3_Phase_Inverter_PWM
 * Target: AURIX TC3xx (GTM TOM)
 *
 * This header declares the public API for initializing and updating a
 * 3-phase inverter PWM using six single-output TOM channels sharing a common time base.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize GTM TOM PWM for a 3-phase inverter using six single-output channels.
 * - Enables GTM and functional clocks
 * - Configures a shared TOM time base at 20 kHz center-aligned
 * - Routes six output pins via P02.x as specified by the design
 * - Sets complementary behavior by polarity with zero dead-time
 * - Stages and applies the initial duties atomically
 */
void initGtmTomPwm(void);

/**
 * Update PWM duties (runtime behavior):
 * - Increase each high-side duty by +10 percentage points, wrapping to 0.0 when exceeding 100.0
 * - Low-side numeric duties mirror their corresponding high-sides (effective complement is via polarity)
 * - Apply all changes atomically using shadow update
 *
 * Note: Call this function at the desired rate (e.g., every 500 ms) from the application loop.
 */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
