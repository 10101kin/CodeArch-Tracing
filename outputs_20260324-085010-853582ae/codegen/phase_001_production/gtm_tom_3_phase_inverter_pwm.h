#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the GTM TOM-based 3-phase complementary, center-aligned PWM using
 * IfxGtm_Tom_Timer and IfxGtm_Tom_PwmHl drivers. See implementation for full
 * iLLD-compliant sequence and error handling.
 */
void initGtmTomPwm(void);

/**
 * Periodic runtime duty-cycle ramp update with wrap-around. Coherently updates
 * all three complementary PWM pairs by writing on-times atomically.
 * Note: Name preserved per SW Detailed Design (does not override iLLD symbol).
 */
void IfxGtm_Tom_PwmHl_setOnTime(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
