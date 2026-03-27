/**
 * @file gtm_tom_3_phase_inverter_pwm.h
 * @brief GTM TOM three-phase inverter PWM driver (TC3xx)
 *
 * This header exposes the public initialization and runtime update APIs for the
 * three-phase complementary PWM using GTM TOM1 with center-aligned operation.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

/**
 * @brief Initialize GTM TOM1 timer and complementary PWM for a 3-phase bridge.
 *
 * Behavior summary:
 * - Enable GTM and FXCLK domain clocks
 * - Configure TOM1 timer base on channel 0, FXCLK0, 20 kHz
 * - Configure complementary high/low outputs for phases U,V,W
 * - Center-aligned PWM mode, dead-time and min-pulse enforced
 * - Apply initial duties: U=25%, V=50%, W=75% using a shadow-update
 */
void initGtmTomPwm(void);

/**
 * @brief Update the on-time ticks for all three phases with a fixed increment,
 *        wrapping within [minPulse..(period-minPulse)] and apply via shadow-update.
 */
void updateGtmTomPwmDutyCycles(void);

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
