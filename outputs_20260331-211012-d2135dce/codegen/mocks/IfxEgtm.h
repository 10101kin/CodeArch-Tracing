/* IfxEgtm base mock */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Support types used by configs */
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

/* Optional constants */
#ifndef IFXEGTM_NUM_CCM_OBJECTS
#define IFXEGTM_NUM_CCM_OBJECTS (3u)
#endif

/* MSC config enums used by IfxEgtm_MscOut */
typedef enum
{
    IfxEgtm_Cfg_MscSet_0,
    IfxEgtm_Cfg_MscSet_1,
    IfxEgtm_Cfg_MscSetypedef enum
{
    IfxEgtm_Cfg_MscSetSignal_0,
    IfxEgtm_Cfg_MscSetSignal_1,
    IfxEgtm_Cfg_MscSetSignatypedef enum
{
    IfxEgtm_Cfg_MscModule_0
} IfxEgtm_Cfg_MscModule;l_5,
    IfxEgtm_Cfg_MscSetSigtypedef enum
{
    IfxEgtm_Cfg_MscSelect_0,
    IfxEgtm_Cfg_MscSelect_1
} IfxEgtm_Cfg_MscSelect;nal_9,
    IfxEgtm_Cfg_MscSetypedef enum
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
} IfxEgtm_IrqMode;e typedef enum
{
    IfxEgtm_MscAltInput_low     = 0,
    IfxEgtm_MscAltInput_lowext  = 1,
    IfxEgtm_MscAltInput_high    = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAltInput;
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
    IfxEgtmtypedef struct
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
    IfxEgtm_CtrlApConfig ctrlApConfig;                        typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;             
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;       
    IfxEgtm_Cfg_MscModule    mscModule;          
    IfxEgtm_Cfg_MscSelect    mscSelect;          
    IfxEgtm_MscAltInput      mscAltIn;           
} IfxEgtm_MscOut;pConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;

typedef struct {
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* Functions to mock */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
