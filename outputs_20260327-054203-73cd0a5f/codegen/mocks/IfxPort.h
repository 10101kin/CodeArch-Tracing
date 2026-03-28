#ifndef IFXPORT_H
#define IFXPORT_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Enums required */
typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

typedef enum {
    IfxPort_Mode_outputPushPullGeneral = 0,
    IfxPort_Mode_inputPullUp = 1
} IfxPort_Mode;

typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0
} IfxPort_PadDriver;

/* Functions */
void IfxPort_setPinState(Ifx_P* port, uint8 pinIndex, IfxPort_State state);
void IfxPort_setPinMode(Ifx_P* port, uint8 pinIndex, IfxPort_Mode mode);
void IfxPort_togglePin(Ifx_P* port, uint8 pinIndex);

#endif /* IFXPORT_H */
