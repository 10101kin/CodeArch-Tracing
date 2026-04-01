/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Public API for GTM TOM 3-Phase Inverter PWM (IfxGtm_Pwm unified driver)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the GTM-based 3-phase complementary PWM using IfxGtm_Pwm on TOM1 Cluster_1.
 * - Center-aligned at 20 kHz, sync start and sync updates
 * - Dead-time configured in software via channel DTM config
 * - Shared TOM timer is initialized for update gating
 * - LED pin (P13.0) configured as push-pull output for ISR toggle indication
 */
void initGtmTom3phInv(void);

/**
 * Update three phase duty cycles (percent) with wrap rule and apply atomically via TOM update gating.
 * - Wrap rule per phase: if ((duty + step) >= 100) duty = 0; then duty += step
 * - Issues a synchronized update pair: disableUpdate → updateChannelsDutyImmediate → applyUpdate
 */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
