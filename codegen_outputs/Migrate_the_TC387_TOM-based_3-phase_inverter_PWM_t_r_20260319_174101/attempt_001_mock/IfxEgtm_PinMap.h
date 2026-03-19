#ifndef IFXEGTM_PINMAP_H
#define IFXEGTM_PINMAP_H

#include "illd_types/Ifx_Types.h"
#include "IfxEgtm_Atom.h"
#include "IfxPort.h"

/* Mock controls */

/* ============= Function Declarations ============= */
void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);
uint32 IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout(void);
void   IfxEgtm_PinMap_Mock_Reset(void);
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_outputMode(void);
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_padDriver(void);

#endif