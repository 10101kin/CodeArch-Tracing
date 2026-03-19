#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

/**
 * GTM TOM 3-Phase Inverter PWM - Production Interface
 *
 * This module provides initialization and runtime duty-cycle update for a 3-phase
 * complementary PWM using GTM TOM on TC3xx, configured for center-aligned operation
 * with hardware dead-time and minimum pulse enforcement.
 *
 * API contract (exact function names/signatures preserved from SW Detailed Design):
 *   - void IfxGtm_Tom_PwmHl_init(void);
 *   - void IfxGtm_Tom_PwmHl_setDuty(void);
 */

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize GTM TOM PWM for 3-phase complementary, center-aligned operation.
 *
 * Behavior (from SW Detailed Design):
 * 1) IfxGtm_enable; 2) IfxGtm_Cmu_enableClocks; 3) Prepare timer and PwmHl config
 *    for 20 kHz with fxclk0; 4) Initialize Timer and PwmHl; 5) Configure dead-time
 *    and min pulse; 6) Program initial duties 25%/50%/75% with synchronous update;
 * 7) Start PWM so updates apply together at the next timer boundary.
 */
void IfxGtm_Tom_PwmHl_init(void);

/**
 * Runtime duty update with wrap and safety clamping.
 *
 * Behavior (from SW Detailed Design):
 * - Maintain persistent U/V/W duties in normalized percent (0.0–1.0).
 * - Each call: add +0.10 to each; if >= 1.0 wrap to 0.0.
 * - Compute period from 20 kHz; compute safeMin=(minPulse+2*deadtime)/period,
 *   safeMax=(1.0-safeMin) and clamp each duty into [safeMin, safeMax].
 * - Apply all three using a non-immediate, synchronous update at the next timer event.
 */
void IfxGtm_Tom_PwmHl_setDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
