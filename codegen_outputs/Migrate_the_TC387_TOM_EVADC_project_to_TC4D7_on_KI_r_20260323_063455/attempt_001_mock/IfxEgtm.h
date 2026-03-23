#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for basic types */

/* API declarations */
/* Mock control: call counts */
/* Mock control */
/* EGTM enable/disable functions (auto-injected for TDD) */

/* ============= Function Declarations ============= */
void IfxEgtm_enable(Ifx_EGTM *egtm);
uint32 IfxEgtm_Mock_GetCallCount_enable(void);
void IfxEgtm_Mock_Reset(void);
void IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */