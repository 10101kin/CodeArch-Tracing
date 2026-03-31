#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* ===== Enums required by template mapping ===== */
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

/* ===== Peer types (minimal placeholders to satisfy struct composition) ===== */
typedef struct { uint32 reserved; } IfxApApu_ApuConfig;
typedef struct { uint32 reserved; } IfxApProt_ProtConfig;

#ifndef IFXEGTM_NUM_CCM_OBJECTS
#define IFXEGTM_NUM_CCM_OBJECTS (2u)
#endif

/* ===== Struct definitions as per template mapping ===== */
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

typedef struct {
    IfxApProt_ProtConfig protseConfig;
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;

typedef struct {
    uint32                mscSet;
    uint32                mscSetSignal;
    uint32                mscModule;
    uint32                mscSelect;
    IfxEgtm_MscAltInput   mscAltIn;
} IfxEgtm_MscOut;

/* ===== Function declarations ===== */
void    IfxEgtm_disable(Ifx_EGTM *egtm);
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
boolean IfxEgtm_isModuleSuspended(Ifx_EGTM *egtm);
void    IfxEgtm_setSuspendMode(Ifx_EGTM *egtm, int mode);
float32 IfxEgtm_getSysClkFrequency(Ifx_EGTM *egtm);
float32 IfxEgtm_getClusterFrequency(Ifx_EGTM *egtm, uint32 cluster);
float32 IfxEgtm_tickToS(float32 ticks, float32 freq);
float32 IfxEgtm_sToTick(float32 seconds, float32 freq);
void    IfxEgtm_initProtSe(void *cfg);
void    IfxEgtm_initCtrlProt(void *cfg);
void    IfxEgtm_initClApu(void *cfg);
void    IfxEgtm_initCtrlApu(void *cfg);
void    IfxEgtm_initWrapApu(void *cfg);
void    IfxEgtm_setClusterClockDiv(Ifx_EGTM *egtm, uint32 cluster, uint32 div);
void    IfxEgtm_clearClusterClockDiv(Ifx_EGTM *egtm, uint32 cluster);
void    IfxEgtm_setResetProtection(Ifx_EGTM *egtm, boolean enable);
boolean IfxEgtm_getResetProtection(Ifx_EGTM *egtm);
void    IfxEgtm_setHrpwmEnable(Ifx_EGTM *egtm, boolean enable);
void    IfxEgtm_setHrpwmChannelEnable(Ifx_EGTM *egtm, uint32 channel, boolean enable);
void    IfxEgtm_enableAeiBridgeWriteBuffer(Ifx_EGTM *egtm);
void    IfxEgtm_disableAeiBridgeWriteBuffer(Ifx_EGTM *egtm);
void    IfxEgtm_setAeiBridgeOpMode(Ifx_EGTM *egtm, uint32 mode);
void    IfxEgtm_ConnectToMsc(Ifx_EGTM *egtm, void *mscCfg);
void    IfxEgtm_initApConfig(void *cfg);
void    IfxEgtm_initAp(void *cfg);
void    IfxEgtm_configureAccessToEgtms(void);
float32 IfxClock_geteGtmFrequency(void);

#endif /* IFXEGTM_H */
