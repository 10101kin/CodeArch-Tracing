#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "IfxGtm_Tom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* Mock controls */

/* ============= Function Declarations ============= */
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_init(void);
void    IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value);
float32 IfxGtm_Tom_Timer_Mock_GetLastArg_init_frequency(void);
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_mode(void);
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_syncStart(void);
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_isrPriority(void);
void    IfxGtm_Tom_Timer_Mock_Reset(void);

#endif