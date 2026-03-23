#include "IfxEgtm_Cmu.h"

/* Call counters */
static uint32 s_enableClocks_count = 0;
static uint32 s_selectClkInput_count = 0;

/* Argument captures */
static uint32 s_enableClocks_lastClkMask = 0u;
static uint32 s_selectClkInput_lastClkIndex = 0u;
static boolean s_selectClkInput_lastUseGlobal = 0;

/* iLLD stubs */
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask) {
    (void)egtm; (void)clkMask;
    s_enableClocks_count++;
    s_enableClocks_lastClkMask = clkMask;
}

void IfxEgtm_Cmu_selectClkInput(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean useGlobal) {
    (void)egtm; (void)clkIndex; (void)useGlobal;
    s_selectClkInput_count++;
    s_selectClkInput_lastClkIndex = (uint32)clkIndex;
    s_selectClkInput_lastUseGlobal = useGlobal;
}

/* Mock controls */
uint32 IfxEgtm_Cmu_Mock_GetCallCount_enableClocks(void) { return s_enableClocks_count; }
uint32 IfxEgtm_Cmu_Mock_GetCallCount_selectClkInput(void) { return s_selectClkInput_count; }

uint32 IfxEgtm_Cmu_Mock_GetLastArg_enableClocks_clkMask(void) { return s_enableClocks_lastClkMask; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_selectClkInput_clkIndex(void) { return s_selectClkInput_lastClkIndex; }
boolean IfxEgtm_Cmu_Mock_GetLastArg_selectClkInput_useGlobal(void) { return s_selectClkInput_lastUseGlobal; }

void IfxEgtm_Cmu_Mock_Reset(void) {
    s_enableClocks_count = 0u;
    s_selectClkInput_count = 0u;
    s_enableClocks_lastClkMask = 0u;
    s_selectClkInput_lastClkIndex = 0u;
    s_selectClkInput_lastUseGlobal = 0;
}
