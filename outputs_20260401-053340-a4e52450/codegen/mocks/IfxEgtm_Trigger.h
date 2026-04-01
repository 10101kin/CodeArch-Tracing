#ifndef IFXEGTM_TRIGGER_H
#define IFXEGTM_TRIGGER_H

#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#include "IfxEgtm.h"

/* Trigger sources/channels/signals (include names needed by tests) */
typedef enum {
    IfxEgtm_TrigSource_atom0 = 0,
    IfxEgtm_TrigSource_atom1 = 1,
    IfxEgtm_TrigSource_tom0  = 2,
    IfxEgtm_TrigSource_tom1  = 3
} IfxEgtm_TrigSource;

typedef enum {
    IfxEgtm_TrigChannel_0 = 0,
    IfxEgtm_TrigChannel_1 = 1,
    IfxEgtm_TrigChannel_2 = 2,
    IfxEgtm_TrigChannel_3 = 3,
    IfxEgtm_TrigChannel_4 = 4,
    IfxEgtm_TrigChannel_5 = 5,
    IfxEgtm_TrigChannel_6 = 6,
    IfxEgtm_TrigChannel_7 = 7
} IfxEgtm_TrigChannel;

typedef enum {
    IfxEgtm_Cfg_AdcTriggerSignal_0 = 0,
    IfxEgtm_Cfg_AdcTriggerSignal_1 = 1,
    IfxEgtm_Cfg_AdcTriggerSignal_2 = 2,
    IfxEgtm_Cfg_AdcTriggerSignal_3 = 3
} IfxEgtm_Cfg_AdcTriggerSignal;

/* API */
boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal);

#endif /* IFXEGTM_TRIGGER_H */
