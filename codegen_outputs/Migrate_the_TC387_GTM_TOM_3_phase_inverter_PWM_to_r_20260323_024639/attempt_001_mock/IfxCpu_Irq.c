#include "IfxCpu_Irq.h"

static uint32 s_installInterruptHandler_count = 0u;
static uint32 s_installInterruptHandler_lastPrio = 0u;
static void  *s_installInterruptHandler_lastPtr = NULL_PTR;

void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber)
{
    s_installInterruptHandler_count++;
    s_installInterruptHandler_lastPtr = isrFuncPointer;
    s_installInterruptHandler_lastPrio = serviceReqPrioNumber;
}

uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void) { return s_installInterruptHandler_count; }
uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_serviceReqPrioNumber(void) { return s_installInterruptHandler_lastPrio; }
void*  IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_isrFuncPointer(void) { return s_installInterruptHandler_lastPtr; }

void IfxCpu_Irq_Mock_Reset(void)
{
    s_installInterruptHandler_count = 0u;
    s_installInterruptHandler_lastPrio = 0u;
    s_installInterruptHandler_lastPtr = NULL_PTR;
}
