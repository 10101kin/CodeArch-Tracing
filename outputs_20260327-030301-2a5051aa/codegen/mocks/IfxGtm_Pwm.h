/* Minimal IfxGtm_Pwm.h to provide IfxGtm_Pwm_ToutMap typedef (fixes unknown type errors) */
#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

typedef struct { uint32 reserved; } IfxGtm_Pwm_ToutMap;

typedef const IfxGtm_Pwm_ToutMap* IfxGtm_Pwm_ToutMapP;

#endif /* IFXGTM_PWM_H */
