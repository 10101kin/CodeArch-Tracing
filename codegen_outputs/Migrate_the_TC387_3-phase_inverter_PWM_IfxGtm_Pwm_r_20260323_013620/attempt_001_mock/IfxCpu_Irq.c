#include "IfxCpu_Irq.h"

static uint32 s_installHandler_count = 0;
static uint32 s_installHandler_lastPrio = 0u;

void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber)
{
    (void)isrFuncPointer;
    s_installHandler_count++;
    s_installHandler_lastPrio = serviceReqPrioNumber;
}

uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void) { return s_installHandler_count; }
uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_serviceReqPrioNumber(void) { return s_installHandler_lastPrio; }

void IfxCpu_Irq_Mock_Reset(void)
{
    s_installHandler_count = 0u;
    s_installHandler_lastPrio = 0u;
}
