/* IfxGtm.h - Base GTM peripheral mock */
#ifndef IFXGTM_H
#define IFXGTM_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Enums */
typedef enum {
    IfxGtm_IrqMode_level       = 0,
    IfxGtm_IrqMode_pulse       = 1,
    IfxGtm_IrqMode_pulseNotify = 2,
    IfxGtm_IrqMode_singlePulse = 3
} IfxGtm_IrqMode;

typedef enum {
    IfxGtm_SuspendMode_none = 0,
    IfxGtm_SuspendMode_hard = 1,
    IfxGtm_SuspendMode_soft = 2
} IfxGtm_SuspendMode;

/* Cluster enum (base peripheral mock must define this) */
typedef enum {
    IfxGtm_Cluster_0 = 0,
    IfxGtm_Cluster_1 = 1
} IfxGtm_Cluster;

/* Minimal SFR cluster type stubs referenced by PWM mock */
typedef struct { uint32 reserved; } Ifx_GTM_ATOM;
typedef struct { uint32 reserved; } Ifx_GTM_TOM;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM;

/* Misc GTM-related types referenced by PWM mock */
typedef struct { uint32 reserved; } IfxGtm_Atom_ToutMap;
typedef struct { uint32 reserved; } IfxGtm_Tom_ToutMap;
typedef struct { uint32 reserved; } IfxGtm_Trig_MscOut;

/* Function declarations */
boolean IfxGtm_isEnabled(Ifx_GTM *gtm);
boolean IfxGtm_isModuleSuspended(Ifx_GTM *gtm);
void    IfxGtm_setSuspendMode(Ifx_GTM *gtm, IfxGtm_SuspendMode mode);
void    IfxGtm_disable(Ifx_GTM *gtm);
void    IfxGtm_enable(Ifx_GTM *gtm);
float32 IfxGtm_getSysClkFrequency(Ifx_GTM *gtm);
float32 IfxGtm_getClusterFrequency(Ifx_GTM *gtm, uint32 cluster);

#endif /* IFXGTM_H */
