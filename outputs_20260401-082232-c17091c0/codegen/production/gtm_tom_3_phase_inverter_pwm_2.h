/*
 * gtm_tom_3_phase_inverter_pwm_2.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM (migration variant).
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_2_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_2_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize GTM TOM-based 3-phase complementary PWM (center-aligned, sync start/update).
 * - Submodule: TOM1, Cluster_1
 * - Frequency: 20 kHz
 * - Dead-time: configured per design
 * - Period interrupt: priority 20, CPU0 (LED toggle done in ISR)
 */
void initGtmTom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_2_H */
