/* IfxEgtm.h */
#ifndef IFXEGTM_H
#define IFXEGTM_H
#include "mock_egtm_atom_tmadc_consolidated.h"

/* EGTM sub-SFR placeholder types used by drivers */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_ATOM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_ATOM_AGC;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_CDTM_DTM;

/* Core enums */
typedef enum {
    IfxEgtm_AeiBridgeOpMode_sync = 0,
    IfxEgtm_AeiBridgeOpMode_async = 1
} IfxEgtm_AeiBridgeOpMode;

typedef enum {
    IfxEgtm_ClusterClockDiv_disable = 0,
    IfxEgtm_ClusterClockDiv_enable = 1,
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2
} IfxEgtm_ClusterClockDiv;

typedef enum {
    IfxEgtm_IrqMode_level = 0,
    IfxEgtm_IrqMode_pulse = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgtm_IrqMode;

typedef enum {
    IfxEgtm_MscAltInput_low = 0,
    IfxEgtm_MscAltInput_lowext = 1,
    IfxEgtm_MscAltInput_high = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAltInput;

typedef enum {
    IfxEgtm_SuspendMode_none = 0,
    IfxEgtm_SuspendMode_hard = 1,
    IfxEgtm_SuspendMode_soft = 2
} IfxEgtm_SuspendMode;

typedef enum {
    IfxEgtm_Cluster_0 = 0,
    IfxEgtm_Cluster_1 = 1,
    IfxEgtm_Cluster_2 = 2
} IfxEgtm_Cluster;

/* API */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
