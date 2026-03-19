#ifndef IFXSCUWDT_H
#define IFXSCUWDT_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint16 and basic types */

/* Required APIs (TC3xx uses IfxScuWdt) */
/* Mock controls */

/* ============= Function Declarations ============= */
uint16 IfxScuWdt_getGlobalSafetyEndinitPasswordInline(void);
uint16 IfxScuWdt_getCpuWatchdogPasswordInline(Ifx_SCU_WDTCPU *watchdog);
uint16 IfxScuWdt_getSafetyWatchdogPasswordInline(void);
void   IfxScuWdt_disableCpuWatchdog(uint16 password);
void   IfxScuWdt_disableSafetyWatchdog(uint16 password);
uint16 IfxScuWdt_getCpuWatchdogPassword(void);
uint16 IfxScuWdt_getGlobalEndinitPassword(void);
uint16 IfxScuWdt_getGlobalSafetyEndinitPassword(void);
uint16 IfxScuWdt_getSafetyWatchdogPassword(void);
uint32 IfxScuWdt_Mock_GetCallCount_disableCpuWatchdog(void);
uint16 IfxScuWdt_Mock_GetLastArg_disableCpuWatchdog_password(void);
uint32 IfxScuWdt_Mock_GetCallCount_disableSafetyWatchdog(void);
uint16 IfxScuWdt_Mock_GetLastArg_disableSafetyWatchdog_password(void);
void   IfxScuWdt_Mock_SetReturn_getCpuWatchdogPassword(uint16 ret);
void   IfxScuWdt_Mock_SetReturn_getGlobalEndinitPassword(uint16 ret);
void   IfxScuWdt_Mock_SetReturn_getGlobalSafetyEndinitPassword(uint16 ret);
void   IfxScuWdt_Mock_SetReturn_getSafetyWatchdogPassword(uint16 ret);
void   IfxScuWdt_Mock_SetReturn_getGlobalSafetyEndinitPasswordInline(uint16 ret);
void   IfxScuWdt_Mock_SetReturn_getCpuWatchdogPasswordInline(uint16 ret);
void   IfxScuWdt_Mock_SetReturn_getSafetyWatchdogPasswordInline(uint16 ret);
uint32 IfxScuWdt_Mock_GetCallCount_getCpuWatchdogPasswordInline(void);
uint32 IfxScuWdt_Mock_GetCallCount_getGlobalSafetyEndinitPasswordInline(void);
uint32 IfxScuWdt_Mock_GetCallCount_getSafetyWatchdogPasswordInline(void);
uint32 IfxScuWdt_Mock_GetCallCount_getCpuWatchdogPassword(void);
uint32 IfxScuWdt_Mock_GetCallCount_getGlobalEndinitPassword(void);
uint32 IfxScuWdt_Mock_GetCallCount_getGlobalSafetyEndinitPassword(void);
uint32 IfxScuWdt_Mock_GetCallCount_getSafetyWatchdogPassword(void);
void   IfxScuWdt_Mock_Reset(void);

#endif /* IFXSCUWDT_H */