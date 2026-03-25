#ifndef IFXGTM_PINMAP_H
#define IFXGTM_PINMAP_H

#include "illd_types/Ifx_Types.h"
#include "IfxGtm_Tom.h"
#include "IfxPort.h"
#include "IfxPort.h"  /* For IfxPort_OutputMode, IfxPort_PadDriver */

/* ============= Type Definitions ============= */
struct IfxGtm_Tom_ToutMap { uint32 __mock; };
/* IfxGtm_Tom_ToutMap - defined in Ifx_Types.h */

/* Opaque map type for TOM TOUT mapping (pointer only) */
/* Mock control */

/* ============= Function Declarations ============= */
void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);
uint32 IfxGtm_PinMap_Mock_GetCallCount_setTomTout(void);
uint32 IfxGtm_PinMap_Mock_GetLastArg_setTomTout_outputMode(void);
uint32 IfxGtm_PinMap_Mock_GetLastArg_setTomTout_padDriver(void);
void IfxGtm_PinMap_Mock_Reset(void);

#endif