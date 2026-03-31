/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Public API for GTM TOM 3-Phase Inverter PWM driver (IfxGtm_Pwm unified driver)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize 3-phase center-aligned PWM on TOM1 using IfxGtm_Pwm unified driver.
 * - U/V/W = 25%/50%/75% initial duty
 * - Frequency = 20 kHz
 * - Single-ended outputs (HS pins only)
 * - Sync start and sync update enabled
 */
void initGtmTom3phInv(void);

/**
 * Update three phase duty cycles (percent) with +10% step and wrap at [10%,90%]:
 * - duty = duty + 10
 * - if duty >= 90 then duty = 10
 * Applies the update synchronously to all configured channels.
 */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
