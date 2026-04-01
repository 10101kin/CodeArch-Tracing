#ifndef IFXGTM_H
#define IFXGTM_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* GTM base enums */
typedef enum
{
    IfxGtm_IrqMode_level       = 0,
    IfxGtm_IrqMode_pulse       = 1,
    IfxGtm_IrqMode_pulseNotify = 2,
    IfxGtm_IrqMode_singlePulse = 3
} IfxGtm_IrqMode;

typedef enum
{
    IfxGtm_SuspendMode_none = 0,
    IfxGtm_SuspendMode_hard = 1,
    IfxGtm_SuspendMode_soft = 2
} IfxGtm_SuspendMode;

/* Cluster enum (owned here to be shared) */
typedef enum
{
    IfxGtm_Cluster_0,
    IfxGtm_Cluster_1,
    IfxGtm_Cluster_2,
    IfxGtm_Cluster_3
} IfxGtm_Cluster;

/* Minimal SFR type stubs for GTM sub-blocks */
typedef struct { uint32 reserved; } Ifx_GTM_ATOM;
typedef struct { uint32 reserved; } Ifx_GTM_TOM;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM;
typedef struct { uint32 reserved; } Ifx_GTM_TOM_TGC;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM_DTM;

/* GTM base control APIs */
boolean IfxGtm_isEnabled(Ifx_GTM *gtm);
void    IfxGtm_enable(Ifx_GTM *gtm);

#endif /* IFXGTM_H */
