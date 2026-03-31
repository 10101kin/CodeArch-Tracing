/* Mock IfxEgtm.h */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* EGTM peer/basic dependent placeholders */
typedef struct
{
    unsigned long wraTagId;
    unsitypedef struct
{
    IfxApProt_Owner protOwner;
    IfxApProt_State protState;
} IfxApProt_ProtConfig;signed long rdbTagId;
    unsigned char vmWrId;
    unsigned char vmRdId;
    unsigned char prsWrId;
    unsigned char prsRdId;
} IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;

typedef struct { uint32 reserved; } Ifx_EGTM_CLS; /* cluster SFR stub */

/* Enums required */
typedef enum
{
    IfxEgtm_AeiBridgeOpMode_sync  = 0u, 
    IfxEgtm_AeiBridgeOpMode_async = 1u  
} IfxEgtm_AeiBritypedef enum
{
    IfxEgtm_ClusterClockDiv_disable    = 0u, 
    IfxEgtm_ClusterClockDiv_enable     = 1u, 
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u  
} Ifxtypedef enum
{
    IfxEgtm_IrqMode_level       = 0,
    IfxEgtm_IrqMode_pulse       = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgttypedef enum
{
    IfxEgtm_MscAltInput_low     = 0,
    IfxEgtm_MscAltInput_lowext  = 1,
    IfxEgtm_MscAltInput_high    = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAlttypedef enum
{
    IfxEgtm_SuspendMode_none = 0,  
    IfxEgtm_SuspendMode_hard = 1,  
    IfxEgtm_SuspendMode_soft = 2   
} IfxEgtm_SuspendMode; 1, IfxEgtm_SuspendMode_softypedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;             
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;       
    IfxEgtm_Cfg_MscModule    mscModule;          
    IfxEgtm_Cfg_MscSelect    mscSelect;          
    IfxEgtm_MscAltInput      mscAltIn;           
} IfxEgtm_MscOut;      
    IfxEgtm_MscAltInput      mscAltIn;           
} IfxEgtm_MscOut;tm_Cfg_MscSelect;

typedef struct {
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* Basic EGTM control prototypes used by module */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
