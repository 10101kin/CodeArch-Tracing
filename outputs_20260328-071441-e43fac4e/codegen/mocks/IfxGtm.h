#ifndef IFXGTM_H
#define IFXGTM_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Enums */
typedef enum {
    IfxGtm_IrqMode_level = 0,
    IfxGtm_IrqMode_pulse = 1,
    IfxGtm_IrqMode_pulseNotify = 2,
    IfxGtm_IrqMode_singlePulse = 3
} IfxGtm_IrqMode;

typedef enum {
    IfxGtm_SuspendMode_none = 0,
    IfxGtm_SuspendMode_hard = 1,
    IfxGtm_SuspendMode_soft = 2
} IfxGtm_SuspendMode;

typedef enum {
    IfxGtm_Cluster_0 = 0,
    IfxGtm_Cluster_1,
    IfxGtm_Cluster_2,
    IfxGtm_Cluster_3,
    IfxGtm_Cluster_4,
    IfxGtm_Cluster_5,
    IfxGtm_Cluster_6,
    IfxGtm_Cluster_7
} IfxGtm_Cluster;

/* Function declarations */
void    IfxGtm_enable(Ifx_GTM *gtm);
boolean IfxGtm_isEnabled(Ifx_GTM *gtm);
boolean IfxGtm_isModuleSuspended(Ifx_GTM *gtm);
void    IfxGtm_setSuspendMode(Ifx_GTM *gtm, int mode);
void    IfxGtm_disable(Ifx_GTM *gtm);
float32 IfxGtm_getSysClkFrequency(Ifx_GTM *gtm);
float32 IfxGtm_getClusterFrequency(Ifx_GTM *gtm, int cluster);

#endif /* IFXGTM_H */
