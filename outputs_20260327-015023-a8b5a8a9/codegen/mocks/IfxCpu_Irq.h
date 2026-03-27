#ifndef IFXCPU_IRQ_H
#define IFXCPU_IRQ_H

#include "mock_qspi.h"

void       IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber);
IfxSrc_Tos IfxCpu_Irq_getTos(void);

#endif /* IFXCPU_IRQ_H */
