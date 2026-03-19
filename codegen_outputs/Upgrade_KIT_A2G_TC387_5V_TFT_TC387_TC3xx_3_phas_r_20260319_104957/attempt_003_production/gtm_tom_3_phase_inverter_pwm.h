/*
 * gtm_tom_3_phase_inverter_pwm.h
 * GTM TOM 3-Phase Inverter PWM Driver (TC3xx / TC387)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

/*
 * Configuration values from requirements (HIGH PRIORITY)
 */
#define TIMEBASE_FREQUENCY_HZ                (20000.0f)     /* 20 kHz */
#define TIMING_PWM_FREQUENCY_HZ              (20000.0f)
#define TIMEBASE_ALIGNMENT_CENTER_ALIGNED    (1)
#define TIMEBASE_CLOCK_SOURCE_FXCLK0         (1)
#define INITIAL_DUTY_PERCENT_U               (25.0f)
#define INITIAL_DUTY_PERCENT_V               (50.0f)
#define INITIAL_DUTY_PERCENT_W               (75.0f)
#define DUTY_UPDATE_POLICY_INTERVAL_MS       (500U)
#define TIMING_UPDATE_INTERVAL_MS            (500U)
#define DUTY_UPDATE_POLICY_STEP_PERCENT      (10.0f)
#define TIMING_SOFTWARE_DEAD_TIME_US         (0.5f)
#define CLOCK_RETAIN_EXISTING_GTM_CLOCK_INIT (1)

/* Number of PWM channels used: 6 (U_HS, U_LS, V_HS, V_LS, W_HS, W_LS) */
#define NUM_OF_CHANNELS                      (6U)

/*
 * Pin mapping – keep existing board mapping (KIT_A2G_TC387_5V_TFT)
 * Use generic pin map header (IfxGtm_PinMap.h) in the source file.
 */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

#ifdef __cplusplus
extern "C" {
#endif

/* Public API (from SW Detailed Design) */
void initGtmTomPwm(void);
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
