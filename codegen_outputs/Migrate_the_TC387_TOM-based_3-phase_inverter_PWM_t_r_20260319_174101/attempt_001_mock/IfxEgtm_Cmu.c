#include "IfxEgtm_Cmu.h"

static uint32 s_enableClocks_count = 0u;
static uint32 s_selectClkInput_count = 0u;
static uint32 s_setGclkDivider_count = 0u;

static uint32 s_enableClocks_lastClkMask = 0u;
static uint32 s_selectClkInput_lastClkIndex = 0u;
static uint32 s_selectClkInput_lastUseGlobal = 0u;
static uint32 s_setGclkDivider_lastNumerator = 0u;
static uint32 s_setGclkDivider_lastDenominator = 0u;

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm;
    s_enableClocks_count++;
    s_enableClocks_lastClkMask = clkMask;
}

void IfxEgtm_Cmu_selectClkInput(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean useGlobal)
{
    (void)egtm;
    s_selectClkInput_count++;
    s_selectClkInput_lastClkIndex = (uint32)clkIndex;
    s_selectClkInput_lastUseGlobal = (uint32)useGlobal;
}

void IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *egtm, uint32 numerator, uint32 denominator)
{
    (void)egtm;
    s_setGclkDivider_count++;
    s_setGclkDivider_lastNumerator = numerator;
    s_setGclkDivider_lastDenominator = denominator;
}

uint32 IfxEgtm_Cmu_Mock_GetCallCount_enableClocks(void) { return s_enableClocks_count; }
uint32 IfxEgtm_Cmu_Mock_GetCallCount_selectClkInput(void) { return s_selectClkInput_count; }
uint32 IfxEgtm_Cmu_Mock_GetCallCount_setGclkDivider(void) { return s_setGclkDivider_count; }

uint32 IfxEgtm_Cmu_Mock_GetLastArg_enableClocks_clkMask(void) { return s_enableClocks_lastClkMask; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_selectClkInput_clkIndex(void) { return s_selectClkInput_lastClkIndex; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_selectClkInput_useGlobal(void) { return s_selectClkInput_lastUseGlobal; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_setGclkDivider_numerator(void) { return s_setGclkDivider_lastNumerator; }
uint32 IfxEgtm_Cmu_Mock_GetLastArg_setGclkDivider_denominator(void) { return s_setGclkDivider_lastDenominator; }

void IfxEgtm_Cmu_Mock_Reset(void)
{
    s_enableClocks_count = 0u;
    s_selectClkInput_count = 0u;
    s_setGclkDivider_count = 0u;
    s_enableClocks_lastClkMask = 0u;
    s_selectClkInput_lastClkIndex = 0u;
    s_selectClkInput_lastUseGlobal = 0u;
    s_setGclkDivider_lastNumerator = 0u;
    s_setGclkDivider_lastDenominator = 0u;
}
