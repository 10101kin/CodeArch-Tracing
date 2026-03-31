#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Output index enum (subset) */
typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

/* State enum */
typedef enum {
    IfxPort_State_low  = 0,
    IfxPort_State_high = 1
} IfxPort_State;

/* Output mode */
typedef enum {
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain,
    IfxPort_OutputMode_openSource
} IfxPort_OutputMode;

/* Pad driver */
typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2,
    IfxPort_PadDriver_cmosAutomotiveSpeed3,
    IfxPort_PadDriver_cmosAutomotiveSpeed4
} IfxPort_PadDriver;

/* Pin descriptor (minimal) */
typedef struct {
    Ifx_P  *port;
    uint8   pinIndex;
} IfxPort_Pin;

/* API */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_PadDriver pad);
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex);
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State state);
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver pad);

#endif /* IFXPORT_H */
