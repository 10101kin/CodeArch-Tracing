#include "IfxCpu_Irq.h"

/* Call counter */
static uint32 s_installInterruptHandler_count = 0u;

/* Captured last arguments */
static uint32 s_last_install_prio = 0u;
static void  *s_last_install_isr  = NULL_PTR;

void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber)
{
    s_installInterruptHandler_count++;
    s_last_install_isr  = isrFuncPointer;
    s_last_install_prio = serviceReqPrioNumber;
}

uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void)
{
    return s_installInterruptHandler_count;
}

uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_prio(void)
{
    return s_last_install_prio;
}

void* IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_isr(void)
{
    return s_last_install_isr;
}

void IfxCpu_Irq_Mock_Reset(void)
{
    s_installInterruptHandler_count = 0u;
    s_last_install_prio = 0u;
    s_last_install_isr  = NULL_PTR;
}
