#ifndef IFXVMT_H
#define IFXVMT_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* Required function declarations */
/* Mock control functions */

/* ============= Function Declarations ============= */
void IfxCcu6_enableModule(void);
uint32 IfxVmt_Mock_GetCallCount_IfxCcu6_enableModule(void);
void   IfxVmt_Mock_Reset(void);

#endif /* IFXVMT_H */