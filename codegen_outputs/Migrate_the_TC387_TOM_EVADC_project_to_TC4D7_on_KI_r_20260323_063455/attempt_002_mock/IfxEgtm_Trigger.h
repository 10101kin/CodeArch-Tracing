#ifndef IFXEGTM_TRIGGER_H
#define IFXEGTM_TRIGGER_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD API declaration */
/* Mock control: call counts */
/* Mock control: return setter/getter */
/* Mock control: argument capture getters (as uint32 cast) */
/* Mock control: reset */

/* ============= Function Declarations ============= */
boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal);
uint32  IfxEgtm_Trigger_Mock_GetCallCount_trigToAdc(void);
void    IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(boolean value);
boolean IfxEgtm_Trigger_Mock_GetReturn_trigToAdc(void);
uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_cluster(void);
uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_source(void);
uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_channel(void);
uint32 IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_adcTrigSignal(void);
void IfxEgtm_Trigger_Mock_Reset(void);

#endif /* IFXEGTM_TRIGGER_H */