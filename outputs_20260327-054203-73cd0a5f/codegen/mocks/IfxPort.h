#ifndef IFXPORT_H
#define IFXPORT_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Common IfxPort enums needed by production */
typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_State_low = 0,
    IfxPort_State_high = 1
} IfxPort_State;

typedef enum {
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain = 1
} IfxPort_OutputMode;

typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1
} IfxPort_PadDriver;

/* Optional minimal port pin type */
typedef struct {
    void *port; /* points to Ifx_P */
    uint8 pinIndex;
} IfxPort_Pin;

#endif /* IFXPORT_H */
