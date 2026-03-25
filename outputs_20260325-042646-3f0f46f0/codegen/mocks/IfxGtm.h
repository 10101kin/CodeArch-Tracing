#ifndef IFXGTM_H
#define IFXGTM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint32, boolean, float32 types */

/* Pull in real GTM types via GCC include_next (for Ifx_GTM, etc.) */
#ifdef __GNUC__
# include_next "IfxGtm.h"
/* Exact function declaration from SW Detailed Design */
/* Mock controls */
/* GTM enable/disable functions (auto-injected for TDD) */

/* ============= Function Declarations ============= */
void IfxGtm_enable(Ifx_GTM *gtm);
uint32 IfxGtm_Mock_GetCallCount_enable(void);
void   IfxGtm_Mock_Reset(void);
void IfxGtm_enable(Ifx_GTM *gtm);
boolean IfxGtm_isEnabled(Ifx_GTM *gtm);

#endif /* IFXGTM_H */