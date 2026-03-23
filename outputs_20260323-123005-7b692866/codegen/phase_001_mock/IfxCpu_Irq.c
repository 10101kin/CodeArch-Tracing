#include "IfxCpu_Irq.h"

static uint32 s_cnt_installInterruptHandler = 0u;
static uint32 s_last_installInterruptHandler_prio = 0u;
static void  *s_last_installInterruptHandler_isr = NULL_PTR;

void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber) {
    s_cnt_installInterruptHandler++;
    s_last_installInterruptHandler_isr = isrFuncPointer;
    s_last_installInterruptHandler_prio = serviceReqPrioNumber;
    (void)isrFuncPointer;
    (void)serviceReqPrioNumber;
}

uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void) {
    return s_cnt_installInterruptHandler;
}

uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_prio(void) {
    return s_last_installInterruptHandler_prio;
}

void* IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_isr(void) {
    return s_last_installInterruptHandler_isr;
}

void IfxCpu_Irq_Mock_Reset(void) {
    s_cnt_installInterruptHandler = 0u;
    s_last_installInterruptHandler_prio = 0u;
    s_last_installInterruptHandler_isr = NULL_PTR;
}
