/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for EGTM ATOM 3-phase complementary PWM + ADC trigger (TC4xx).
 *
 * Note:
 *  - Watchdog disable must remain in CpuX_Main.c only (per AURIX standard).
 *  - This header intentionally contains only the public function prototypes.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize EGTM ATOM for 3-phase complementary PWM and a dedicated single-channel PWM for ADC trigger.
 * - Configures ATOM0 CH0-2 on P20.8..P20.13 (complementary pairs) for center-aligned PWM at 20 kHz, 1 us dead-time.
 * - Configures ATOM0 CH3 on P33.0 for edge-aligned 50% duty to be routed as ADC trigger.
 * - Enables EGTM clocks if not already enabled.
 * - Sets up unified PWM driver and period-event ISR callback routing.
 */
void initEgtmAtom3phInv(void);

/**
 * Update the three inverter phase duties immediately and synchronously.
 * requestDuty[0..2] are in percent [0..100]. Values are clamped to [0, 100].
 * Applies the update via IfxEgtm_Pwm_updateChannelsDutyImmediate().
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
