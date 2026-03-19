#ifndef BSP_H
#define BSP_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint32, boolean, etc. */

/* waitTime API (NOTE: initTime does NOT exist) */
/* Mock controls */

/* ============= Function Declarations ============= */
void waitTime(Ifx_TickTime ticks);
uint32 Bsp_Mock_GetCallCount_waitTime(void);
void   Bsp_Mock_Reset(void);
Ifx_TickTime Bsp_Mock_GetLastArg_waitTime_ticks(void);

#endif