#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Enums and types */
typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

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

/* Function declarations */
void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode);
void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State state);
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);

#endif /* IFXPORT_H */
