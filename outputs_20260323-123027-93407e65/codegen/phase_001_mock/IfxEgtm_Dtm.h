#ifndef IFXEGTM_DTM_H
#define IFXEGTM_DTM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for base types */

/* Mock controls */
/* Value capture getters */

/* ============= Function Declarations ============= */
void IfxEgtm_Dtm_setRelfall(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relfall);
void IfxEgtm_Dtm_setClockSource(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_ClockSource clockSource);
void IfxEgtm_Dtm_setRelrise(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relrise);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelfall(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setClockSource(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelrise(void);
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setClockSource_clockSource(void);
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise(void);
void   IfxEgtm_Dtm_Mock_Reset(void);

#endif /* IFXEGTM_DTM_H */