#ifndef IFXPORT_PINMAP_H
#define IFXPORT_PINMAP_H

#include "IfxPort.h"
#include "IfxGtm_Pwm.h"

/* Extern pin symbol stubs required by production code/tests */
extern IfxGtm_Pwm_ToutMap IfxGtm_TOM1_1N_TOUT14_P00_5_OUT;
extern IfxGtm_Pwm_ToutMap IfxGtm_TOM1_2N_TOUT15_P00_6_OUT;
extern IfxGtm_Pwm_ToutMap IfxGtm_TOM1_2_TOUT12_P00_3_OUT;
extern IfxGtm_Pwm_ToutMap IfxGtm_TOM1_3N_TOUT16_P00_7_OUT;
extern IfxGtm_Pwm_ToutMap IfxGtm_TOM1_3_TOUT13_P00_4_OUT;
extern IfxGtm_Pwm_ToutMap IfxGtm_TOM1_5_TOUT11_P00_2_OUT;
/* Additional pins to resolve prior build errors */
extern IfxGtm_Pwm_ToutMap IfxGtm_TOM1_4_TOUT14_P00_5_OUT;
extern IfxGtm_Pwm_ToutMap IfxGtm_TOM1_6_TOUT16_P00_7_OUT;

#endif /* IFXPORT_PINMAP_H */
