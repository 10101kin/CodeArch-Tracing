#ifndef IFXEGTM_TRIGGER_H
#define IFXEGTM_TRIGGER_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm_Pwm.h" /* For IfxEgtm_Cluster */

/* Enums to support trigger API and previously referenced constants */
typedef enum {
    IfxEgtm_TrigSource_atom0 = 0,
    IfxEgtm_TrigSource_atom1 = 1,
    IfxEgtm_TrigSource_atom2 = 2,
    IfxEgtm_TrigSource_atom3 = 3,
    IfxEgtm_TrigSource_atom4 = 4,
    IfxEgtm_TrigSource_atom5 = 5,
    IfxEgtm_TrigSource_atom6 = 6,
    IfxEgtm_TrigSource_atom7 = 7
} IfxEgtm_TrigSource;

typedef enum {
    IfxEgtm_TrigChannel_0  = 0,
    IfxEgtm_TrigChannel_1  = 1,
    IfxEgtm_TrigChannel_2  = 2,
    IfxEgtm_TrigChannel_3  = 3,
    IfxEgtm_TrigChannel_4  = 4,
    IfxEgtm_TrigChannel_5  = 5,
    IfxEgtm_TrigChannel_6  = 6,
    IfxEgtm_TrigChannel_7  = 7,
    IfxEgtm_TrigChannel_8  = 8,
    IfxEgtm_TrigChannel_9  = 9,
    IfxEgtm_TrigChannel_10 = 10,
    IfxEgtm_TrigChannel_11 = 11,
    IfxEgtm_TrigChannel_12 = 12,
    IfxEgtm_TrigChannel_13 = 13,
    IfxEgtm_TrigChannel_14 = 14,
    IfxEgtm_TrigChannel_15 = 15
} IfxEgtm_TrigChannel;

typedef enum {
    IfxEgtm_Cfg_AdcTriggerSignal_0 = 0,
    IfxEgtm_Cfg_AdcTriggerSignal_1 = 1,
    IfxEgtm_Cfg_AdcTriggerSignal_2 = 2,
    IfxEgtm_Cfg_AdcTriggerSignal_3 = 3
} IfxEgtm_Cfg_AdcTriggerSignal;

/* Function declarations */
boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal);

#endif /* IFXEGTM_TRIGGER_H */
