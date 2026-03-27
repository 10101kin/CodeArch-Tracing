/* IfxEgtm mock header */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Basic EGTM cluster enum */
typedef enum {
    IfxEgtm_Cluster_0 = 0,
    IfxEgtm_Cluster_1,
    IfxEgtm_Cluster_2,
    IfxEgtm_Cluster_3,
    IfxEgtm_Cluster_4,
    IfxEgtm_Cluster_5
} IfxEgtm_Cluster;

/* Supporting config type stubs used by structs below */
typedef struct { uint32 dummy; } IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;

typedef enum {
    IfxEgtm_AeiBridgeOpMode_sync  = 0u,
    IfxEgtm_AeiBridgeOpMode_async = 1u
} IfxEgtm_AeiBridgeOpMode;

typedef enum {
    IfxEgtm_ClusterClockDiv_disable    = 0u,
    IfxEgtm_ClusterClockDiv_enable     = 1u,
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u
} IfxEgtm_ClusterClockDiv;

typedef enum {
    IfxEgtm_IrqMode_level       = 0,
    IfxEgtm_IrqMode_pulse       = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgtm_IrqMode;

typedef enum {
    IfxEgtm_MscAltInput_low     = 0,
    IfxEgtm_MscAltInput_lowext  = 1,
    IfxEgtm_MscAltInput_high    = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAltInput;

typedef enum {
    IfxEgtm_SuspendMode_none = 0,
    IfxEgtm_SuspendMode_hard = 1,
    IfxEgtm_SuspendMode_soft = 2
} IfxEgtm_SuspendMode;

/* EGTM AP/MSC related structs (minimally defined for compilation) */
typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_ClApConfig;

typedef struct { IfxApProt_ProtConfig proteConfig; IfxApApu_ApuConfig apuConfig; } IfxEgtm_CtrlApConfig;

typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_WrapApConfig;

typedef struct {
    IfxApProt_ProtConfig protseConfig;
    IfxEgtm_ClApConfig   clApConfig[4];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;

typedef uint32 IfxEgtm_Cfg_MscSet;
typedef uint32 IfxEgtm_Cfg_MscSetSignal;
typedef uint32 IfxEgtm_Cfg_MscModule;
typedef uint32 IfxEgtm_Cfg_MscSelect;

typedef struct {
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* Functions used by module */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
