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
    IfxPort_InputMode_noPull = 0,
    IfxPort_InputMode_pullUp = 1,
    IfxPort_InputMode_pullDown = 2
} IfxPort_InputMode;

typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

/* Basic port configuration helpers */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode);
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver driver);
void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode);
void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, uint8 altIdx);

#endif
