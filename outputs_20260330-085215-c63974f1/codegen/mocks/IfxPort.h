#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Basic Port enums required by production */
typedef enum {
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain = 1
} IfxPort_OutputMode;

typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1,
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = 2,
    IfxPort_PadDriver_cmosAutomotiveSpeed4 = 3,
    IfxPort_PadDriver_medium = 4
} IfxPort_PadDriver;

typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

/* Function declarations used by production/tests */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_PadDriver padDriver);
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);

#endif /* IFXPORT_H */
