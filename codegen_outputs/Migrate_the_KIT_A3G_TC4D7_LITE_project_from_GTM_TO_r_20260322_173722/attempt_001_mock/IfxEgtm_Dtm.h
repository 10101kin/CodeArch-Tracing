#ifndef IFXEGTM_DTM_H
#define IFXEGTM_DTM_H

#include "illd_types/Ifx_Types.h"

/* ============= Type Definitions ============= */
typedef struct Ifx_EGTM_CLS_CDTM_DTM Ifx_EGTM_CLS_CDTM_DTM;
typedef enum {
    IfxEgtm_Dtm_Ch_0 = 0,
    IfxEgtm_Dtm_Ch_1 = 1,
    IfxEgtm_Dtm_Ch_2 = 2,
    IfxEgtm_Dtm_Ch_3 = 3
} IfxEgtm_Dtm_Ch;
typedef enum {
    IfxEgtm_Dtm_ClockSource_cmuClk0 = 0,
    IfxEgtm_Dtm_ClockSource_cmuClk1 = 1
} IfxEgtm_Dtm_ClockSource;

/* Forward declaration for DTM SFR block */
/* Minimal enums for DTM channel and clock source */
/* API */
/* Mock controls */

/* ============= Function Declarations ============= */
void IfxEgtm_Dtm_setRelrise(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relrise);
void IfxEgtm_Dtm_setClockSource(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_ClockSource clockSource);
void IfxEgtm_Dtm_setRelfall(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relfall);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelrise(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_channel(void);
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setClockSource(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setClockSource_clockSource(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelfall(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_channel(void);
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall(void);
void   IfxEgtm_Dtm_Mock_Reset(void);

#endif