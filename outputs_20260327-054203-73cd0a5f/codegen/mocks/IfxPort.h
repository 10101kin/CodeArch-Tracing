#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Enums first (ordering critical) */
typedef enum { IfxPort_OutputIdx_general=0 } IfxPort_OutputIdx;

typedef enum { IfxPort_State_low=0, IfxPort_State_high=1 } IfxPort_State;

typedef enum { IfxPort_OutputMode_pushPull=0, IfxPort_OutputMode_openDrain=1 } IfxPort_OutputMode;

typedef enum { IfxPort_PadDriver_cmosAutomotiveSpeed1=0, IfxPort_PadDriver_cmosAutomotiveSpeed2=1 } IfxPort_PadDriver;

/* Simple pin abstraction */
typedef struct
{
    Ifx_P *port;
    uint8  pinIndex;
} IfxPort_Pin;

/* Basic Port API used by production */
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex);
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, int mode);
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, int pad);

#endif /* IFXPORT_H */
