#ifndef IFXWTU_H
#define IFXWTU_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint16, uint32, boolean, float32 types */

/* TC4xx Watchdog Timer Unit (WTU) mocks */
/* Mock controls */

/* ============= Function Declarations ============= */
void   IfxWtu_disableCpuWatchdog(uint16 password);
void   IfxWtu_disableSystemWatchdog(uint16 password);
uint16 IfxWtu_getCpuWatchdogPassword(void);
uint16 IfxWtu_getSystemWatchdogPassword(void);
uint32 IfxWtu_Mock_GetCallCount_disableCpuWatchdog(void);
uint32 IfxWtu_Mock_GetCallCount_disableSystemWatchdog(void);
uint32 IfxWtu_Mock_GetCallCount_getCpuWatchdogPassword(void);
uint32 IfxWtu_Mock_GetCallCount_getSystemWatchdogPassword(void);
void   IfxWtu_Mock_SetReturn_getCpuWatchdogPassword(uint16 value);
void   IfxWtu_Mock_SetReturn_getSystemWatchdogPassword(uint16 value);
uint16 IfxWtu_Mock_GetLastArg_disableCpuWatchdog_password(void);
uint16 IfxWtu_Mock_GetLastArg_disableSystemWatchdog_password(void);
void   IfxWtu_Mock_Reset(void);

#endif