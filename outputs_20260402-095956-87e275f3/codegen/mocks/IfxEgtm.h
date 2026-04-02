#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Provide SFR cluster type used by PWM */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

/* Enums and types from driver-specific knowledge */
typedef enum { IfxEgtm_AeiBridgeOpMode_sync = 0u, IfxEgtm_AeiBridgeOpMode_async = 1u } IfxEgtm_AeiBridgeOpMode;
typedef enum { IfxEgtm_ClusterClockDiv_disable = 0u, IfxEgtm_ClusterClockDiv_enable = 1u, IfxEgtm_ClusterClockDiv_enableDiv2 = 2u } IfxEgtm_ClusterClockDiv;
typedef enum { IfxEgtm_IrqMode_level = 0, IfxEgtm_IrqMode_pulse = 1, IfxEgtm_IrqMode_pulseNotify = 2, IfxEgtm_IrqMode_singlePulse = 3 } IfxEgtm_IrqMode;
typedef enum { IfxEgtm_MscAltInput_low = 0, IfxEgtm_MscAltInput_lowext = 1, IfxEgtm_MscAltInput_high = 2, IfxEgtm_MscAltInput_highext = 3 } IfxEgtm_MscAltInput;
typedef enum { IfxEgtm_SuspendMode_none = 0, IfxEgtm_SuspendMode_hard = 1, IfxEgtm_SuspendMode_soft = 2 } IfxEgtm_SuspendMode;

typedef struct { uint32 dummy; } IfxEgtm_ClApConfig;
typedef struct { uint32 dummy; } IfxEgtm_CtrlApConfig;
typedef struct { uint32 dummy; } IfxEgtm_WrapApConfig;
typedef struct { uint32 dummy; } IfxEgtm_ApConfig;
typedef struct { uint32 dummy; } IfxEgtm_MscOut;

/* Functions used */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
