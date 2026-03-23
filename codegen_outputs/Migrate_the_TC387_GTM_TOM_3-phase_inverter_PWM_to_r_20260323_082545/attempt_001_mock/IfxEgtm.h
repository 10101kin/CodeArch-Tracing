#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD function declarations */
/* Mock controls */
/* EGTM enable/disable functions (auto-injected for TDD) */

/* ============= Function Declarations ============= */
void IfxEgtm_enable(Ifx_EGTM *egtm);
uint32 IfxEgtm_Mock_GetCallCount_enable(void);
void   IfxEgtm_Mock_Reset(void);
void IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif