/* IfxPort types + functions (enums only as needed) */
#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Port output mode enum */
typedef enum {
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain = 1
} IfxPort_OutputMode;

/* Port pad driver enum */
typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1,
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = 2,
    IfxPort_PadDriver_cmosAutomotiveSpeed4 = 3
} IfxPort_PadDriver;

/* Additional enums required by rules */
typedef enum {
    IfxPort_OutputIdx_general = 0,
    IfxPort_OutputIdx_alt1 = 1,
    IfxPort_OutputIdx_alt2 = 2,
    IfxPort_OutputIdx_alt3 = 3,
    IfxPort_OutputIdx_alt4 = 4,
    IfxPort_OutputIdx_alt5 = 5,
    IfxPort_OutputIdx_alt6 = 6,
    IfxPort_OutputIdx_alt7 = 7
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

#endif /* IFXPORT_H */
