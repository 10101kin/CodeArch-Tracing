/* Mock of IfxEgtm.h */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Provide minimal cluster SFR type used in PWM structs */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

/* Basic dependent config stubs (single-owner here) */
typedef struct { uint32 dummy; } IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;

/* MSC config enums referenced by IfxEgtm_MscOut */
typedef enum { IfxEgtm_Cfg_MscSet_a = 0 } IfxEgtm_Cfg_MscSet;
typedef enum { IfxEgtm_Cfg_MscSetSignal_a = 0 } IfxEgtm_Cfg_MscSetSignal;
typedef enum { IfxEgtm_Cfg_MscModule_0 = 0 } IfxEgtm_Cfg_MscModule;
typedef enum { IfxEgtm_Cfg_MscSelect_0 = 0 } IfxEgtm_Cfg_MscSelect;

/* Enums required by driver */
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

/* Some headers define IfxEgtm_Cluster as enum; model as uint32 here */
typedef uint32 IfxEgtm_Cluster;

/* IfxSrc VM Id (needed by PWM interrupt config). Include enumerators to fix previous build error. */
typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1,
    IfxSrc_VmId_2,
    IfxSrc_VmId_3
} IfxSrc_VmId;

/* Ap and MSC configs */
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
    /* size-only placeholders for arrays not used by test code */
    IfxEgtm_ClApConfig   clApConfig[1];
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

/* Functions */
void    IfxEgtm_disable(Ifx_EGTM *egtm);
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
boolean IfxEgtm_isModuleSuspended(Ifx_EGTM *egtm);
void    IfxEgtm_setSuspendMode(Ifx_EGTM *egtm, IfxEgtm_SuspendMode mode);
float32 IfxEgtm_getSysClkFrequency(Ifx_EGTM *egtm);
float32 IfxEgtm_getClusterFrequency(Ifx_EGTM *egtm, IfxEgtm_Cluster cluster);
float32 IfxEgtm_tickToS(Ifx_EGTM *egtm, uint32 ticks);
uint32  IfxEgtm_sToTick(Ifx_EGTM *egtm, float32 seconds);
void    IfxEgtm_initProtSe(Ifx_EGTM *egtm);
void    IfxEgtm_initCtrlProt(Ifx_EGTM *egtm);
void    IfxEgtm_initClApu(Ifx_EGTM *egtm, IfxEgtm_ClApConfig *cfg);
void    IfxEgtm_initCtrlApu(Ifx_EGTM *egtm, IfxEgtm_CtrlApConfig *cfg);
void    IfxEgtm_initWrapApu(Ifx_EGTM *egtm, IfxEgtm_WrapApConfig *cfg);
void    IfxEgtm_setClusterClockDiv(Ifx_EGTM *egtm, IfxEgtm_Cluster cluster, IfxEgtm_ClusterClockDiv div);
void    IfxEgtm_clearClusterClockDiv(Ifx_EGTM *egtm, IfxEgtm_Cluster cluster);
void    IfxEgtm_setResetProtection(Ifx_EGTM *egtm, boolean en);
boolean IfxEgtm_getResetProtection(Ifx_EGTM *egtm);
void    IfxEgtm_setHrpwmEnable(Ifx_EGTM *egtm, boolean en);
void    IfxEgtm_setHrpwmChannelEnable(Ifx_EGTM *egtm, uint32 ch, boolean en);
void    IfxEgtm_enableAeiBridgeWriteBuffer(Ifx_EGTM *egtm);
void    IfxEgtm_disableAeiBridgeWriteBuffer(Ifx_EGTM *egtm);
void    IfxEgtm_setAeiBridgeOpMode(Ifx_EGTM *egtm, IfxEgtm_AeiBridgeOpMode mode);
void    IfxEgtm_ConnectToMsc(Ifx_EGTM *egtm, IfxEgtm_MscOut *msc);
void    IfxEgtm_initApConfig(IfxEgtm_ApConfig *cfg);
void    IfxEgtm_initAp(Ifx_EGTM *egtm, IfxEgtm_ApConfig *cfg);
void    IfxEgtm_configureAccessToEgtms(void);
float32 IfxClock_geteGtmFrequency(void);

#endif /* IFXEGTM_H */
