#ifndef IFXWTU_H
#define IFXWTU_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD API declarations (TC4xx Watchdog Unit) */
/* Required password-based APIs for TC4xx */
/* Mock control: call counts */
/* Mock control: return setters/getters for non-void */
/* Mock control: argument capture for value-carrying params */
/* Mock control: reset */

/* ============= Function Declarations ============= */
void   IfxWtu_initCpuWatchdog(Ifx_WTU_WDTCPU *wtu, IfxWtu_Config *config);
void   IfxWtu_initConfig(IfxWtu_Config *config);
void   IfxWtu_disableCpuWatchdog(uint16 password);
void   IfxWtu_disableSystemWatchdog(uint16 password);
uint16 IfxWtu_getCpuWatchdogPassword(void);
uint16 IfxWtu_getSystemWatchdogPassword(void);
uint32 IfxWtu_Mock_GetCallCount_initCpuWatchdog(void);
uint32 IfxWtu_Mock_GetCallCount_initConfig(void);
uint32 IfxWtu_Mock_GetCallCount_disableCpuWatchdog(void);
uint32 IfxWtu_Mock_GetCallCount_disableSystemWatchdog(void);
uint32 IfxWtu_Mock_GetCallCount_getCpuWatchdogPassword(void);
uint32 IfxWtu_Mock_GetCallCount_getSystemWatchdogPassword(void);
void   IfxWtu_Mock_SetReturn_getCpuWatchdogPassword(uint16 value);
uint16 IfxWtu_Mock_GetReturn_getCpuWatchdogPassword(void);
void   IfxWtu_Mock_SetReturn_getSystemWatchdogPassword(uint16 value);
uint16 IfxWtu_Mock_GetReturn_getSystemWatchdogPassword(void);
uint16 IfxWtu_Mock_GetLastArg_disableCpuWatchdog_password(void);
uint16 IfxWtu_Mock_GetLastArg_disableSystemWatchdog_password(void);
void IfxWtu_Mock_Reset(void);

#endif /* IFXWTU_H */