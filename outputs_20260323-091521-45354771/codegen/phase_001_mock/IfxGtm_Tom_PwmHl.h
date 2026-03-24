#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "IfxGtm_Tom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for boolean */

/* Function declarations */
/* Mock control functions */

/* ============= Function Declarations ============= */
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
uint32  IfxGtm_Tom_PwmHl_Mock_GetCallCount_init(void);
void    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(boolean value);
void    IfxGtm_Tom_PwmHl_Mock_Reset(void);

#endif /* IFXGTM_TOM_PWMHL_H */