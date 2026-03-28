/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for 3-phase complementary PWM on GTM TOM1 Cluster_1 using IfxGtm_Pwm.
 *
 * Notes:
 * - No watchdog handling here (handled only in CpuX main files).
 * - No STM timing logic here; caller is responsible for periodic scheduling.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initializes 3-phase complementary PWM (TOM1 Cluster_1) at 20 kHz, center-aligned, 1 us dead time */
void initGtmTom3phInv(void);

/* Updates phase duty cycles (percent 0..100) with STEP=10 and wrap rule, applies immediate synced update */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
