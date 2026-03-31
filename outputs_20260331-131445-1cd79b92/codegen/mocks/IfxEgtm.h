#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Shared EGTM SFR placeholders used by drivers */
typedef struct { uint32 dummy; } Ifx_EGTM_CLS;
typedef struct { uint32 dummy; } Ifx_EGTM_CLS_ATOM;
typedef struct { uint32 dummy; } Ifx_EGTM_CLS_ATOM_AGC;
typedef struct { uint32 dummy; } Ifx_EGTM_CLS_CDTM_DTM;

/* Cluster enum (shared) */
typedef enum
{
    IfxEgtm_Cluster_0 = 0,
    IfxEgtm_Cluster_1 = 1,
    IfxEgtm_Cluster_2 = 2
} IfxEgtm_Cluster;

/* Irq / suspend / misc enums */
typedef enum { IfxEgtm_AeiBridgeOpMode_sync = 0u, IfxEgtm_AeiBridgeOpMode_async = 1u } IfxEgtm_AeiBridgeOpMode;
typedef enum { IfxEgtm_ClusterClockDiv_disable = 0u, IfxEgtm_ClusterClockDiv_enable = 1u, IfxEgtm_ClusterClockDiv_enableDiv2 = 2u } IfxEgtm_ClusterClockDiv;
typedef enum { IfxEgtm_IrqMode_level = 0, IfxEgtm_IrqMode_pulse = 1, IfxEgtm_IrqMode_pulseNotify = 2, IfxEgtm_IrqMode_singlePulse = 3 } IfxEgtm_IrqMode;
typedef enum { IfxEgtm_MscAltInput_low = 0, IfxEgtm_MscAltInput_lowext = 1, IfxEgtm_MscAltInput_high = 2, IfxEgtm_MscAltInput_highext = 3 } IfxEgtm_MscAltInput;
typedef enum { IfxEgtm_SuspendMode_none = 0, IfxEgtm_SuspendMode_hard = 1, IfxEgtm_SuspendMode_soft = 2 } IfxEgtm_SuspendMode;

/* DTM Clock Source (shared so Atom_Timer can use it) */
typedef enum
{
    IfxEgtm_Dtm_ClockSource_systemClock,
    IfxEgtm_Dtm_ClockSource_cmuClock0,
    IfxEgtm_Dtm_ClockSource_cmuClock1,
    IfxEgtm_Dtm_ClockSource_cmuClock2
} IfxEgtm_Dtm_ClockSource;

/* Forward references for AP configs used by EGTM and PORT drivers */
typedef struct { uint32 dummy; } IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;

/* EGTM AP configuration structs */
typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_ClApConfig;
typedef struct { IfxApProt_ProtConfig proteConfig; IfxApApu_ApuConfig apuConfig; } IfxEgtm_CtrlApConfig;
typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_WrapApConfig;

#ifndef IFXEGTM_NUM_CCM_OBJECTS
# define IFXEGTM_NUM_CCM_OBJECTS (3)
#endif

typedef struct
{
    IfxApProt_ProtConfig protseConfig;
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;

/* MSC Out */
typedef enum { IfxEgtm_Cfg_MscSet_0 = 0 } IfxEgtm_Cfg_MscSet;
typedef enum { IfxEgtm_Cfg_MscSetSignal_0 = 0 } IfxEgtm_Cfg_MscSetSignal;
typedef enum { IfxEgtm_Cfg_MscModule_0 = 0 } IfxEgtm_Cfg_MscModule;
typedef enum { IfxEgtm_Cfg_MscSelect_0 = 0 } IfxEgtm_Cfg_MscSelect;

typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* Functions (only those required by mocks) */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
void    IfxEgtm_enable(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
