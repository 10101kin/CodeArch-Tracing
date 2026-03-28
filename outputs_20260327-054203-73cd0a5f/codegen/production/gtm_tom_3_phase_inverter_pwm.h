/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM driver
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the GTM TOM-based 3-phase complementary PWM (center-aligned)
 * - Enables GTM and FXCLK domain
 * - Configures TOM timer base (TOM1, CH0) at 20 kHz using FXCLK0
 * - Initializes PwmHl for 3 complementary phases with configured pins
 * - Sets dead-time and minimum pulse, push-pull, CMOS pad driver, active-high polarity
 * - Starts the timer and applies initial duties (U=25%, V=50%, W=75%) synchronously
 */
void initGtmTomPwm(void);

/**
 * Update duty cycles of the 3 phases with a fixed increment step.
 * - Reads current period from persistent timer handle
 * - Increments each phase on-time by a fixed fraction of period
 * - Wraps to minimum pulse threshold when exceeding maximum allowable on-time
 * - Applies updates synchronously via shadow transfer
 */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
