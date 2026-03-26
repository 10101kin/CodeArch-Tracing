#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

/* Public API (from SW Detailed Design) */
void GTM_TOM_3PhaseInverterPWM_init(void);
void GTM_TOM_3PhaseInverterPWM_updateDuties(void);

/* Function prototypes (auto-generated) */
void interruptGtmTom(void);
void IfxGtm_periodEventFunction(void *data);


#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
