#ifndef IFXPORT_H
#define IFXPORT_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Enums first (used by structs or functions) */
typedef enum
{
    IfxPort_OutputMode_pushPull   = 0,
    IfxPort_OutputMode_openDrain  = 1
} IfxPort_OutputMode;

typedef enum
{
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1,
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = 2,
    IfxPort_PadDriver_cmosAutomotiveSpeed4 = 3
} IfxPort_PadDriver;

typedef enum
{
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum
{
    IfxPort_State_notChanged = 0,
    IfxPort_State_low        = 1,
    IfxPort_State_high       = 2
} IfxPort_State;

/* Function declarations */
void IfxPort_togglePin(Ifx_P *port, uint8 pin);
void IfxPort_setPinHigh(Ifx_P *port, uint8 pin);
void IfxPort_setPinLow(Ifx_P *port, uint8 pin);
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pin, IfxPort_OutputMode mode, IfxPort_PadDriver pad);

#endif /* IFXPORT_H */
