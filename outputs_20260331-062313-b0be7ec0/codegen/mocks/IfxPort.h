#ifndef IFXPORT_H
#define IFXPORT_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Port output mode */
typedef enum {
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain
} IfxPort_OutputMode;

/* Port pad driver */
typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2,
    IfxPort_PadDriver_cmosAutomotiveSpeed3,
    IfxPort_PadDriver_cmosAutomotiveSpeed4
} IfxPort_PadDriver;

/* Optional additional enums often used */
typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

#endif /* IFXPORT_H */
