#ifndef IFXPORT_H
#define IFXPORT_H
#include "mock_egtm_atom_tmadc_consolidated.h"

typedef enum {
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain = 1
} IfxPort_OutputMode;

typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1,
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = 2,
    IfxPort_PadDriver_cmosAutomotiveSpeed4 = 3
} IfxPort_PadDriver;

typedef enum {
    IfxPort_InputMode_noPullDevice = 0,
    IfxPort_InputMode_pullDown = 1,
    IfxPort_InputMode_pullUp = 2
} IfxPort_InputMode;

typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

#endif /* IFXPORT_H */
