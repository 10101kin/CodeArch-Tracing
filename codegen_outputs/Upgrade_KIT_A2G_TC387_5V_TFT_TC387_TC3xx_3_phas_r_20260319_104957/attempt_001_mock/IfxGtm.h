#ifndef IFXGTM_H
#define IFXGTM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED */

/* Mock controls */
/* GTM enable/disable functions (auto-injected for TDD) */

/* ============= Function Declarations ============= */
void IfxGtm_enable(Ifx_GTM *gtm);
uint32 IfxGtm_Mock_GetCallCount_enable(void);
void   IfxGtm_Mock_Reset(void);
void IfxGtm_enable(Ifx_GTM *gtm);
boolean IfxGtm_isEnabled(Ifx_GTM *gtm);

#endif /* IFXGTM_H */