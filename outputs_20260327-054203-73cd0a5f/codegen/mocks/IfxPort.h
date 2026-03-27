#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Minimal required enums */
typedef enum {
    IfxPort_OutputIdx_general = 0,
    IfxPort_OutputIdx_alt1,
    IfxPort_OutputIdx_alt2,
    IfxPort_OutputIdx_alt3
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_notActive = 0,
    IfxPort_State_active    = 1
} IfxPort_State;

#endif /* IFXPORT_H */
