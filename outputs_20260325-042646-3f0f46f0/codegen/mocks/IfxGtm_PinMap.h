#ifndef IFXGTM_PINMAP_H
#define IFXGTM_PINMAP_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint32, boolean, float32 types */
#include "IfxPort.h"
#include "IfxGtm_Tom.h"
#include "IfxPort.h"

/* Ensure Port-related enums are visible to this header */
/* Pull in real pin map types via GCC include_next (for IfxGtm_Tom_ToutMap, etc.) */
#ifdef __GNUC__
# include_next "IfxGtm_PinMap.h"
/* Exact function declaration from SW Detailed Design */
/* Mock controls */

/* ============= Function Declarations ============= */
void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);
uint32 IfxGtm_PinMap_Mock_GetCallCount_setTomTout(void);
uint32 IfxGtm_PinMap_Mock_GetLastArg_setTomTout_outputMode(void);
uint32 IfxGtm_PinMap_Mock_GetLastArg_setTomTout_padDriver(void);
void   IfxGtm_PinMap_Mock_Reset(void);

#endif /* IFXGTM_PINMAP_H */