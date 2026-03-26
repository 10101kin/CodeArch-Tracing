#include "IfxGtm_Cmu.h"

/* Call counters */
static uint32 s_enableClocks_count = 0u;
static uint32 s_selectClkInput_count = 0u;

/* Last-argument captures */
static uint32 s_enableClocks_lastClkMask = 0u;
static uint32 s_selectClkInput_lastClkIndex = 0u;
static uint32 s_selectClkInput_lastUseGlobal = 0u;

void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask) {
    (void)gtm;
    s_enableClocks_count++;
    s_enableClocks_lastClkMask = clkMask;
}

void IfxGtm_Cmu_selectClkInput(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, boolean useGlobal) {
    (void)gtm;
    s_selectClkInput_count++;
    s_selectClkInput_lastClkIndex = (uint32)clkIndex;
    s_selectClkInput_lastUseGlobal = (uint32)useGlobal;
}

uint32 IfxGtm_Cmu_Mock_GetCallCount_enableClocks(void) { return s_enableClocks_count; }
uint32 IfxGtm_Cmu_Mock_GetCallCount_selectClkInput(void) { return s_selectClkInput_count; }
uint32 IfxGtm_Cmu_Mock_GetLastArg_enableClocks_clkMask(void) { return s_enableClocks_lastClkMask; }
uint32 IfxGtm_Cmu_Mock_GetLastArg_selectClkInput_clkIndex(void) { return s_selectClkInput_lastClkIndex; }
uint32 IfxGtm_Cmu_Mock_GetLastArg_selectClkInput_useGlobal(void) { return s_selectClkInput_lastUseGlobal; }

void IfxGtm_Cmu_Mock_Reset(void) {
    s_enableClocks_count = 0u;
    s_selectClkInput_count = 0u;
    s_enableClocks_lastClkMask = 0u;
    s_selectClkInput_lastClkIndex = 0u;
    s_selectClkInput_lastUseGlobal = 0u;
}

float32 IfxGtm_Cmu_getGclkFrequency(Ifx_GTM *gtm) {
    (void)gtm;
    return 100000000.0f;
}

float32 IfxGtm_Cmu_getClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, boolean assumeEnabled) {
    (void)gtm;
    (void)clkIndex;
    (void)assumeEnabled;
    return 100000000.0f;
}

float32 IfxGtm_Cmu_getFxclkFrequency(Ifx_GTM *gtm, uint8 fxclkIndex, boolean assumeEnabled) {
    (void)gtm;
    (void)fxclkIndex;
    (void)assumeEnabled;
    return 100000000.0f;
}

float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm) {
    (void)gtm;
    return 100000000.0f;
}

void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency) {
    (void)gtm;
    (void)frequency;
}

void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, float32 frequency) {
    (void)gtm;
    (void)clkIndex;
    (void)frequency;
}
