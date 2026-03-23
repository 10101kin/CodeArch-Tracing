#ifndef IFXCPU_IRQ_H
#define IFXCPU_IRQ_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* Required function declarations */
/* Mock control functions */

/* ============= Function Declarations ============= */
void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber);
uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void);
void  *IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_isr(void);
uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_prio(void);
void   IfxCpu_Irq_Mock_Reset(void);

#endif /* IFXCPU_IRQ_H */