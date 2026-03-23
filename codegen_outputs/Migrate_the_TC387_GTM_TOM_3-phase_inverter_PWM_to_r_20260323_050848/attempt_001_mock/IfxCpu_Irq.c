#include "IfxCpu_Irq.h"

static uint32 s_installInterruptHandler_count = 0;
static uint32 s_last_installInterruptHandler_prio = 0;

void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber) {
    (void)isrFuncPointer;
    s_installInterruptHandler_count++;
    s_last_installInterruptHandler_prio = serviceReqPrioNumber;
}

uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void) { return s_installInterruptHandler_count; }
uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_serviceReqPrioNumber(void) { return s_last_installInterruptHandler_prio; }

void IfxCpu_Irq_Mock_Reset(void) {
    s_installInterruptHandler_count = 0;
    s_last_installInterruptHandler_prio = 0;
}
