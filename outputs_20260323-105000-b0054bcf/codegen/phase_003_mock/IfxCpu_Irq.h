#ifndef IFXCPU_IRQ_H
#define IFXCPU_IRQ_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint32 */

/* API declaration (exact signature) */
/* Mock control: call counters */
/* Mock control: capture last args */
/* Reset mock state */

/* ============= Function Declarations ============= */
void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber);
uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void);
uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_prio(void);
void*  IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_isr(void);
void IfxCpu_Irq_Mock_Reset(void);

#endif