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

/* API */
boolean IfxGtm_isEnabled(Ifx_GTM *gtm);
boolean IfxGtm_isModuleSuspended(Ifx_GTM *gtm);
void    IfxGtm_setSuspendMode(Ifx_GTM *gtm, IfxGtm_SuspendMode mode);
void    IfxGtm_disable(Ifx_GTM *gtm);
void    IfxGtm_enable(Ifx_GTM *gtm);
float32 IfxGtm_getSysClkFrequency(Ifx_GTM *gtm);
float32 IfxGtm_getClusterFrequency(Ifx_GTM *gtm);

#endif /* IFXGTM_H */
