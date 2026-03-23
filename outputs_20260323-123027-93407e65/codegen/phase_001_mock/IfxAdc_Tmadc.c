#include "IfxAdc_Tmadc.h"

static uint32 s_initModule_count = 0u;
static uint32 s_runModule_count = 0u;
static uint32 s_initModuleConfig_count = 0u;

static uint32 s_initModule_lastSampleRate = 0u;
static uint32 s_initModule_lastNumChannels = 0u;
static uint32 s_initModuleConfig_lastSampleRate = 0u;
static uint32 s_initModuleConfig_lastNumChannels = 0u;

void IfxAdc_Tmadc_initModule(IfxAdc_Tmadc *tmadc, const IfxAdc_Tmadc_Config *config)
{
    (void)tmadc;
    s_initModule_count++;
    if (config != NULL_PTR) {
        s_initModule_lastSampleRate = config->sampleRate;
        s_initModule_lastNumChannels = config->numChannels;
    }
}

void IfxAdc_Tmadc_runModule(IfxAdc_Tmadc *tmadc)
{
    (void)tmadc;
    s_runModule_count++;
}

void IfxAdc_Tmadc_initModuleConfig(IfxAdc_Tmadc_Config *config, Ifx_ADC *adc)
{
    (void)adc;
    s_initModuleConfig_count++;
    if (config != NULL_PTR) {
        s_initModuleConfig_lastSampleRate = config->sampleRate;
        s_initModuleConfig_lastNumChannels = config->numChannels;
    }
}

uint32 IfxAdc_Tmadc_Mock_GetCallCount_initModule(void)      { return s_initModule_count; }
uint32 IfxAdc_Tmadc_Mock_GetCallCount_runModule(void)       { return s_runModule_count; }
uint32 IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig(void){ return s_initModuleConfig_count; }

uint32 IfxAdc_Tmadc_Mock_GetLastArg_initModule_sampleRate(void)        { return s_initModule_lastSampleRate; }
uint32 IfxAdc_Tmadc_Mock_GetLastArg_initModule_numChannels(void)       { return s_initModule_lastNumChannels; }
uint32 IfxAdc_Tmadc_Mock_GetLastArg_initModuleConfig_sampleRate(void)  { return s_initModuleConfig_lastSampleRate; }
uint32 IfxAdc_Tmadc_Mock_GetLastArg_initModuleConfig_numChannels(void) { return s_initModuleConfig_lastNumChannels; }

void IfxAdc_Tmadc_Mock_Reset(void)
{
    s_initModule_count = 0u;
    s_runModule_count = 0u;
    s_initModuleConfig_count = 0u;

    s_initModule_lastSampleRate = 0u;
    s_initModule_lastNumChannels = 0u;
    s_initModuleConfig_lastSampleRate = 0u;
    s_initModuleConfig_lastNumChannels = 0u;
}
