/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM driver (IfxGtm_Pwm unified driver)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

/* Public API */
/**
 * Initialize 3-phase complementary, center-aligned PWM on GTM TOM1, Cluster_1
 * with synchronous start/update at 20 kHz and 1 us dead time. Also prepares the
 * LED GPIO (P13.0) as push-pull output default LOW for ISR toggling.
 */
void initGtmTom3phInv(void);

/**
 * Update the duty cycles for phases U, V, W in percent using the wrap rule
 * and apply them immediately to the configured PWM channels.
 */
void updateGtmTom3phInvDuty(void);

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
