#ifndef IFXEGTM_PINMAP_H
#define IFXEGTM_PINMAP_H

#include "IfxEgtm_Atom.h"
#include "IfxPort.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */
#include "IfxPort.h"    /* For IfxPort_OutputMode and IfxPort_PadDriver */

/* ============= Type Definitions ============= */
typedef struct IfxEgtm_Atom_ToutMap IfxEgtm_Atom_ToutMap;

/* Forward declaration for map struct (pointer-only usage) */
/* iLLD API declarations to mock */
/* Mock control functions */

/* ============= Function Declarations ============= */
void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);
uint32 IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout(void);
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_outputMode(void);
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_padDriver(void);
void   IfxEgtm_PinMap_Mock_Reset(void);

#endif