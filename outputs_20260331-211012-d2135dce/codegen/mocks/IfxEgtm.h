/* Mock IfxEgtm.h */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Enums and dependent types used across drivers */
typedef enum
{
    IfxEgtm_AeiBridgeOpMode_sync  = 0u, 
    IfxEgtm_AeiBridgeOpMode_async = 1u  
} IfxEgtm_AeiBridgeOpModetypedef enum
{
    IfxEgtm_ClusterClockDiv_disable    = 0u, 
    IfxEgtm_ClusterClockDiv_enable     = 1u, 
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u  
} IfxEgtm_ClusterClockDitypedef enum
{
    IfxEgtm_IrqMode_level       = 0,
    IfxEgtm_IrqMode_pulse       = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgtm_IrqMode;rqtypedef enum
{
    IfxEgtm_MscAltInput_low     = 0,
    IfxEgtm_MscAltInput_lowext  = 1,
    IfxEgtm_MscAltInput_high    = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAltInput;tItypedef enum
{
    IfxEgtm_SuspendMode_none = 0,  
    IfxEgtm_SuspendMode_hard = 1,  
    IfxEgtm_SuspendMode_soft = 2   
} IfxEgtm_SuspendMode;ndMode;

/* Placeholder enums for MSC config used by IfxEtypedef enum
{
    IfxEgtm_Cfg_MscSet_0,
    IfxEgtm_Cfg_MscSetypedef enum
{
    IfxEgtm_Cfg_MscSetSignal_0,
    IfxEgtm_Cfg_MscSetSignatypedef enum
{
    IfxEgtm_Cfg_MscModule_0
} IfxEgtm_Cfg_MscModule;,typedef enum
{
    IfxEgtm_Cfg_MscSelect_0,
    IfxEgtm_Cfg_MscSelect_1
} IfxEgtm_Cfg_MscSelect;  IfxEgtm_Cfg_MscSetSignal_6,
    IfxEgtm_Cfg_MscSetSignal_7,
    IfxEgtm_Cfg_MscSetSignal_8,
    IfxEgtm_Cfg_MscSetSignal_9,
    IfxEgtm_Cfg_MscSetSignal_10,
    IfxEgtm_Cfg_MscSetSignal_11,
    IfxEgtm_Cfg_MscSetSignal_12,
    IfxEgtm_Cfg_MscSetSignal_13,
    IfxEgtm_Cfg_MscSetSignal_14,
    IfxEgtm_Cfg_MscSetSignal_15
} IfxEgtm_Cfg_MscSetSignal;Set;tm_Cfg_MscSet;
typedef enum { IfxEgtm_Cfg_MscSetSignal_0 = 0 } IfxEgtm_Cfg_MscSetSignal;
typedef enum { IfxEgtm_Cfg_MscModule_0 = 0 } IfxEgtm_Cfg_MscModule;
typedef enum { IfxEgtm_Cfg_MscSelect_0 = 0 } IfxEgtm_Cfg_MscSelect;

/* Structs */
typedef struct
{
    IfxApApu_ApuConfig apuConfig;       
} IfxEgtm_ClApCotypedef struct
{
    IfxApProt_ProtConfig proteConfig;       
    IfxApApu_ApuConfig   apuConfig;         
} IfxEgtmtypedef struct
{
    IfxApApu_ApuConfig apuConfig;       
} IfxEgtm_WrapApCotypedef struct
{
    IfxApProt_ProtConfig protseConfig;                              
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];       
    IfxEgtm_CtrlApConfig ctrlApConfig;                              
    IfxEgtm_WrapApConfig wrapApConfig;   typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;             
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;       
    IfxEgtm_Cfg_MscModule    mscModule;          
    IfxEgtm_Cfg_MscSelect    mscSelect;          
    IfxEgtm_MscAltInput      mscAltIn;           
} IfxEgtm_MscOut;scSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* Minimal API required by this module */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
