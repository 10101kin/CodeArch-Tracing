#ifndef IFXEGTM_TRIGGER_H
#define IFXEGTM_TRIGGER_H
#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#include "IfxEgtm.h"

boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal);

#endif /* IFXEGTM_TRIGGER_H */
