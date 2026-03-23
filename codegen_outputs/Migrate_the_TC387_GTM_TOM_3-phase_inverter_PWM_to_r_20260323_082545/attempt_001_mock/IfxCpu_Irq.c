#include "IfxCpu_Irq.h"

/* Call counters */
static uint32 s_installInterruptHandler_count = 0;

/* Argument captures */
static void *s_installInterruptHandler_lastIsr = NULL_PTR;
static uint32 s_installInterruptHandler_lastPrio = 0u;

/* iLLD stub */
void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber) {
    (void)isrFuncPointer; (void)serviceReqPrioNumber;
    s_installInterruptHandler_count++;
    s_installInterruptHandler_lastIsr = isrFuncPointer;
    s_installInterruptHandler_lastPrio = serviceReqPrioNumber;
}

/* Mock controls */
uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void) { return s_installInterruptHandler_count; }
void*  IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_isr(void) { return s_installInterruptHandler_lastIsr; }
uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_prio(void) { return s_installInterruptHandler_lastPrio; }

void IfxCpu_Irq_Mock_Reset(void) {
    s_installInterruptHandler_count = 0u;
    s_installInterruptHandler_lastIsr = NULL_PTR;
    s_installInterruptHandler_lastPrio = 0u;
}
