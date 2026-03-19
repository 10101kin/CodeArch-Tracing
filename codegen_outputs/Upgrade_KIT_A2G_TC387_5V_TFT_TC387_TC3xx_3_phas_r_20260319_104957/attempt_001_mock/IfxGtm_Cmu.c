#include "IfxGtm_Cmu.h"

static uint32  s_enableClocks_count = 0u;
static uint32  s_enableClocks_lastMask = 0u;

static uint32  s_getFxClk_count = 0u;
static uint32  s_getFxClk_lastIndex = 0u;
static uint32  s_getFxClk_lastAssume = 0u;
static float32 s_getFxClk_ret = 0.0f;

void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask) {
    (void)gtm;
    s_enableClocks_count++;
    s_enableClocks_lastMask = clkMask;
}

float32 IfxGtm_Cmu_getFxClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk clkIndex, boolean assumeEnabled) {
    (void)gtm;
    s_getFxClk_count++;
    s_getFxClk_lastIndex = (uint32)clkIndex;
    s_getFxClk_lastAssume = (uint32)assumeEnabled;
    return s_getFxClk_ret;
}

uint32 IfxGtm_Cmu_Mock_GetCallCount_enableClocks(void) { return s_enableClocks_count; }
uint32 IfxGtm_Cmu_Mock_GetLastArg_enableClocks_clkMask(void) { return s_enableClocks_lastMask; }

uint32 IfxGtm_Cmu_Mock_GetCallCount_getFxClkFrequency(void) { return s_getFxClk_count; }
uint32 IfxGtm_Cmu_Mock_GetLastArg_getFxClkFrequency_clkIndex(void) { return s_getFxClk_lastIndex; }
uint32 IfxGtm_Cmu_Mock_GetLastArg_getFxClkFrequency_assumeEnabled(void) { return s_getFxClk_lastAssume; }
void   IfxGtm_Cmu_Mock_SetReturn_getFxClkFrequency(float32 ret) { s_getFxClk_ret = ret; }

void IfxGtm_Cmu_Mock_Reset(void) {
    s_enableClocks_count = 0u; s_enableClocks_lastMask = 0u;
    s_getFxClk_count = 0u; s_getFxClk_lastIndex = 0u; s_getFxClk_lastAssume = 0u; s_getFxClk_ret = 0.0f;
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
