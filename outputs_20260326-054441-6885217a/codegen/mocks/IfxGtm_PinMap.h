#ifndef IFXGTM_PINMAP_H
#define IFXGTM_PINMAP_H

#include "IfxGtm_Tom.h"
#include "IfxPort.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */
#include "IfxPort.h"    /* Uses IfxPort_OutputMode, IfxPort_PadDriver */

/* ============= Type Definitions ============= */
/* IfxGtm_Tom_ToutMap - defined in ifxgtm_tom.h */

/* Forward declaration for TOM TOUT map (pointer-only) */
/* API declarations */
/* Mock controls */

/* ============= Function Declarations ============= */
void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);
uint32 IfxGtm_PinMap_Mock_GetCallCount_setTomTout(void);
uint32 IfxGtm_PinMap_Mock_GetLastArg_setTomTout_outputMode(void);
uint32 IfxGtm_PinMap_Mock_GetLastArg_setTomTout_padDriver(void);
void IfxGtm_PinMap_Mock_Reset(void);

#endif