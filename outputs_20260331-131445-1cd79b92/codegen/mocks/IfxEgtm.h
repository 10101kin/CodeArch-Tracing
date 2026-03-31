#ifndef IFXEGTM_H
#define IFXEGTM_H
#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Peer dependency placeholders used by ApConfig structs */
typedef struct { uint32 dummy; } IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;

/* Types required */
typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_ClApConfig;

typedef struct {
    IfxApProt_ProtConfig proteConfig;
    IfxApApu_ApuConfig   apuConfig;
} IfxEgtm_CtrlApConfig;

typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_WrapApConfig;

typedef struct {
    IfxApProt_ProtConfig protseConfig;
    /* Keep arrays minimal to avoid undefined macros in mock */
    IfxEgtm_ClApConfig   clApConfig[1];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;

/* MSC related placeholder enums/structs */
typedef enum {
    IfxEgtm_MscAltInput_low = 0,
    IfxEgtm_MscAltInput_lowext = 1,
    IfxEgtm_MscAltInput_high = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAltInput;

typedef struct {
    uint32 mscSet;
    uint32 mscSetSignal;
    uint32 mscModule;
    uint32 mscSelect;
    IfxEgtm_MscAltInput mscAltIn;
} IfxEgtm_MscOut;

/* General enums */
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
    IfxEgtm_SuspendMode_none = 0,
    IfxEgtm_SuspendMode_hard = 1,
    IfxEgtm_SuspendMode_soft = 2
} IfxEgtm_SuspendMode;

/* Function declarations (subset required by tests) */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
void IfxEgtm_enable(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
