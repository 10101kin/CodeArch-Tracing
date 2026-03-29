/* IfxEgtm.h - Mock */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Additional EGTM-related base types */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef uint8 IfxEgtm_Cluster; /* simplified cluster index */

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

/* MSC related config placeholders */
typedef uint8 IfxEgtm_Cfg_MscSet;
typedef uint8 IfxEgtm_Cfg_MscSetSignal;
typedef uint8 IfxEgtm_Cfg_MscModule;
typedef uint8 IfxEgtm_Cfg_MscSelect;

typedef struct {
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* Functions from DRIVERS TO MOCK */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
void    IfxEgtm_enable(Ifx_EGTM *egtm);

#ifdef __cplusplus
}
#endif

#endif /* IFXEGTM_H */
