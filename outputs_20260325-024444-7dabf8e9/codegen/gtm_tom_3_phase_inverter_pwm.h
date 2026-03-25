/*
 * gtm_tom_3_phase_inverter_pwm.h
 * 3-phase inverter PWM using GTM TOM with unified IfxGtm_Pwm driver and TOM timebase
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration values from requirements */
#define NUM_OF_CHANNELS                         (3u)
#define PWM_FREQUENCY_HZ                        (10000.0f)    /* 10 kHz center-aligned */
#define DEAD_TIME_SEC                           (5.0e-7f)     /* 500 ns */
#define CLOCK_GTM_GCLK_HZ                       (300000000.0f) /* 300 MHz */
#define CLOCK_CMU_CLK0_HZ                       (100000000.0f) /* 100 MHz */

#define DUTY_25_PERCENT                         (25.0f)
#define DUTY_50_PERCENT                         (50.0f)
#define DUTY_75_PERCENT                         (75.0f)
#define DUTY_STEP_PERCENT                       (10.0f)

/* Public API */
void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
