#ifndef IFXCPU_IRQ_H
#define IFXCPU_IRQ_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD API declaration */
/* Mock control functions */
/* Reset all counters and captured values */

/* ============= Function Declarations ============= */
void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber);
uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void);
uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_serviceReqPrioNumber(void);
void IfxCpu_Irq_Mock_Reset(void);

#endif /* IFXCPU_IRQ_H */