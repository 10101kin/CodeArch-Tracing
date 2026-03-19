#include "IfxGtm_Cmu.h"

static uint32 s_enableClocks_count = 0;
static uint32 s_enableClocks_lastClkMask = 0u;

static uint32 s_getFxClkFrequency_count = 0;
static float32 s_getFxClkFrequency_ret = 0.0f;
static uint32 s_getFxClkFrequency_lastClkIndex = 0u;
static uint32 s_getFxClkFrequency_lastAssumeEnabled = 0u;

/* For compatibility with tests referencing this counter */
static uint32 s_setClkFrequency_count = 0; /* no API, but keep variable present */

void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask) {
    (void)gtm;
    s_enableClocks_count++;
    s_enableClocks_lastClkMask = clkMask;
}

float32 IfxGtm_Cmu_getFxClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk clkIndex, boolean assumeEnabled) {
    (void)gtm;
    s_getFxClkFrequency_count++;
    s_getFxClkFrequency_lastClkIndex = (uint32)clkIndex;
    s_getFxClkFrequency_lastAssumeEnabled = (uint32)assumeEnabled;
    return s_getFxClkFrequency_ret;
}

uint32 IfxGtm_Cmu_Mock_GetCallCount_enableClocks(void) { return s_enableClocks_count; }
uint32 IfxGtm_Cmu_Mock_GetLastArg_enableClocks_clkMask(void) { return s_enableClocks_lastClkMask; }

uint32 IfxGtm_Cmu_Mock_GetCallCount_getFxClkFrequency(void) { return s_getFxClkFrequency_count; }
void   IfxGtm_Cmu_Mock_SetReturn_getFxClkFrequency(float32 value) { s_getFxClkFrequency_ret = value; }
uint32 IfxGtm_Cmu_Mock_GetLastArg_getFxClkFrequency_clkIndex(void) { return s_getFxClkFrequency_lastClkIndex; }
uint32 IfxGtm_Cmu_Mock_GetLastArg_getFxClkFrequency_assumeEnabled(void) { return s_getFxClkFrequency_lastAssumeEnabled; }

uint32 IfxGtm_Cmu_Mock_GetCallCount_setClkFrequency(void) { return s_setClkFrequency_count; }

void IfxGtm_Cmu_Mock_Reset(void) {
    s_enableClocks_count = 0; s_enableClocks_lastClkMask = 0u;
    s_getFxClkFrequency_count = 0; s_getFxClkFrequency_ret = 0.0f;
    s_getFxClkFrequency_lastClkIndex = 0u; s_getFxClkFrequency_lastAssumeEnabled = 0u;
    s_setClkFrequency_count = 0; /* keep reset for compatibility */
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
