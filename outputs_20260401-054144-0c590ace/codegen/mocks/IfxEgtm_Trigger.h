/* IfxEgtm_Trigger mock */
#ifndef IFXEGTM_TRIGGER_H
#define IFXEGTM_TRIGGER_H
#include "mock_egtm_atom_tmadc_consolidated.h"
#include "IfxEgtm.h"

boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal);

#endif /* IFXEGTM_TRIGGER_H */
