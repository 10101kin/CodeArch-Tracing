#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Public API (from SW Detailed Design)
 * EXACT function names and signatures preserved.
 */
void IfxGtm_Tom_PwmHl_init(void);
void IfxGtm_Tom_PwmHl_setDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
