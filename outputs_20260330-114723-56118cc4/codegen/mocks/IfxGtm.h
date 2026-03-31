/* IfxGtm.h - mock header for GTM base types */
#ifndef IFXGTM_H
#define IFXGTM_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* IRQ mode enum */
typedef enum
{
    IfxGtm_IrqMode_level       = 0,
    IfxGtm_IrqMode_pulse       = 1,
    IfxGtm_IrqMode_pulseNotify = 2,
    IfxGtm_IrqMode_singlePulse = 3
} IfxGtm_IrqMode;

/* Suspend mode enum */
typedef enum
{
    IfxGtm_SuspendMode_none = 0,
    IfxGtm_SuspendMode_hard = 1,
    IfxGtm_SuspendMode_soft = 2
} IfxGtm_SuspendMode;

/* Minimal cluster enum/type used by PWM (simplified) */
typedef enum { IfxGtm_Cluster_0 = 0, IfxGtm_Cluster_1 = 1 } IfxGtm_Cluster;

/* Declarations of functions used by production code */
boolean IfxGtm_isEnabled(Ifx_GTM *gtm);
void IfxGtm_enable(Ifx_GTM *gtm);

/* Provide stub definitions of SFR cluster types referenced by PWM (declared once) */
/* Ifx_GTM_ATOM, Ifx_GTM_TOM and Ifx_GTM_CDTM are defined in IfxGtm_Pwm.h as small stubs */

#endif /* IFXGTM_H */
