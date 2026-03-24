#include "IfxEgtm_Cmu.h"

static uint32 s_enableClocks_count = 0u;
static uint32 s_enableClocks_lastClkMask = 0u;
static uint32 s_isFxClockEnabled_count = 0u;
static boolean s_isFxClockEnabled_ret = 0u;

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm;
    s_enableClocks_count++;
    s_enableClocks_lastClkMask = clkMask;
}

boolean IfxEgtm_Cmu_isFxClockEnabled(Ifx_EGTM *egtm)
{
    (void)egtm;
    s_isFxClockEnabled_count++;
    return s_isFxClockEnabled_ret;
}

uint32 IfxEgtm_Cmu_Mock_GetCallCount_enableClocks(void) { return s_enableClocks_count; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_enableClocks_clkMask(void) { return s_enableClocks_lastClkMask; }
uint32 IfxEgtm_Cmu_Mock_GetCallCount_isFxClockEnabled(void) { return s_isFxClockEnabled_count; }
void   IfxEgtm_Cmu_Mock_SetReturn_isFxClockEnabled(boolean value) { s_isFxClockEnabled_ret = value; }

void IfxEgtm_Cmu_Mock_Reset(void)
{
    s_enableClocks_count = 0u;
    s_enableClocks_lastClkMask = 0u;
    s_isFxClockEnabled_count = 0u;
    s_isFxClockEnabled_ret = 0u;
}
