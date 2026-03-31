/*
 * IfxPort.h - Port mock header
 */
#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Common enums used by production */
typedef enum
{
    IfxPort_OutputIdx_general = 0,
    IfxPort_OutputIdx_alt1    = 1,
    IfxPort_OutputIdx_alt2    = 2,
    IfxPort_OutputIdx_alt3    = 3
} IfxPort_OutputIdx;

typedef enum
{
    IfxPort_State_notChanged = 0,
    IfxPort_State_low        = 0,
    IfxPort_State_high       = 1
} IfxPort_State;

/* Minimal pin structure (often used indirectly) */
typedef struct
{
    Ifx_P   *port;
    uint8    pinIndex;
} IfxPort_Pin;

#endif /* IFXPORT_H */
