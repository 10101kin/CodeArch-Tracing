#include "IfxEgtm_Cmu.h"

static uint32 s_selectClkInput_count = 0u;
static uint32 s_setClkCount_count = 0u;
static uint32 s_enableClocks_count = 0u;

static uint32 s_selectClkInput_lastClkIndex = 0u;
static uint32 s_selectClkInput_lastUseGlobal = 0u;
static uint32 s_setClkCount_lastClkIndex = 0u;
static uint32 s_setClkCount_lastCount = 0u;
static uint32 s_enableClocks_lastClkMask = 0u;

void IfxEgtm_Cmu_selectClkInput(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean useGlobal)
{
    (void)egtm;
    s_selectClkInput_count++;
    s_selectClkInput_lastClkIndex = (uint32)clkIndex;
    s_selectClkInput_lastUseGlobal = (uint32)useGlobal;
}

void IfxEgtm_Cmu_setClkCount(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, uint32 count)
{
    (void)egtm;
    s_setClkCount_count++;
    s_setClkCount_lastClkIndex = (uint32)clkIndex;
    s_setClkCount_lastCount = count;
}

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm;
    s_enableClocks_count++;
    s_enableClocks_lastClkMask = clkMask;
}

uint32 IfxEgtm_Cmu_Mock_GetCallCount_selectClkInput(void) { return s_selectClkInput_count; }
uint32 IfxEgtm_Cmu_Mock_GetCallCount_setClkCount(void) { return s_setClkCount_count; }
uint32 IfxEgtm_Cmu_Mock_GetCallCount_enableClocks(void) { return s_enableClocks_count; }

uint32 IfxEgtm_Cmu_Mock_GetLastArg_selectClkInput_clkIndex(void) { return s_selectClkInput_lastClkIndex; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_selectClkInput_useGlobal(void) { return s_selectClkInput_lastUseGlobal; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_setClkCount_clkIndex(void) { return s_setClkCount_lastClkIndex; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_setClkCount_count(void) { return s_setClkCount_lastCount; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_enableClocks_clkMask(void) { return s_enableClocks_lastClkMask; }

void IfxEgtm_Cmu_Mock_Reset(void)
{
    s_selectClkInput_count = 0u;
    s_setClkCount_count = 0u;
    s_enableClocks_count = 0u;
    s_selectClkInput_lastClkIndex = 0u;
    s_selectClkInput_lastUseGlobal = 0u;
    s_setClkCount_lastClkIndex = 0u;
    s_setClkCount_lastCount = 0u;
    s_enableClocks_lastClkMask = 0u;
}
