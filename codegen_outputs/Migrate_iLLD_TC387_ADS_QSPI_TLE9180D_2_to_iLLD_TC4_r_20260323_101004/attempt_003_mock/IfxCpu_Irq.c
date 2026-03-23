#include "IfxCpu_Irq.h"

/* Call counters */
static uint32 s_installInterruptHandler_count = 0u;

/* Captured last numeric arg */
static uint32 s_installInterruptHandler_last_prio = 0u;

/* API implementation */
void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber)
{
    (void)isrFuncPointer;
    s_installInterruptHandler_count++;
    s_installInterruptHandler_last_prio = serviceReqPrioNumber;
}

/* Mock control implementations */
uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void)
{
    return s_installInterruptHandler_count;
}

uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_serviceReqPrioNumber(void)
{
    return s_installInterruptHandler_last_prio;
}

void IfxCpu_Irq_Mock_Reset(void)
{
    s_installInterruptHandler_count = 0u;
    s_installInterruptHandler_last_prio = 0u;
}
