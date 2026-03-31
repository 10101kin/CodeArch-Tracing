#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Provide dependent types used by EGTM and peers */
#ifndef IFXEGTM_NUM_CCM_OBJECTS
#define IFXEGTM_NUM_CCM_OBJECTS 3
#endif

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
} IfxApApu_ApuConfig;   /* minimal stub */
typedef struct
{
    IfxApProt_Owner protOwner;
    IfxApProt_State protState;
} IfxApProt_ProtConfig; /* minimal stub */

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

/* Minimal MSC cfg enums referenced by IfxEgtm_MscOut */
typedef enum { IfxEgtm_Cfg_MscSet_none = 0 } IfxEgtm_Cfg_MscSet;
typedef enum { IfxEgtm_Cfg_MscSetSignal_none = 0 } IfxEgtm_Cfg_MscSetSignal;
typedef enum { IfxEgtm_Cfg_MscModule_0 = 0 } IfxEgtm_Cfg_MscModule;
typedef enum { IfxEgtm_Cfg_MscSelect_0 = 0 } IfxEgtm_Cfg_MscSelect;

/* EGTM AP configuration types (minimal, keep field names) */
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

typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;             
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;       
    IfxEgtm_Cfg_MscModule    mscModule;          
    IfxEgtm_Cfg_MscSelect    mscSelect;          
    IfxEgtm_MscAltInput      mscAltIn;           
} IfxEgtm_MscOut;

/* Minimal APIs used by production */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
