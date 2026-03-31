#ifndef IFXEGTM_TRIGGER_H
#define IFXEGTM_TRIGGER_H
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm_Pwm.h"

/* Placeholder trigger enums used in signature */
typedef enum { IfxEgtm_TrigSource_0 = 0 } IfxEgtm_TrigSource;
typedef enum { IfxEgtm_TrigChannel_0 = 0 } IfxEgtm_TrigChannel;
typedef enum { IfxEgtm_Cfg_AdcTriggerSignal_0 = 0 } IfxEgtm_Cfg_AdcTriggerSignal;

/* Function to mock */
boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal);

#endif /* IFXEGTM_TRIGGER_H */
