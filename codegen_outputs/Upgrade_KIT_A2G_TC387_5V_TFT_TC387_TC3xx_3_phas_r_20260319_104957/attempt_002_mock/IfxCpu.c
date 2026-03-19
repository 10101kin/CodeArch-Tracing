#include "IfxCpu.h"

static uint32 s_getCoreId_count = 0;
static IfxCpu_Id s_getCoreId_ret = IfxCpu_Id_0;  /* default per convention */

static uint32 s_disableInterrupts_count = 0;
static boolean s_disableInterrupts_ret = (boolean)0;

static uint32 s_enableInterrupts_count = 0;
static uint32 s_forceDisableInterrupts_count = 0;

static uint32 s_waitEvent_count = 0;
static boolean s_waitEvent_ret = (boolean)0;
static uint32 s_waitEvent_lastTimeoutMs = 0u;

static uint32 s_emitEvent_count = 0;

static uint32 s_disableAllExceptMaster_count = 0;
static uint32 s_disableAllExceptMaster_lastMaster = 0u;

IfxCpu_Id IfxCpu_getCoreId(void) {
    s_getCoreId_count++;
    return s_getCoreId_ret;
}

boolean IfxCpu_disableInterrupts(void) {
    s_disableInterrupts_count++;
    return s_disableInterrupts_ret;
}

void IfxCpu_enableInterrupts(void) {
    s_enableInterrupts_count++;
}

void IfxCpu_forceDisableInterrupts(void) {
    s_forceDisableInterrupts_count++;
}

boolean IfxCpu_waitEvent(IfxCpu_syncEvent *event, uint32 timeoutMilliSec) {
    (void)event;
    s_waitEvent_count++;
    s_waitEvent_lastTimeoutMs = timeoutMilliSec;
    return s_waitEvent_ret;
}

void IfxCpu_emitEvent(IfxCpu_syncEvent *event) {
    (void)event;
    s_emitEvent_count++;
}

void IfxCpu_disableInterruptsAllExceptMaster(IfxCpu_ResourceCpu masterCpu) {
    s_disableAllExceptMaster_count++;
    s_disableAllExceptMaster_lastMaster = (uint32)masterCpu;
}

uint32 IfxCpu_Mock_GetCallCount_getCoreId(void) { return s_getCoreId_count; }
void   IfxCpu_Mock_SetReturn_getCoreId(IfxCpu_Id value) { s_getCoreId_ret = value; }

uint32 IfxCpu_Mock_GetCallCount_disableInterrupts(void) { return s_disableInterrupts_count; }
void   IfxCpu_Mock_SetReturn_disableInterrupts(boolean value) { s_disableInterrupts_ret = value; }

uint32 IfxCpu_Mock_GetCallCount_enableInterrupts(void) { return s_enableInterrupts_count; }
uint32 IfxCpu_Mock_GetCallCount_forceDisableInterrupts(void) { return s_forceDisableInterrupts_count; }

uint32 IfxCpu_Mock_GetCallCount_waitEvent(void) { return s_waitEvent_count; }
void   IfxCpu_Mock_SetReturn_waitEvent(boolean value) { s_waitEvent_ret = value; }
uint32 IfxCpu_Mock_GetLastArg_waitEvent_timeoutMilliSec(void) { return s_waitEvent_lastTimeoutMs; }

uint32 IfxCpu_Mock_GetCallCount_emitEvent(void) { return s_emitEvent_count; }

uint32 IfxCpu_Mock_GetCallCount_disableInterruptsAllExceptMaster(void) { return s_disableAllExceptMaster_count; }
uint32 IfxCpu_Mock_GetLastArg_disableInterruptsAllExceptMaster_masterCpu(void) { return s_disableAllExceptMaster_lastMaster; }

void IfxCpu_Mock_Reset(void) {
    s_getCoreId_count = 0; s_getCoreId_ret = IfxCpu_Id_0;
    s_disableInterrupts_count = 0; s_disableInterrupts_ret = (boolean)0;
    s_enableInterrupts_count = 0;
    s_forceDisableInterrupts_count = 0;
    s_waitEvent_count = 0; s_waitEvent_ret = (boolean)0; s_waitEvent_lastTimeoutMs = 0u;
    s_emitEvent_count = 0;
    s_disableAllExceptMaster_count = 0; s_disableAllExceptMaster_lastMaster = 0u;
}
