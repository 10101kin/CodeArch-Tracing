#ifndef IFXEGTM_DTM_H
#define IFXEGTM_DTM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint16, Ifx_EGTM_CLS_CDTM_DTM */

/* ============= Type Definitions ============= */
typedef enum {
    IfxEgtm_Dtm_Ch_0 = 0,
    IfxEgtm_Dtm_Ch_1 = 1
} IfxEgtm_Dtm_Ch;
typedef enum {
    IfxEgtm_Dtm_ClockSource_cmuClk0 = 0,
    IfxEgtm_Dtm_ClockSource_cmuClk1 = 1
} IfxEgtm_Dtm_ClockSource;

/* Minimal DTM channel and clock source enums */
/* Mock control */

/* ============= Function Declarations ============= */
void IfxEgtm_Dtm_setRelrise(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relrise);
void IfxEgtm_Dtm_setClockSource(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_ClockSource clockSource);
void IfxEgtm_Dtm_setRelfall(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relfall);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelrise(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setClockSource(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelfall(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_channel(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setClockSource_clockSource(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_channel(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall(void);
void IfxEgtm_Dtm_Mock_Reset(void);

#endif