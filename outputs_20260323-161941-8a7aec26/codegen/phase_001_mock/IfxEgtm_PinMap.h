#ifndef IFXEGTM_PINMAP_H
#define IFXEGTM_PINMAP_H

#include "IfxEgtm_Atom.h"
#include "IfxPort.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */
#include "IfxPort.h"    /* Uses IfxPort_OutputMode, IfxPort_PadDriver */

/* iLLD API declarations */
/* Mock controls: call counts */
/* Mock controls: last-argument capture getters */
/* Mock controls: reset */

/* ============= Function Declarations ============= */
void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);
uint32 IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout(void);
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_outputMode(void);
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_padDriver(void);
void IfxEgtm_PinMap_Mock_Reset(void);

#endif /* IFXEGTM_PINMAP_H */