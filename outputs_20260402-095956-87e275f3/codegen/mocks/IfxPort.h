#ifndef IFXPORT_H
#define IFXPORT_H
#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Basic IfxPort types/enums used by production */
typedef enum { IfxPort_OutputMode_pushPull = ((0u<<1u)|1u), IfxPort_OutputMode_openDrain = ((1u<<1u)|1u) } IfxPort_OutputMode;
typedef enum {
    IfxPort_OutputIdx_general = ((0u<<4u)|1u), IfxPort_OutputIdx_alt1, IfxPort_OutputIdx_alt2, IfxPort_OutputIdx_alt3,
    IfxPort_OutputIdx_alt4, IfxPort_OutputIdx_alt5, IfxPort_OutputIdx_alt6, IfxPort_OutputIdx_alt7,
    IfxPort_OutputIdx_alt8, IfxPort_OutputIdx_alt9, IfxPort_OutputIdx_alt10, IfxPort_OutputIdx_alt11,
    IfxPort_OutputIdx_alt12, IfxPort_OutputIdx_alt13, IfxPort_OutputIdx_alt14, IfxPort_OutputIdx_alt15
} IfxPort_OutputIdx;

typedef enum { IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0 } IfxPort_PadDriver; /* minimal */

/* Functions used by production */
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index);

#endif /* IFXPORT_H */
