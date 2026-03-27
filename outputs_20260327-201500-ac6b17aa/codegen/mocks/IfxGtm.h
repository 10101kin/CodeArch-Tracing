/* IfxGtm.h - mock */
#ifndef IFXGTM_H
#define IFXGTM_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Enums */
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

/* Cluster enum (shared across GTM) */
typedef enum
{
    IfxGtm_Cluster_0 = 0,
    IfxGtm_Cluster_1 = 1,
    IfxGtm_Cluster_2 = 2,
    IfxGtm_Cluster_3 = 3,
    IfxGtm_Cluster_4 = 4,
    IfxGtm_Cluster_5 = 5,
    IfxGtm_Cluster_6 = 6,
    IfxGtm_Cluster_7 = 7
} IfxGtm_Cluster;

/* Function declarations used by production */
boolean IfxGtm_isEnabled(Ifx_GTM *gtm);
void    IfxGtm_enable(Ifx_GTM *gtm);

#ifdef __cplusplus
}
#endif

#endif /* IFXGTM_H */
