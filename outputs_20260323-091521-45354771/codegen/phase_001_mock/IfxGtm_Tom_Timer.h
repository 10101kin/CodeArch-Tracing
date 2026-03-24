#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "IfxGtm_Tom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for boolean, uint32 */

/* Function declarations */
/* Mock control functions */

/* ============= Function Declarations ============= */
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_init(void);
void    IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value);
void    IfxGtm_Tom_Timer_Mock_Reset(void);

#endif /* IFXGTM_TOM_TIMER_H */