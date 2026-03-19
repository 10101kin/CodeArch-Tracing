#ifndef IFXGTM_PINMAP_H
#define IFXGTM_PINMAP_H

#include "illd_types/Ifx_Types.h"
#include "IfxGtm_Tom.h"
#include "IfxPort.h"
#include "IfxPort.h"         /* for IfxPort_OutputMode, IfxPort_PadDriver */
#include "IfxGtm_Tom_Timer.h" /* for IfxGtm_Tom_ToutMap (opaque) */

/* Mock control */

/* ============= Function Declarations ============= */
void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);
uint32 IfxGtm_PinMap_Mock_GetCallCount_setTomTout(void);
uint32 IfxGtm_PinMap_Mock_GetLastArg_setTomTout_outputMode(void);
uint32 IfxGtm_PinMap_Mock_GetLastArg_setTomTout_padDriver(void);
void   IfxGtm_PinMap_Mock_Reset(void);

#endif