/*
 * gtm_tom_3_phase_inverter_pwm.h
 * GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm driver
 * Target: TC3xx (TC387)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm_PinMap.h"   /* Generic pin map header (no family-specific suffix!) */

#ifdef __cplusplus
extern "C" {
#endif

/* ===================== Requirements-derived configuration macros ===================== */
#define NUM_OF_CHANNELS                    (6u)

/* Timebase requirements */
#define TIMEBASE_DRIVER                    IfxGtm_Tom_Timer
#define TIMEBASE_MODULE                    IfxGtm_Tom_1
#define TIMEBASE_FREQUENCY_HZ              (20000.0f) /* 20 kHz */
#define TIMEBASE_ALIGNMENT_CENTER_ALIGNED  (1u)       /* informational */
#define TIMEBASE_CLOCK_SOURCE              IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0

/* Initial duty requirements (percent, 0..100) */
#define INITIAL_DUTY_PERCENT_U             (25.0f)
#define INITIAL_DUTY_PERCENT_V             (50.0f)
#define INITIAL_DUTY_PERCENT_W             (75.0f)

/* Update policy */
#define DUTY_UPDATE_POLICY_INTERVAL_MS     (500u)
#define TIMING_UPDATE_INTERVAL_MS          (500u)
#define DUTY_UPDATE_POLICY_STEP_PERCENT    (10.0f)

/* Software dead-time (microseconds) */
#define TIMING_SOFTWARE_DEAD_TIME_US       (0.5f)

/* Clock policy */
#define CLOCK_RETAIN_EXISTING_GTM_CLOCK_INIT   (1u)

/* ===================== Board pin assignments (validated for TC3xx) ===================== */
/* Keep existing pin mapping as requested */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* Public API (from SW Detailed Design) */
void initGtmTomPwm(void);
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
