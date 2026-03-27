#ifndef IFXPORT_H
#define IFXPORT_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Enums needed by other headers */
typedef enum { IfxPort_OutputIdx_general = 0 } IfxPort_OutputIdx;

typedef enum { IfxPort_State_low = 0, IfxPort_State_high = 1 } IfxPort_State;

typedef enum { IfxPort_OutputMode_pushPull = 0, IfxPort_OutputMode_openDrain = 1 } IfxPort_OutputMode;

typedef enum { IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0, IfxPort_PadDriver_cmosAutomotiveSpeed2 } IfxPort_PadDriver;

typedef enum { IfxPort_Mode_outputPushPull = 0, IfxPort_Mode_inputPullUp = 1 } IfxPort_Mode;

typedef struct { Ifx_P *port; uint8 pin; } IfxPort_Pin;

/* Basic helpers often referenced */
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex);

#endif /* IFXPORT_H */
