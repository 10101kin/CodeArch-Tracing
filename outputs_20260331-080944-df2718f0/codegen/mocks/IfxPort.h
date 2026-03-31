#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Enums needed by production */
typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_low  = 0,
    IfxPort_State_high = 1
} IfxPort_State;

/* Pin descriptor */
typedef struct {
    Ifx_P *port;
    uint8  pinIndex;
} IfxPort_Pin;

/* Minimal API used */
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);

#endif /* IFXPORT_H */
