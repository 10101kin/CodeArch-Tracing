#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Verified/required enums */
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

/* Auxiliary types referenced by configs (stubs) */
#ifndef IFXEGTM_NUM_CCM_OBJECTS
#define IFXEGTM_NUM_CCM_OBJECTS (4)
#endif

typedef struct { uint32 dummy; } IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;

typedef enum { IfxEgtm_Cfg_MscSet_a = 0 } IfxEgtm_Cfg_MscSet;
typedef enum { IfxEgtm_Cfg_MscSetSignal_a = 0 } IfxEgtm_Cfg_MscSetSignal;
typedef enum { IfxEgtm_Cfg_MscModule_0 = 0 } IfxEgtm_Cfg_MscModule;
typedef enum { IfxEgtm_Cfg_MscSelect_0 = 0 } IfxEgtm_Cfg_MscSelect;

/* Structs per verified layout */
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

/* Function declarations (subset per template mapping) */
void    IfxEgtm_disable(Ifx_EGTM *egtm);
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
boolean IfxEgtm_isModuleSuspended(Ifx_EGTM *egtm);
void    IfxEgtm_setSuspendMode(Ifx_EGTM *egtm, IfxEgtm_SuspendMode mode);
float32 IfxEgtm_getSysClkFrequency(void);
float32 IfxEgtm_getClusterFrequency(Ifx_EGTM *egtm, uint32 cluster);
float32 IfxEgtm_tickToS(Ifx_EGTM *egtm, uint32 ticks);
uint32  IfxEgtm_sToTick(Ifx_EGTM *egtm, float32 seconds);
void    IfxEgtm_initProtSe(Ifx_EGTM *egtm, void *protSeCfg);
void    IfxEgtm_initCtrlProt(Ifx_EGTM *egtm, void *protCfg);
void    IfxEgtm_initClApu(Ifx_EGTM *egtm, const IfxEgtm_ClApConfig *cfg);
void    IfxEgtm_initCtrlApu(Ifx_EGTM *egtm, const IfxEgtm_CtrlApConfig *cfg);
void    IfxEgtm_initWrapApu(Ifx_EGTM *egtm, const IfxEgtm_WrapApConfig *cfg);
void    IfxEgtm_setClusterClockDiv(Ifx_EGTM *egtm, uint32 cluster, IfxEgtm_ClusterClockDiv div);
void    IfxEgtm_clearClusterClockDiv(Ifx_EGTM *egtm, uint32 cluster);
void    IfxEgtm_setResetProtection(Ifx_EGTM *egtm, boolean enabled);
boolean IfxEgtm_getResetProtection(Ifx_EGTM *egtm);
void    IfxEgtm_setHrpwmEnable(Ifx_EGTM *egtm, boolean enabled);
void    IfxEgtm_setHrpwmChannelEnable(Ifx_EGTM *egtm, uint32 channel, boolean enabled);
void    IfxEgtm_enableAeiBridgeWriteBuffer(Ifx_EGTM *egtm);
void    IfxEgtm_disableAeiBridgeWriteBuffer(Ifx_EGTM *egtm);
void    IfxEgtm_setAeiBridgeOpMode(Ifx_EGTM *egtm, IfxEgtm_AeiBridgeOpMode mode);
void    IfxEgtm_ConnectToMsc(Ifx_EGTM *egtm, const IfxEgtm_MscOut *mscOut);
void    IfxEgtm_initApConfig(IfxEgtm_ApConfig *cfg);
void    IfxEgtm_initAp(Ifx_EGTM *egtm, const IfxEgtm_ApConfig *cfg);
void    IfxEgtm_configureAccessToEgtms(boolean enable);
float32 IfxClock_geteGtmFrequency(void);

#endif /* IFXEGTM_H */
