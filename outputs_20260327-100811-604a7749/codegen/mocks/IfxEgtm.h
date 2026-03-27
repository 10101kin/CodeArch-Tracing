#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Additional EGTM SFR fragments used by PWM */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

/* Enums required by IfxEgtm_Pwm */
typedef enum {
    IfxEgtm_AeiBridgeOpMode_sync = 0u,
    IfxEgtm_AeiBridgeOpMode_async = 1u
} IfxEgtm_AeiBridgeOpMode;

typedef enum {
    IfxEgtm_ClusterClockDiv_disable = 0u,
    IfxEgtm_ClusterClockDiv_enable = 1u,
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u
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

/* Cluster index enum */
typedef enum {
    IfxEgtm_Cluster_0 = 0,
    IfxEgtm_Cluster_1,
    IfxEgtm_Cluster_2,
    IfxEgtm_Cluster_3,
    IfxEgtm_Cluster_4,
    IfxEgtm_Cluster_5
} IfxEgtm_Cluster;

/* Minimal MSC Out structure used by PWM channel config */
typedef struct {
    uint32 mscSet;
    uint32 mscSetSignal;
    uint32 mscModule;
    uint32 mscSelect;
    IfxEgtm_MscAltInput mscAltIn;
} IfxEgtm_MscOut;

/* Function declarations (only those used in DRIVERS TO MOCK) */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
