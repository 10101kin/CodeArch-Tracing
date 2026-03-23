#include "IfxAdc_Tmadc.h"

/* Call counters */
static uint32 s_initChannel_count      = 0;
static uint32 s_runModule_count        = 0;
static uint32 s_initModuleConfig_count = 0;
static uint32 s_initChannelConfig_count= 0;
static uint32 s_initModule_count       = 0;

void IfxAdc_Tmadc_initChannel(IfxAdc_Tmadc_Ch *channel, IfxAdc_Tmadc_ChConfig *config)
{
    (void)channel;
    (void)config;
    s_initChannel_count++;
}

void IfxAdc_Tmadc_runModule(IfxAdc_Tmadc *tmadc)
{
    (void)tmadc;
    s_runModule_count++;
}

void IfxAdc_Tmadc_initModuleConfig(IfxAdc_Tmadc_Config *config, Ifx_ADC *adc)
{
    (void)config;
    (void)adc;
    s_initModuleConfig_count++;
}

void IfxAdc_Tmadc_initChannelConfig(IfxAdc_Tmadc_ChConfig *config, Ifx_ADC *adc)
{
    (void)config;
    (void)adc;
    s_initChannelConfig_count++;
}

void IfxAdc_Tmadc_initModule(IfxAdc_Tmadc *tmadc, const IfxAdc_Tmadc_Config *config)
{
    (void)tmadc;
    (void)config;
    s_initModule_count++;
}

/* Call count getters */
uint32 IfxAdc_Tmadc_Mock_GetCallCount_initChannel(void)      { return s_initChannel_count; }
uint32 IfxAdc_Tmadc_Mock_GetCallCount_runModule(void)        { return s_runModule_count; }
uint32 IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig(void) { return s_initModuleConfig_count; }
uint32 IfxAdc_Tmadc_Mock_GetCallCount_initChannelConfig(void){ return s_initChannelConfig_count; }
uint32 IfxAdc_Tmadc_Mock_GetCallCount_initModule(void)       { return s_initModule_count; }

void IfxAdc_Tmadc_Mock_Reset(void)
{
    s_initChannel_count = 0;
    s_runModule_count = 0;
    s_initModuleConfig_count = 0;
    s_initChannelConfig_count = 0;
    s_initModule_count = 0;
}
