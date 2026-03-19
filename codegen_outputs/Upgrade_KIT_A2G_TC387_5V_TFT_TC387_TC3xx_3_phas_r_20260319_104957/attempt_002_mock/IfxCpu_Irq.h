#ifndef IFXCPU_IRQ_H
#define IFXCPU_IRQ_H

#include "illd_types/Ifx_Types.h"

/* Mock control */

/* ============= Function Declarations ============= */
void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber);
uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void);
uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_prio(void);
void*  IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_isr(void);
void   IfxCpu_Irq_Mock_Reset(void);

#endif