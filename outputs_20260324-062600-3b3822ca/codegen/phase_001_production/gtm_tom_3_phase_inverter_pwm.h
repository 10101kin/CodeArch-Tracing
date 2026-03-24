/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm (single-output)
 * Target: TC3xx
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Requirements-derived configuration values */
#define TIMEBASE_CHANNEL                 (0u)       /* TOM1_CH0 as timebase (requirement reference) */
#define TIMEBASE_ALIGNMENT_CENTER        (1u)
#define TIMEBASE_FREQUENCY_HZ            (20000.0f) /* 20 kHz */
#define TIMING_PWM_FREQUENCY_HZ          (20000.0f) /* 20 kHz */
#define TIMING_DUTY_UPDATE_PERIOD_MS     (500u)
#define TIMING_DUTY_STEP_PERCENT         (10.0f)
#define TIMING_WRAPAROUND_AT_100_TO_0    (1u)
#define CLOCK_REQUIRES_XTAL              (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ   (300u)

/* Channel and initial duty configuration */
#define NUM_OF_CHANNELS                  (3u)
#define DUTY_INIT_U_PERCENT              (25.0f)
#define DUTY_INIT_V_PERCENT              (50.0f)
#define DUTY_INIT_W_PERCENT              (75.0f)

/* Public API */
void initGtmTom3PhaseInverterPwm(void);
void updateGtmTom3PhaseDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
