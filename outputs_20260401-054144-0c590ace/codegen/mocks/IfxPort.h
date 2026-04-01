/* IfxPort.h - basic port types */
#ifndef IFXPORT_H
#define IFXPORT_H
#include "mock_egtm_atom_tmadc_consolidated.h"

typedef enum {
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain = 1
} IfxPort_OutputMode;

typedef enum {
    IfxPort_InputMode_noPullDevice = 0,
    IfxPort_InputMode_pullUp = 1,
    IfxPort_InputMode_pullDown = 2
} IfxPort_InputMode;

typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1,
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = 2,
    IfxPort_PadDriver_cmosAutomotiveSpeed4 = 3
} IfxPort_PadDriver;

typedef enum {
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

typedef enum {
    IfxPort_OutputIdx_general = -1,
    IfxPort_OutputIdx_0 = 0,
    IfxPort_OutputIdx_1 = 1,
    IfxPort_OutputIdx_2 = 2,
    IfxPort_OutputIdx_3 = 3
} IfxPort_OutputIdx;

/* Function declarations often used by drivers */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx idx);
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver driver);
void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode);
void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, uint8 altFn);

#endif /* IFXPORT_H */
