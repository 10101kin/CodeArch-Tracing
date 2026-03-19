#ifndef IFXSRC_H
#define IFXSRC_H

#include "illd_types/Ifx_Types.h"

/* Mock controls */

/* ============= Function Declarations ============= */
void IfxSrc_enableBroadcastInterruptLine(volatile Ifx_INT *intr, IfxSrc_Int_group groupNum, uint8 interruptLine);
void IfxSrc_disableBroadcastInterruptLine(volatile Ifx_INT *intr, IfxSrc_Int_group groupNum, uint8 interruptLine);
uint32 IfxSrc_Mock_GetCallCount_enableBroadcastInterruptLine(void);
uint32 IfxSrc_Mock_GetCallCount_disableBroadcastInterruptLine(void);
void   IfxSrc_Mock_Reset(void);
uint32 IfxSrc_Mock_GetLastArg_enableBroadcastInterruptLine_group(void);
uint8  IfxSrc_Mock_GetLastArg_enableBroadcastInterruptLine_line(void);
uint32 IfxSrc_Mock_GetLastArg_disableBroadcastInterruptLine_group(void);
uint8  IfxSrc_Mock_GetLastArg_disableBroadcastInterruptLine_line(void);

#endif