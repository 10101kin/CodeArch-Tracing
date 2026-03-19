#ifndef BSP_H
#define BSP_H

#include "illd_types/Ifx_Types.h"  /* for Ifx_TickTime */

/* Mock control */

/* ============= Function Declarations ============= */
void waitTime(Ifx_TickTime ticks);
uint32      Bsp_Mock_GetCallCount_waitTime(void);
Ifx_TickTime Bsp_Mock_GetLastArg_waitTime_ticks(void);
void        Bsp_Mock_Reset(void);

#endif