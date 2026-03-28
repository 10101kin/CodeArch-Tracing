/* Mock IfxEgtm base */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Auxiliary config types used in AP config (minimal stubs) */
typedef struct { uint32 dummy; } IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;

/* MSC cfg enum placeholders */
typedef uint32 IfxEgtm_Cfg_MscSet;
typedef uint32 IfxEgtm_Cfg_MscSetSignal;
typedef uint32 IfxEgtm_Cfg_MscModule;
typedef uint32 IfxEgtm_Cfg_MscSelect;

/* Cluster index type and minimal values (fixes: 'IfxEgtm_Cluster_0' undeclared) */
typedef enum {
    IfxEgtm_Cluster_0 = 0,
    IfxEgtm_Cluster_1 = 1,
    IfxEgtm_Cluster_2 = 2,
    IfxEgtm_Cluster_3 = 3
} IfxEgtm_Cluster;

/* Missing SFR pointer type used in PWM */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

/* IRQ mode, MSC Alt input, suspend */
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

typedef enum {
    IfxEgtm_AeiBridgeOpMode_sync  = 0u,
    IfxEgtm_AeiBridgeOpMode_async = 1u
} IfxEgtm_AeiBridgeOpMode;

typedef enum {
    IfxEgtm_ClusterClockDiv_disable    = 0u,
    IfxEgtm_ClusterClockDiv_enable     = 1u,
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u
} IfxEgtm_ClusterClockDiv;

/* MSC OUT struct */
typedef struct {
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* AP config containers */
typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_ClApConfig;
typedef struct { IfxApProt_ProtConfig proteConfig; IfxApApu_ApuConfig apuConfig; } IfxEgtm_CtrlApConfig;
typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_WrapApConfig;

typedef struct {
    IfxApProt_ProtConfig protseConfig;
    IfxEgtm_ClApConfig   clApConfig[1];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;

/* Functions (subset needed) */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
void    IfxEgtm_enable(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
