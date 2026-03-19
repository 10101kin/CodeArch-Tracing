#ifndef IFXCPU_IRQ_H
#define IFXCPU_IRQ_H

#include "illd_types/Ifx_Types.h"

/* Mock controls */

/* ============= Function Declarations ============= */
void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber);
uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void);
void   IfxCpu_Irq_Mock_Reset(void);
void  *IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_isr(void);
uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_prio(void);

#endif