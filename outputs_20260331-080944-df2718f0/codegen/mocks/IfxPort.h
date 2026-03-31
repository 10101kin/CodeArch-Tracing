#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* ===== Enums ===== */

typedef enum
{
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain = 1
} IfxPort_OutputMode;

typedef enum
{
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1
} IfxPort_PadDriver;

typedef enum
{
    IfxPort_OutputIdx_general = 0,
    IfxPort_OutputIdx_alt1    = 1,
    IfxPort_OutputIdx_alt2    = 2
} IfxPort_OutputIdx;

typedef enum
{
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

/* ===== Function declarations (minimal) ===== */

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode);
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver driver);
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);

#endif /* IFXPORT_H */
