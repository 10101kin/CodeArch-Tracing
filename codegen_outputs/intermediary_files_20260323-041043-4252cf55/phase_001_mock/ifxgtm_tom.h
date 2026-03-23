#ifndef IFXGTM_TOM_H
#define IFXGTM_TOM_H

#include "illd_types/Ifx_Types.h"

/*
 * Auto-generated stub for IfxGtm_Tom.h
 * This header was included by other mock headers but didn't exist.
 * Add type/function definitions here as needed.
 */

/* GTM TOM (Timer Output Module) types */
/* IfxGtm_Tom - from illd_types/Ifx_Types.h */
/* IfxGtm_Tom_Ch - from illd_types/Ifx_Types.h */

/* IfxPort_OutputIdx - from illd_types/Ifx_Types.h */

/* IfxGtm_Tom_ToutMap - from illd_types/Ifx_Types.h */

typedef IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;

void IfxGtm_Tom_Timer_init(void *driver, void *config);
void IfxGtm_Tom_Timer_run(void *driver);
void IfxGtm_Tom_Timer_stop(void *driver);

#endif /* IFXGTM_TOM_H */
