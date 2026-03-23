#include "IfxEgtm_Trigger.h"

/* Call counters */
static uint32  s_trigToAdc_count = 0;

/* Return value */
static boolean s_trigToAdc_ret   = 0u;

/* Argument capture */
static uint32 s_trig_cluster      = 0u;
static uint32 s_trig_source       = 0u;
static uint32 s_trig_channel      = 0u;
static uint32 s_trig_adcTrigSignal= 0u;

boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal)
{
    s_trigToAdc_count++;
    s_trig_cluster       = (uint32)egtmCluster;
    s_trig_source        = (uint32)egtmSource;
    s_trig_channel       = (uint32)Channel;
    s_trig_adcTrigSignal = (uint32)adcTrigSignal;
    return s_trigToAdc_ret;
}

uint32  IfxEgtm_Trigger_Mock_GetCallCount_trigToAdc(void) { return s_trigToAdc_count; }
void    IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(boolean value) { s_trigToAdc_ret = value; }
boolean IfxEgtm_Trigger_Mock_GetReturn_trigToAdc(void) { return s_trigToAdc_ret; }

uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_cluster(void)       { return s_trig_cluster; }
uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_source(void)        { return s_trig_source; }
uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_channel(void)       { return s_trig_channel; }
uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_adcTrigSignal(void) { return s_trig_adcTrigSignal; }

void IfxEgtm_Trigger_Mock_Reset(void)
{
    s_trigToAdc_count = 0u;
    s_trigToAdc_ret   = 0u;
    s_trig_cluster = s_trig_source = s_trig_channel = s_trig_adcTrigSignal = 0u;
}
