#ifndef IFXEGTM_PINMAP_H
#define IFXEGTM_PINMAP_H

#include "IfxEgtm_Atom.h"
#include "IfxPort.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* ============= Type Definitions ============= */
typedef struct IfxEgtm_Atom_ToutMap IfxEgtm_Atom_ToutMap;

/* Opaque forward declaration for map type */
/* API declaration */
/* Mock control - call counts */
/* Capture getters for value params */
/* Reset */

/* ============= Function Declarations ============= */
void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);
uint32 IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout(void);
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_outputMode(void);
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_padDriver(void);
void   IfxEgtm_PinMap_Mock_Reset(void);

#endif /* IFXEGTM_PINMAP_H */