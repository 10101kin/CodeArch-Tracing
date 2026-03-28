#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Base peripheral and related types */
typedef uint32 Ifx_UReg_32Bit;            /* 32-bit register type */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS; /* Cluster SFR stub */
typedef uint32 IfxEgtm_Cluster;           /* Cluster index */
typedef uint32 IfxSrc_VmId;               /* VM id type */

/* External MSC config value types used by IfxEgtm_MscOut */
typedef uint32 IfxEgtm_Cfg_MscSet;
typedef uint32 IfxEgtm_Cfg_MscSetSignal;
typedef uint32 IfxEgtm_Cfg_MscModule;
typedef uint32 IfxEgtm_Cfg_MscSelect;

/* Irq mode enum as required */
typedef enum {
    IfxEgtm_IrqMode_level       = 0,
    IfxEgtm_IrqMode_pulse       = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgtm_IrqMode;

/* MSC Alt input */
typedef enum {
    IfxEgtm_MscAltInput_low     = 0,
    IfxEgtm_MscAltInput_lowext  = 1,
    IfxEgtm_MscAltInput_high    = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAltInput;

/* AEI bridge */
typedef enum {
    IfxEgtm_AeiBridgeOpMode_sync  = 0u,
    IfxEgtm_AeiBridgeOpMode_async = 1u
} IfxEgtm_AeiBridgeOpMode;

/* Cluster Clock Div */
typedef enum {
    IfxEgtm_ClusterClockDiv_disable    = 0u,
    IfxEgtm_ClusterClockDiv_enable     = 1u,
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u
} IfxEgtm_ClusterClockDiv;

/* Suspend Mode */
typedef enum {
    IfxEgtm_SuspendMode_none = 0,
    IfxEgtm_SuspendMode_hard = 1,
    IfxEgtm_SuspendMode_soft = 2
} IfxEgtm_SuspendMode;

/* APU / PROT config placeholder types used by AP config */
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;
typedef struct { uint32 dummy; } IfxApApu_ApuConfig;

/* AP config structures */
typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_ClApConfig;

typedef struct {
    IfxApProt_ProtConfig proteConfig;
    IfxApApu_ApuConfig   apuConfig;
} IfxEgtm_CtrlApConfig;

typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_WrapApConfig;

#ifndef IFXEGTM_NUM_CCM_OBJECTS
# define IFXEGTM_NUM_CCM_OBJECTS 1
#endif

typedef struct {
    IfxApProt_ProtConfig protseConfig;
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;

/* MSC Out configuration */
typedef struct {
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* Functions required */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
