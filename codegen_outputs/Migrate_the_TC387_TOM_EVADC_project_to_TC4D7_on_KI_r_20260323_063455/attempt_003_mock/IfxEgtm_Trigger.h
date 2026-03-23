#ifndef IFXEGTM_TRIGGER_H
#define IFXEGTM_TRIGGER_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* API declarations */
/* Mock control - call counts */
/* Capture getters for value params */
/* Return-value control */
/* Reset */

/* ============= Function Declarations ============= */
boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal);
uint32  IfxEgtm_Trigger_Mock_GetCallCount_trigToAdc(void);
uint32  IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_cluster(void);
uint32  IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_source(void);
uint32  IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_channel(void);
uint32  IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_signal(void);
void    IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(boolean value);
void    IfxEgtm_Trigger_Mock_Reset(void);

#endif /* IFXEGTM_TRIGGER_H */