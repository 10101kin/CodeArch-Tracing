/* IfxEgtm.h - per-driver mock header */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* ====== Dependent config placeholder types ====== */
typedef struct
{
    unsigned long wraTagId;
    unsigned long wrbTagId;
    unsigned long rdaTagId;
    unsigned long rdbTagId;
    unsigned char vmWrId;
    unsigned char vmRdId;
    unsigned char prsWrId;
    unsigned char prsRdId;
} IfxApApu_ApuConfig;{ uint32 reserved; } IfxApProt_ProtConfig;

/* MSC config enums used in IfxEgtm_MscOut */
typedef enum
{
    IfxEgtm_Cfg_MscSet_0,
    IfxEgtm_Cfg_MscSet_1,
    IfxEgtm_Cfg_MscSet_2,
    IfxEgtm_Cfg_MscSet_3
} IfxEgtm_Cfg_MscSet;{ IfxEgtm_Cfg_MscSetSignal_a = 0 } IfxEgtm_Cfg_MscSetSignal;{ IfxEgtm_Cfg_MscModule_a = 0 } IfxEgtm_Cfg_MscModule;{ IfxEgtm_Cfg_MscSetypedef enum
{
    IfxEgtm_AeiBridgeOpMode_sync  = 0u, 
    IfxEgtm_AeiBridgeOpMode_async = 1u  
} IfxEgtm_AeiBridgeOpMode;pMode_sync = 0u, IfxEgtm_AeiBridgeOpMode_async = 1u } IfxEgtm_AeiBridgeOpMode;{ IfxEgtm_ClusterClockDiv_disable = 0u, IfxEgtm_ClusterClockDiv_enable = 1u, IfxEgtm_ClusterClockDiv_enableDiv2 = 2u } IfxEgtm_ClusterClockDiv;{
    IfxEgtm_IrqMode_level = 0,
    IfxEgtm_IrqMode_pulse = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgtm_IrqMode;{
    IfxEgtm_MscAltInput_low = 0,
    IfxEgtm_MscAltInput_lowext = 1,
    IfxEgtm_MscAltInput_high = 2,
typedef struct
{
    IfxApApu_ApuConfig apuConfig;       
} IfxEgtm_ClApConfig;SuspendMode_none = 0, IfxEgtm_SuspendMode_hard = 1, IfxEgtm_SuspendMode_soft = 2 } IfxEgtm_SuspendMode;

/* ====== Structs ====== */
typedef struct { IfxApApu_ApuConfig apuConfitypedef struct
{
    IfxApProt_ProtConfig protseConfig;                              
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];       
    IfxEgtm_CtrlApConfig ctrlApConfig;                              
    IfxEgtm_WrapApConfig wrapApConfig;                              
} IfxEgtm_ApConfig;edef struct {
    IfxApProt_ProtConfig protseConfig;
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;{
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* ====== Functions (subset used) ====== */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
