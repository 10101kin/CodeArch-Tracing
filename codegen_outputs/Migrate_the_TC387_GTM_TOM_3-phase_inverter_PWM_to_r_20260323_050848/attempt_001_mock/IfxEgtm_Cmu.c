#include "IfxEgtm_Cmu.h"

static uint32 s_enableClocks_count = 0;
static uint32 s_selectClkInput_count = 0;

static uint32 s_last_enableClocks_clkMask = 0;
static uint32 s_last_selectClkInput_clkIndex = 0;
static uint32 s_last_selectClkInput_useGlobal = 0;

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask) {
    (void)egtm;
    s_enableClocks_count++;
    s_last_enableClocks_clkMask = clkMask;
}

void IfxEgtm_Cmu_selectClkInput(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean useGlobal) {
    (void)egtm;
    s_selectClkInput_count++;
    s_last_selectClkInput_clkIndex = (uint32)clkIndex;
    s_last_selectClkInput_useGlobal = (uint32)useGlobal;
}

uint32 IfxEgtm_Cmu_Mock_GetCallCount_enableClocks(void) { return s_enableClocks_count; }
uint32 IfxEgtm_Cmu_Mock_GetCallCount_selectClkInput(void) { return s_selectClkInput_count; }

uint32 IfxEgtm_Cmu_Mock_GetLastArg_enableClocks_clkMask(void) { return s_last_enableClocks_clkMask; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_selectClkInput_clkIndex(void) { return s_last_selectClkInput_clkIndex; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_selectClkInput_useGlobal(void) { return s_last_selectClkInput_useGlobal; }

void IfxEgtm_Cmu_Mock_Reset(void) {
    s_enableClocks_count = 0;
    s_selectClkInput_count = 0;
    s_last_enableClocks_clkMask = 0;
    s_last_selectClkInput_clkIndex = 0;
    s_last_selectClkInput_useGlobal = 0;
}
