#include "IfxSrc.h"

static uint32 s_enableBroadcast_count = 0u;
static uint32 s_disableBroadcast_count = 0u;
static uint32 s_enableBroadcast_lastGroup = 0u;
static uint8  s_enableBroadcast_lastLine = 0u;
static uint32 s_disableBroadcast_lastGroup = 0u;
static uint8  s_disableBroadcast_lastLine = 0u;

void IfxSrc_enableBroadcastInterruptLine(volatile Ifx_INT *intr, IfxSrc_Int_group groupNum, uint8 interruptLine)
{
    (void)intr;
    s_enableBroadcast_count++;
    s_enableBroadcast_lastGroup = (uint32)groupNum;
    s_enableBroadcast_lastLine = interruptLine;
}

void IfxSrc_disableBroadcastInterruptLine(volatile Ifx_INT *intr, IfxSrc_Int_group groupNum, uint8 interruptLine)
{
    (void)intr;
    s_disableBroadcast_count++;
    s_disableBroadcast_lastGroup = (uint32)groupNum;
    s_disableBroadcast_lastLine = interruptLine;
}

uint32 IfxSrc_Mock_GetCallCount_enableBroadcastInterruptLine(void) { return s_enableBroadcast_count; }
uint32 IfxSrc_Mock_GetCallCount_disableBroadcastInterruptLine(void) { return s_disableBroadcast_count; }

uint32 IfxSrc_Mock_GetLastArg_enableBroadcastInterruptLine_group(void) { return s_enableBroadcast_lastGroup; }
uint8  IfxSrc_Mock_GetLastArg_enableBroadcastInterruptLine_line(void) { return s_enableBroadcast_lastLine; }
uint32 IfxSrc_Mock_GetLastArg_disableBroadcastInterruptLine_group(void) { return s_disableBroadcast_lastGroup; }
uint8  IfxSrc_Mock_GetLastArg_disableBroadcastInterruptLine_line(void) { return s_disableBroadcast_lastLine; }

void IfxSrc_Mock_Reset(void)
{
    s_enableBroadcast_count = 0u;
    s_disableBroadcast_count = 0u;
    s_enableBroadcast_lastGroup = 0u;
    s_enableBroadcast_lastLine = 0u;
    s_disableBroadcast_lastGroup = 0u;
    s_disableBroadcast_lastLine = 0u;
}
