/* Mock IfxPort.h */
#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Enums minimally required */
typedef enum {
    IfxPort_State_notChanged = 0,
    IfxPort_State_low = 1,
    IfxPort_State_high = 2
} IfxPort_State;

typedef enum {
    IfxPort_OutputIdx_general = 0
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_Omode_pushPull = 0,
    IfxPort_Omode_openDrain = 1
} IfxPort_OutputMode;

typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0
} IfxPort_PadDriver;

/* Simple pin type */
typedef struct {
    volatile Ifx_P *port;
    uint8           pinIndex;
} IfxPort_Pin;

/* Optionally some basic API (unused by tests) */
static inline void IfxPort_setPinState(volatile Ifx_P *port, uint8 pinIndex, IfxPort_State state)
{ (void)port; (void)pinIndex; (void)state; }

#endif /* IFXPORT_H */
