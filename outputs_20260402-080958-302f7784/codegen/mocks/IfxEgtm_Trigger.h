#ifndef IFXEGTM_TRIGGER_H
#define IFXEGTM_TRIGGER_H

#include "mock_egtmatompwm.h"
#include "IfxEgtm.h"

/* Minimal trigger-related enums to satisfy signature */
typedef enum { IfxEgtm_TrigSource_0 = 0, IfxEgtm_TrigSource_1 = 1 } IfxEgtm_TrigSource;
typedef enum { IfxEgtm_TrigChannel_0 = 0, IfxEgtm_TrigChannel_1 = 1 } IfxEgtm_TrigChannel;
typedef enum { IfxEgtm_Cfg_AdcTriggerSignal_0 = 0, IfxEgtm_Cfg_AdcTriggerSignal_1 = 1 } IfxEgtm_Cfg_AdcTriggerSignal;

boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal);

#endif /* IFXEGTM_TRIGGER_H */
