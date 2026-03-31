#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Enums needed by production and other mocks */
typedef enum
{
    IfxPort_OutputIdx_0 = 0,
    IfxPort_OutputIdx_1,
    IfxPort_OutputIdx_2,
    IfxPort_OutputIdx_3,
    IfxPort_OutputIdx_4,
    IfxPort_OutputIdx_5,
    IfxPort_OutputIdx_6,
    IfxPort_OutputIdx_7,
    IfxPort_OutputIdx_8,
    IfxPort_OutputIdx_9,
    IfxPort_OutputIdx_10,
    IfxPort_OutputIdx_11,
    IfxPort_OutputIdx_12,
    IfxPort_OutputIdx_13,
    IfxPort_OutputIdx_14,
    IfxPort_OutputIdx_15
} IfxPort_OutputIdx;

typedef enum
{
    IfxPort_State_low  = 0,
    IfxPort_State_high = 1
} IfxPort_State;

typedef enum
{
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain = 1
} IfxPort_OutputMode;

typedef enum
{
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1,
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = 2,
    IfxPort_PadDriver_cmosAutomotiveSpeed4 = 3
} IfxPort_PadDriver;

#endif /* IFXPORT_H */
