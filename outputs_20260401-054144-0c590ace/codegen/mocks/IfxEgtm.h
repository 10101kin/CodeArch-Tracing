#ifndef IFXEGTM_H
#define IFXEGTM_H
#include "mock_egtm_atom_tmadc_consolidated.h"

/* Minimal SFR cluster placeholders used in other headers */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_ATOM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_ATOM_AGC;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_CDTM_DTM;

/* Enums required by Pwm/Timer */
typedef enum { IfxEgtm_AeiBridgeOpMode_sync = 0u, IfxEgtm_AeiBridgeOpMode_async = 1u } IfxEgtm_AeiBridgeOpMode;
typedef enum { IfxEgtm_ClusterClockDiv_disable = 0u, IfxEgtm_ClusterClockDiv_enable = 1u, IfxEgtm_ClusterClockDiv_enableDiv2 = 2u } IfxEgtm_ClusterClockDiv;
typedef enum { IfxEgtm_IrqMode_level = 0, IfxEgtm_IrqMode_pulse = 1, IfxEgtm_IrqMode_pulseNotify = 2, IfxEgtm_IrqMode_singlePulse = 3 } IfxEgtm_IrqMode;
typedef enum { IfxEgtm_MscAltInput_low = 0, IfxEgtm_MscAltInput_lowext = 1, IfxEgtm_MscAltInput_high = 2, IfxEgtm_MscAltInput_highext = 3 } IfxEgtm_MscAltInput;
typedef enum { IfxEgtm_SuspendMode_none = 0, IfxEgtm_SuspendMode_hard = 1, IfxEgtm_SuspendMode_soft = 2 } IfxEgtm_SuspendMode;

/* Cluster enum (single-owner here to avoid redefinition) */
typedef enum { IfxEgtm_Cluster_0 = 0, IfxEgtm_Cluster_1 = 1, IfxEgtm_Cluster_2 = 2 } IfxEgtm_Cluster;

/* MSC config helper enums (minimal) */
typedef enum { IfxEgtm_Cfg_MscSet_0 = 0 } IfxEgtm_Cfg_MscSet;
typedef enum { IfxEgtm_Cfg_MscSetSignal_0 = 0 } IfxEgtm_Cfg_MscSetSignal;
typedef enum { IfxEgtm_Cfg_MscModule_0 = 0 } IfxEgtm_Cfg_MscModule;
typedef enum { IfxEgtm_Cfg_MscSelect_0 = 0 } IfxEgtm_Cfg_MscSelect;

/* IfxEgtm_MscOut struct */
typedef struct {
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* Base enable/status API (decl only; stubs in mock .c) */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
