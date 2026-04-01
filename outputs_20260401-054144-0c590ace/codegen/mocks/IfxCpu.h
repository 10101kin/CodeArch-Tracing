#ifndef IFXCPU_H
#define IFXCPU_H
#include "mock_egtm_atom_tmadc_consolidated.h"
/* Interrupt helper APIs */
void IfxCpu_enableInterrupts(void);
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), Ifx_Priority priority);
#endif
