#include "IfxEgtm_Trigger.h"

static uint32 s_trigToAdc_count = 0;
static boolean s_trigToAdc_ret = 0; /* default FALSE */
static uint32 s_trigToAdc_lastCluster = 0;
static uint32 s_trigToAdc_lastSource = 0;
static uint32 s_trigToAdc_lastChannel = 0;
static uint32 s_trigToAdc_lastAdcTrigSignal = 0;

boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal)
{
    s_trigToAdc_count++;
    s_trigToAdc_lastCluster = (uint32)egtmCluster;
    s_trigToAdc_lastSource = (uint32)egtmSource;
    s_trigToAdc_lastChannel = (uint32)Channel;
    s_trigToAdc_lastAdcTrigSignal = (uint32)adcTrigSignal;
    return s_trigToAdc_ret;
}

uint32 IfxEgtm_Trigger_Mock_GetCallCount_trigToAdc(void)
{
    return s_trigToAdc_count;
}

void IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(boolean value)
{
    s_trigToAdc_ret = value;
}

uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_egtmCluster(void) { return s_trigToAdc_lastCluster; }
uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_egtmSource(void)  { return s_trigToAdc_lastSource; }
uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_Channel(void)     { return s_trigToAdc_lastChannel; }
uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_adcTrigSignal(void) { return s_trigToAdc_lastAdcTrigSignal; }

void IfxEgtm_Trigger_Mock_Reset(void)
{
    s_trigToAdc_count = 0;
    s_trigToAdc_ret = 0;
    s_trigToAdc_lastCluster = 0;
    s_trigToAdc_lastSource = 0;
    s_trigToAdc_lastChannel = 0;
    s_trigToAdc_lastAdcTrigSignal = 0;
}
