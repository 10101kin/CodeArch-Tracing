#ifndef IFXEGTM_DTM_H
#define IFXEGTM_DTM_H

#include "illd_types/Ifx_Types.h"

/* Mock controls */
/* Captures */

/* ============= Function Declarations ============= */
void IfxEgtm_Dtm_setRelrise(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relrise);
void IfxEgtm_Dtm_setOutput0DeadTimePath(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_DeadTimePath deadTimePath);
void IfxEgtm_Dtm_setRelfall(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relfall);
void IfxEgtm_Dtm_setOutput1DeadTimePath(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_DeadTimePath deadTimePath);
void IfxEgtm_Dtm_setOutput1Polarity(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_OutputPolarity polarity);
void IfxEgtm_Dtm_setOutput1Select(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_Output1Select output1Select);
void IfxEgtm_Dtm_setOutput1Function(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_Output1Function output1Function);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelrise(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput0DeadTimePath(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelfall(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1DeadTimePath(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Polarity(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Select(void);
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Function(void);
void   IfxEgtm_Dtm_Mock_Reset(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_channel(void);
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput0DeadTimePath_channel(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput0DeadTimePath_deadTimePath(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_channel(void);
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1DeadTimePath_channel(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1DeadTimePath_deadTimePath(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Polarity_channel(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Polarity_polarity(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Select_channel(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Select_output1Select(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Function_channel(void);
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Function_output1Function(void);

#endif