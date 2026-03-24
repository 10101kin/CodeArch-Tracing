#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize GTM TOM 3-phase inverter PWM.
 * - Enables GTM and clocks
 * - Configures TOM1 time base for 20 kHz center-aligned PWM
 * - Configures complementary PWM (3 pairs) with deadtime and min pulse
 * - Routes pins to P00.[3/2], P00.[5/4], P00.[7/6]
 * - Starts the timer and programs initial on-times (U=25%, V=50%, W=75%)
 * - Uses shadow update to latch at next period boundary
 */
void initGtmTom3phInv(void);

/**
 * Runtime duty update function.
 * - Reads current period
 * - Increments each phase on-time by +10% of the period
 * - Wraps to 0 when reaching/exceeding 100%
 * - Applies updates synchronously using shadow update
 * Caller controls call rate (e.g., every 500 ms).
 */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
