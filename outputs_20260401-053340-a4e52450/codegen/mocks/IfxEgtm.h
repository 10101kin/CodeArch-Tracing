#ifndef IFXEGTM_H
#define IFXEGTM_H
#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"

/* Minimal dependent enums/types required */
typedef enum { IfxEgtm_AeiBridgeOpMode_sync = 0, IfxEgtm_AeiBridgeOpMode_async = 1 } IfxEgtm_AeiBridgeOpMode;
typedef enum { IfxEgtm_ClusterClockDiv_disable = 0, IfxEgtm_ClusterClockDiv_enable = 1, IfxEgtm_ClusterClockDiv_enableDiv2 = 2 } IfxEgtm_ClusterClockDiv;
typedef enum { IfxEgtm_IrqMode_level = 0, IfxEgtm_IrqMode_pulse = 1, IfxEgtm_IrqMode_pulseNotify = 2, IfxEgtm_IrqMode_singlePulse = 3 } IfxEgtm_IrqMode;
typedef enum { IfxEgtm_MscAltInput_low = 0, IfxEgtm_MscAltInput_lowext = 1, IfxEgtm_MscAltInput_high = 2, IfxEgtm_MscAltInput_highext = 3 } IfxEgtm_MscAltInput;
typedef enum { IfxEgtm_SuspendMode_none = 0, IfxEgtm_SuspendMode_hard = 1, IfxEgtm_SuspendMode_soft = 2 } IfxEgtm_SuspendMode;

typedef enum { IfxEgtm_Cfg_MscSet_0 = 0 } IfxEgtm_Cfg_MscSet;
typedef enum { IfxEgtm_Cfg_MscSetSignal_0 = 0 } IfxEgtm_Cfg_MscSetSignal;
typedef enum { IfxEgtm_Cfg_MscModule_0 = 0 } IfxEgtm_Cfg_MscModule;
typedef enum { IfxEgtm_Cfg_MscSelect_0 = 0 } IfxEgtm_Cfg_MscSelect;

/* MscOut used by PWM ChannelConfig */
typedef struct {
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* Cluster enum used broadly */
typedef enum { IfxEgtm_Cluster_0 = 0, IfxEgtm_Cluster_1 = 1, IfxEgtm_Cluster_2 = 2 } IfxEgtm_Cluster;

/* Trigger-related enums used by IfxEgtm_Trigger */
typedef enum { IfxEgtm_TrigSource_atom = 0, IfxEgtm_TrigSource_tom = 1 } IfxEgtm_TrigSource;
typedef enum { IfxEgtm_TrigChannel_0 = 0 } IfxEgtm_TrigChannel;
typedef enum { IfxEgtm_Cfg_AdcTriggerSignal_0 = 0 } IfxEgtm_Cfg_AdcTriggerSignal;

/* Functions required */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
void    IfxEgtm_enable(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
