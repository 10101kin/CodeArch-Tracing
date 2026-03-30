#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Provide SFR cluster stub used by PWM driver */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

/* Ensure dependent macro exists for array sizes in config structs */
#ifndef IFXEGTM_NUM_CCM_OBJECTS
#define IFXEGTM_NUM_CCM_OBJECTS 1
#endif

/* Types required by IfxEgtm interface */
typedef struct
{
    IfxApApu_ApuConfig apuConfig;
} IfxEgtm_ClApConfig;

typedef struct
{
    IfxApProt_ProtConfig proteConfig;
    IfxApApu_ApuConfig   apuConfig;
} IfxEgtm_CtrlApConfig;

typedef struct
{
    IfxApApu_ApuConfig apuConfig;
} IfxEgtm_WrapApConfig;

typedef struct
{
    IfxApProt_ProtConfig protseConfig;
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;

typedef enum
{
    IfxEgtm_AeiBridgeOpMode_sync  = 0u,
    IfxEgtm_AeiBridgeOpMode_async = 1u
} IfxEgtm_AeiBridgeOpMode;

typedef enum
{
    IfxEgtm_ClusterClockDiv_disable    = 0u,
    IfxEgtm_ClusterClockDiv_enable     = 1u,
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u
} IfxEgtm_ClusterClockDiv;

typedef enum
{
    IfxEgtm_IrqMode_level       = 0,
    IfxEgtm_IrqMode_pulse       = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgtm_IrqMode;

typedef enum
{
    IfxEgtm_MscAltInput_low     = 0,
    IfxEgtm_MscAltInput_lowext  = 1,
    IfxEgtm_MscAltInput_high    = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAltInput;

typedef enum
{
    IfxEgtm_SuspendMode_none = 0,
    IfxEgtm_SuspendMode_hard = 1,
    IfxEgtm_SuspendMode_soft = 2
} IfxEgtm_SuspendMode;

/* MSC OUT config referenced by PWM driver */
typedef struct
{
    uint32               mscSet;       /* placeholder field */
    uint32               mscSetSignal; /* placeholder field */
    uint32               mscModule;    /* placeholder field */
    uint32               mscSelect;    /* placeholder field */
    IfxEgtm_MscAltInput  mscAltIn;     /* actual alt input selection */
} IfxEgtm_MscOut;

/* Minimal API needed by production */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
