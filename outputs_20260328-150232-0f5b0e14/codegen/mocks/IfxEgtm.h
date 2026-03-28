/* Mock of IfxEgtm.h */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* EGTM cluster SFR stub */
typedef struct Ifx_EGTM_CLS { uint32 reserved; } Ifx_EGTM_CLS;

/* Additional config-related stub enums/typedefs */
typedef enum { IfxEgtm_AeiBridgeOpMode_sync = 0u, IfxEgtm_AeiBridgeOpMode_async = 1u } IfxEgtm_AeiBridgeOpMode;

typedef enum { IfxEgtm_ClusterClockDiv_disable = 0u, IfxEgtm_ClusterClockDiv_enable = 1u, IfxEgtm_ClusterClockDiv_enableDiv2 = 2u } IfxEgtm_ClusterClockDiv;

typedef enum { IfxEgtm_IrqMode_level = 0, IfxEgtm_IrqMode_pulse = 1, IfxEgtm_IrqMode_pulseNotify = 2, IfxEgtm_IrqMode_singlePulse = 3 } IfxEgtm_IrqMode;

typedef enum { IfxEgtm_MscAltInput_low = 0, IfxEgtm_MscAltInput_lowext = 1, IfxEgtm_MscAltInput_high = 2, IfxEgtm_MscAltInput_highext = 3 } IfxEgtm_MscAltInput;

typedef enum { IfxEgtm_SuspendMode_none = 0, IfxEgtm_SuspendMode_hard = 1, IfxEgtm_SuspendMode_soft = 2 } IfxEgtm_SuspendMode;

/* Simplified cluster and DTM clock source typedefs */
typedef uint32 IfxEgtm_Cluster;
typedef enum { IfxEgtm_Dtm_ClockSource_cmuClk0 = 0, IfxEgtm_Dtm_ClockSource_cmuClk1 = 1, IfxEgtm_Dtm_ClockSource_cmuClk2 = 2, IfxEgtm_Dtm_ClockSource_cmuClk6 = 6, IfxEgtm_Dtm_ClockSource_cmuClk7 = 7 } IfxEgtm_Dtm_ClockSource;

/* APU/PROT config placeholder types (structures only used as opaque in tests) */
typedef struct { uint32 dummy; } IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;

/* AP config wrappers */
typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_ClApConfig;
typedef struct { IfxApProt_ProtConfig proteConfig; IfxApApu_ApuConfig apuConfig; } IfxEgtm_CtrlApConfig;
typedef struct { IfxApApu_ApuConfig apuConfig; } IfxEgtm_WrapApConfig;

typedef struct
{
    IfxApProt_ProtConfig protseConfig;
    IfxEgtm_ClApConfig   clApConfig[2];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;

typedef enum { IfxEgtm_Cfg_MscSet_0 = 0 } IfxEgtm_Cfg_MscSet;
typedef enum { IfxEgtm_Cfg_MscSetSignal_0 = 0 } IfxEgtm_Cfg_MscSetSignal;
typedef enum { IfxEgtm_Cfg_MscModule_0 = 0 } IfxEgtm_Cfg_MscModule;
typedef enum { IfxEgtm_Cfg_MscSelect_0 = 0 } IfxEgtm_Cfg_MscSelect;

typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;
    IfxEgtm_Cfg_MscModule    mscModule;
    IfxEgtm_Cfg_MscSelect    mscSelect;
    IfxEgtm_MscAltInput      mscAltIn;
} IfxEgtm_MscOut;

/* Function declarations (subset required by rules) */
void    IfxEgtm_disable(Ifx_EGTM *egtm);
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
boolean IfxEgtm_isModuleSuspended(Ifx_EGTM *egtm);
void    IfxEgtm_setSuspendMode(Ifx_EGTM *egtm, IfxEgtm_SuspendMode mode);
float32 IfxEgtm_getSysClkFrequency(Ifx_EGTM *egtm);
float32 IfxEgtm_getClusterFrequency(Ifx_EGTM *egtm);
float32 IfxEgtm_tickToS(Ifx_EGTM *egtm, uint32 ticks);
uint32  IfxEgtm_sToTick(Ifx_EGTM *egtm, float32 seconds);
void    IfxEgtm_initProtSe(Ifx_EGTM *egtm);
void    IfxEgtm_initCtrlProt(Ifx_EGTM *egtm);
void    IfxEgtm_initClApu(Ifx_EGTM *egtm, IfxEgtm_ClApConfig *cfg);
void    IfxEgtm_initCtrlApu(Ifx_EGTM *egtm, IfxEgtm_CtrlApConfig *cfg);
void    IfxEgtm_initWrapApu(Ifx_EGTM *egtm, IfxEgtm_WrapApConfig *cfg);
void    IfxEgtm_setClusterClockDiv(Ifx_EGTM *egtm, uint32 clusterIdx, IfxEgtm_ClusterClockDiv div);
void    IfxEgtm_clearClusterClockDiv(Ifx_EGTM *egtm, uint32 clusterIdx);
void    IfxEgtm_setResetProtection(Ifx_EGTM *egtm, boolean enable);
boolean IfxEgtm_getResetProtection(Ifx_EGTM *egtm);
void    IfxEgtm_setHrpwmEnable(Ifx_EGTM *egtm, boolean enable);
void    IfxEgtm_setHrpwmChannelEnable(Ifx_EGTM *egtm, uint32 ch, boolean enable);
void    IfxEgtm_enableAeiBridgeWriteBuffer(Ifx_EGTM *egtm);
void    IfxEgtm_disableAeiBridgeWriteBuffer(Ifx_EGTM *egtm);
void    IfxEgtm_setAeiBridgeOpMode(Ifx_EGTM *egtm, IfxEgtm_AeiBridgeOpMode mode);
void    IfxEgtm_ConnectToMsc(Ifx_EGTM *egtm, IfxEgtm_MscOut *msc);
void    IfxEgtm_initApConfig(IfxEgtm_ApConfig *cfg);
void    IfxEgtm_initAp(Ifx_EGTM *egtm, IfxEgtm_ApConfig *cfg);
void    IfxEgtm_configureAccessToEgtms(void);
float32 IfxClock_geteGtmFrequency(void);

#endif /* IFXEGTM_H */
