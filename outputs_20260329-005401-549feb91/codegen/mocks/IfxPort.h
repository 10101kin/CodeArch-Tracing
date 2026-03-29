#ifndef IFXPORT_H
#define IFXPORT_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Port enums and types */
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
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

/* Minimal pin abstraction if needed */
typedef struct {
    Ifx_P* port;
    uint8  pinIndex;
} IfxPort_Pin;

/* Functions used in production (provide prototypes for tests) */
void IfxPort_togglePin(Ifx_P* port, uint8 pin);
void IfxPort_setPinModeOutput(Ifx_P* port, uint8 pin, IfxPort_OutputMode mode, IfxPort_PadDriver pad);

#endif /* IFXPORT_H */
