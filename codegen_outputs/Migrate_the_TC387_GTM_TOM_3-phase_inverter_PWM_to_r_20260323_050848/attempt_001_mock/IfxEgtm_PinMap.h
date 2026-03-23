#ifndef IFXEGTM_PINMAP_H
#define IFXEGTM_PINMAP_H

#include "IfxEgtm_Atom.h"
#include "IfxPort.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for basic types */
#include "IfxPort.h"    /* For IfxPort_OutputMode, IfxPort_PadDriver */

/* ============= Type Definitions ============= */
typedef struct {
    uint32 reserved;
} IfxEgtm_Atom_ToutMap;

/* Minimal Atom Tout map config struct placeholder */
/* Mock control */

/* ============= Function Declarations ============= */
void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);
uint32 IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout(void);
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_outputMode(void);
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_padDriver(void);
void IfxEgtm_PinMap_Mock_Reset(void);

#endif