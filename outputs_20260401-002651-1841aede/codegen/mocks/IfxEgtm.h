/* IfxEgtm.h - Per-driver mock header */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Enums */
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

/* Minimal dependent config enums/typedefs used by structs */
typedef uint8 IfxEgtm_Cfg_MscSet;
typedef uint8 IfxEgtm_Cfg_MscSetSignal;
typedef uint8 IfxEgtm_Cfg_MscModule;
typedef uint8 IfxEgtm_Cfg_MscSelect;

/* Structs */
typedef struct {
    IfxApApu_ApuConfig apuConfig;
} IfxEgtm_ClApConfig;

typedef struct {
    IfxApProt_ProtConfig proteConfig;
    IfxApApu_ApuConfig   apuConfig;
} IfxEgtm_CtrlApConfig;

typedef struct {
    IfxApApu_ApuConfig apuConfig;
} IfxEgtm_WrapApConfig;

typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;             
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;       
    IfxEgtm_Cfg_MscModule    mscModule;          
    IfxEgtm_Cfg_MscSelect    mscSelect;          
    IfxEgtm_MscAltInput      mscAltIn;           
} IfxEgtm_MscOut;

typedef struct {
    IfxApProt_ProtConfig protseConfig;
    IfxEgtm_ClApConfig   clApConfig[1];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;

/* Minimal APIs needed by production */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
void    IfxEgtm_enable(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
