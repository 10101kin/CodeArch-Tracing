#ifndef IFXCPU_H
#define IFXCPU_H
#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Minimal CPU API used in iLLD examples */
void IfxCpu_Irq_installInterruptHandler(void (*handler)(void), int vector);
void IfxCpu_enableInterrupts(void);

#endif /* IFXCPU_H */
