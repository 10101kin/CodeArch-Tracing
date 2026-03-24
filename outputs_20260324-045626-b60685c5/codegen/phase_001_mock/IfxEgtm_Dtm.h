#ifndef IFXEGTM_DTM_H
#define IFXEGTM_DTM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD function declarations */
/* Mock control: call counts */
/* Mock control: last-arg capture */
/* Mock control: reset */

/* ============= Function Declarations ============= */
void IfxEgtm_Dtm_setClockSource(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_ClockSource clockSource);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setClockSource(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setClockSource(void);
void IfxEgtm_Dtm_Mock_Reset(void);

#endif