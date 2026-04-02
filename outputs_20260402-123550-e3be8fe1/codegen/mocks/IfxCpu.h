#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#ifndef IFXCPU_H
#define IFXCPU_H

/* Minimal IfxCpu interface used by drivers */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), int vectabNum, int priority);
void IfxCpu_enableInterrupts(void);

#endif /* IFXCPU_H */
