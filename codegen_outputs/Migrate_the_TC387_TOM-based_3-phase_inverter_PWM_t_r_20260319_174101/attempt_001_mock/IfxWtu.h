#ifndef IFXWTU_H
#define IFXWTU_H

#include "illd_types/Ifx_Types.h"

/* Mock controls */
/* Set returns */
/* Captures */

/* ============= Function Declarations ============= */
uint16 IfxWtu_getCpuWatchdogPasswordInline(Ifx_WTU_WDTCPU *watchdog);
uint16 IfxWtu_getSecurityWatchdogPasswordInline(void);
uint16 IfxWtu_getSystemWatchdogPasswordInline(void);
uint16 IfxWtu_getCpuWatchdogPassword(void);
uint16 IfxWtu_getSecurityWatchdogPassword(void);
uint16 IfxWtu_getSystemWatchdogPassword(void);
void   IfxWtu_disableCpuWatchdog(uint16 password);
void   IfxWtu_disableSecurityWatchdog(uint16 password);
void   IfxWtu_disableSystemWatchdog(uint16 password);
uint32 IfxWtu_Mock_GetCallCount_getCpuWatchdogPasswordInline(void);
uint32 IfxWtu_Mock_GetCallCount_getSecurityWatchdogPasswordInline(void);
uint32 IfxWtu_Mock_GetCallCount_getSystemWatchdogPasswordInline(void);
uint32 IfxWtu_Mock_GetCallCount_getCpuWatchdogPassword(void);
uint32 IfxWtu_Mock_GetCallCount_getSecurityWatchdogPassword(void);
uint32 IfxWtu_Mock_GetCallCount_getSystemWatchdogPassword(void);
uint32 IfxWtu_Mock_GetCallCount_disableCpuWatchdog(void);
uint32 IfxWtu_Mock_GetCallCount_disableSecurityWatchdog(void);
uint32 IfxWtu_Mock_GetCallCount_disableSystemWatchdog(void);
void   IfxWtu_Mock_Reset(void);
void   IfxWtu_Mock_SetReturn_getCpuWatchdogPasswordInline(uint16 ret);
void   IfxWtu_Mock_SetReturn_getSecurityWatchdogPasswordInline(uint16 ret);
void   IfxWtu_Mock_SetReturn_getSystemWatchdogPasswordInline(uint16 ret);
void   IfxWtu_Mock_SetReturn_getCpuWatchdogPassword(uint16 ret);
void   IfxWtu_Mock_SetReturn_getSecurityWatchdogPassword(uint16 ret);
void   IfxWtu_Mock_SetReturn_getSystemWatchdogPassword(uint16 ret);
uint16 IfxWtu_Mock_GetLastArg_disableCpuWatchdog_password(void);
uint16 IfxWtu_Mock_GetLastArg_disableSecurityWatchdog_password(void);
uint16 IfxWtu_Mock_GetLastArg_disableSystemWatchdog_password(void);

#endif