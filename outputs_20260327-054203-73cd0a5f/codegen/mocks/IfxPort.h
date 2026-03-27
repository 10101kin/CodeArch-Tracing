#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Minimal enums often referenced */
typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_low = 0,
    IfxPort_State_high
} IfxPort_State;

typedef enum {
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain
} IfxPort_OutputMode;

typedef enum {
    IfxPort_PadDriver_medium = 0,
    IfxPort_PadDriver_strong,
    IfxPort_PadDriver_weak
} IfxPort_PadDriver;

/* Basic pin control functions */
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex);

#endif /* IFXPORT_H */
