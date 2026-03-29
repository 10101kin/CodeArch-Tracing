/*
 * IfxEgtm.h - mock header
 */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Additional shared base types used by structs */
typedef struct { uint32 dummy; } Ifx_EGTM_CLS; /* Cluster SFR stub */

typedef struct { uint32 dummy; } IfxApApu_ApuConfig;    /* shared placeholder */
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;  /* shared placeholder */

typedef uint32 IfxEgtm_Cfg_MscSet;       /* placeholder */
typedef uint32 IfxEgtm_Cfg_MscSetSignal; /* placeholder */
typedef uint32 IfxEgtm_Cfg_MscModule;    /* placeholder */
typedef uint32 IfxEgtm_Cfg_MscSelect;    /* placeholder */

/* Cluster index enum (tests referenced IfxEgtm_Cluster_1) */
typedef enum
{
    IfxEgtm_Cluster_0 = 0,
    IfxEgtm_Cluster_1 = 1,
    IfxEgtm_Cluster_2 = 2,
    IfxEgtm_Cluster_3 = 3
} IfxEgtm_Cluster;

/* Enums per template */
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

/* DTM supplemental enums referenced by PWM FastShutoffConfig */
typedef enum { IfxEgtm_Dtm_ShutoffInput_0 = 0 } IfxEgtm_Dtm_ShutoffInput;
typedef enum { IfxEgtm_Dtm_SignalLevel_low = 0, IfxEgtm_Dtm_SignalLevel_high = 1 } IfxEgtm_Dtm_SignalLevel;

/* Structs per template */
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
    /* in real iLLD: IfxEgtm_ClApConfig clApConfig[IFXEGTM_NUM_CCM_OBJECTS]; Omitted array size in mock */
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

/* Function declarations */
void    IfxEgtm_disable(Ifx_EGTM *egtm);
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);
boolean IfxEgtm_isModuleSuspended(Ifx_EGTM *egtm);
void    IfxEgtm_setSuspendMode(Ifx_EGTM *egtm, IfxEgtm_SuspendMode mode);
float32 IfxEgtm_getSysClkFrequency(void);
float32 IfxEgtm_getClusterFrequency(Ifx_EGTM *egtm, uint32 clusterIndex);
float32 IfxEgtm_tickToS(float32 freq, uint32 ticks);
uint32  IfxEgtm_sToTick(float32 freq, float32 seconds);
void    IfxEgtm_enableAeiBridgeWriteBuffer(Ifx_EGTM *egtm);
void    IfxEgtm_setClusterClockDiv(Ifx_EGTM *egtm, uint32 clusterIndex, IfxEgtm_ClusterClockDiv div);
void    IfxEgtm_clearClusterClockDiv(Ifx_EGTM *egtm, uint32 clusterIndex);
float32 IfxClock_geteGtmFrequency(void);

#endif /* IFXEGTM_H */
