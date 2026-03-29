#ifndef IFXGTM_H
#define IFXGTM_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* GTM clusters and SFR cluster types used by PWM driver */
typedef enum {
    IfxGtm_Cluster_0 = 0,
    IfxGtm_Cluster_1 = 1
} IfxGtm_Cluster;

typedef struct { uint32 r; } Ifx_GTM_ATOM;
typedef struct { uint32 r; } Ifx_GTM_TOM;
typedef struct { uint32 r; } Ifx_GTM_CDTM;

typedef struct { uint32 dummy; } IfxGtm_Trig_MscOut;

/* Irq and suspend enums */
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

/* Functions */
boolean IfxGtm_isEnabled(Ifx_GTM *gtm);
boolean IfxGtm_isModuleSuspended(Ifx_GTM *gtm);
void    IfxGtm_setSuspendMode(Ifx_GTM *gtm, IfxGtm_SuspendMode mode);
void    IfxGtm_disable(Ifx_GTM *gtm);
void    IfxGtm_enable(Ifx_GTM *gtm);
float32 IfxGtm_getSysClkFrequency(Ifx_GTM *gtm);
float32 IfxGtm_getClusterFrequency(Ifx_GTM *gtm, IfxGtm_Cluster cluster);

#endif /* IFXGTM_H */
