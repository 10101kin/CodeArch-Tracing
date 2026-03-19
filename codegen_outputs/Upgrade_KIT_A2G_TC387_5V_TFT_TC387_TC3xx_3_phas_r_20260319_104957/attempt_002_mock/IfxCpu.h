#ifndef IFXCPU_H
#define IFXCPU_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD API declarations */
/* Mock control functions */

/* ============= Function Declarations ============= */
IfxCpu_Id IfxCpu_getCoreId(void);
boolean IfxCpu_disableInterrupts(void);
void IfxCpu_enableInterrupts(void);
void IfxCpu_forceDisableInterrupts(void);
boolean IfxCpu_waitEvent(IfxCpu_syncEvent *event, uint32 timeoutMilliSec);
void IfxCpu_emitEvent(IfxCpu_syncEvent *event);
void IfxCpu_disableInterruptsAllExceptMaster(IfxCpu_ResourceCpu masterCpu);
uint32 IfxCpu_Mock_GetCallCount_getCoreId(void);
void   IfxCpu_Mock_SetReturn_getCoreId(IfxCpu_Id value);
uint32 IfxCpu_Mock_GetCallCount_disableInterrupts(void);
void   IfxCpu_Mock_SetReturn_disableInterrupts(boolean value);
uint32 IfxCpu_Mock_GetCallCount_enableInterrupts(void);
uint32 IfxCpu_Mock_GetCallCount_forceDisableInterrupts(void);
uint32 IfxCpu_Mock_GetCallCount_waitEvent(void);
void   IfxCpu_Mock_SetReturn_waitEvent(boolean value);
uint32 IfxCpu_Mock_GetLastArg_waitEvent_timeoutMilliSec(void);
uint32 IfxCpu_Mock_GetCallCount_emitEvent(void);
uint32 IfxCpu_Mock_GetCallCount_disableInterruptsAllExceptMaster(void);
uint32 IfxCpu_Mock_GetLastArg_disableInterruptsAllExceptMaster_masterCpu(void);
void   IfxCpu_Mock_Reset(void);

#endif