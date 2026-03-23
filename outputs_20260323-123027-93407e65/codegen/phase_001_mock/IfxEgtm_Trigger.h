#ifndef IFXEGTM_TRIGGER_H
#define IFXEGTM_TRIGGER_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for base types */

/* Mock controls */
/* Value capture getters */

/* ============= Function Declarations ============= */
boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal);
uint32  IfxEgtm_Trigger_Mock_GetCallCount_trigToAdc(void);
void    IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(boolean value);
uint32  IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_egtmCluster(void);
uint32  IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_egtmSource(void);
uint32  IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_Channel(void);
uint32  IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_adcTrigSignal(void);
void    IfxEgtm_Trigger_Mock_Reset(void);

#endif /* IFXEGTM_TRIGGER_H */