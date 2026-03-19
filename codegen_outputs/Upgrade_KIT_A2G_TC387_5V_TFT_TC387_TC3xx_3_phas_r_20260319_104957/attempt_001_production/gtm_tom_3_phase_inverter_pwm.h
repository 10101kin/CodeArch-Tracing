/***************************************************************************
 * File: gtm_tom_3_phase_inverter_pwm.h
 * Description: GTM TOM 3-Phase Inverter PWM driver (TC3xx, unified IfxGtm_Pwm)
 *              Uses TOM1 CH1..CH6 with software-computed complementary duties
 *              and a common IfxGtm_Tom_Timer timebase at 20 kHz center-aligned.
 *
 * Requirements implemented:
 *  - TIMEBASE_DRIVER = IfxGtm_Tom_Timer
 *  - TIMEBASE_MODULE = GTM.TOM1
 *  - TIMEBASE_FREQUENCY_HZ = 20000
 *  - TIMEBASE_ALIGNMENT = center-aligned (up-down)
 *  - TIMEBASE_CLOCK_SOURCE = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0
 *  - INITIAL_DUTY_PERCENT_U = 25
 *  - INITIAL_DUTY_PERCENT_V = 50
 *  - INITIAL_DUTY_PERCENT_W = 75
 *  - DUTY_UPDATE_POLICY_INTERVAL_MS = 500 (handled by caller timing)
 *  - DUTY_UPDATE_POLICY_STEP_PERCENT = 10
 *  - TIMING_SOFTWARE_DEAD_TIME_US = 0.5
 *  - CLOCK_RETAIN_EXISTING_GTM_CLOCK_INIT = True
 *
 * Notes:
 *  - Watchdog disable is NOT performed here (must be in CpuN_Main.c only).
 *  - Uses generic PinMap header (IfxGtm_PinMap.h), not family-specific.
 ***************************************************************************/
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ Configuration Macros ============================ */
#define NUM_OF_CHANNELS                        (6u)

/* Timebase (GTM TOM1) configuration from requirements */
#define TIMEBASE_FREQUENCY_HZ                  (20000.0f)
#define TIMEBASE_CLOCK_SOURCE                  IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0

/* Initial high-side duties (percent) from requirements */
#define INITIAL_DUTY_PERCENT_U                 (25.0f)
#define INITIAL_DUTY_PERCENT_V                 (50.0f)
#define INITIAL_DUTY_PERCENT_W                 (75.0f)

/* Update policy from requirements */
#define DUTY_UPDATE_POLICY_INTERVAL_MS         (500u)
#define DUTY_UPDATE_POLICY_STEP_PERCENT        (10.0f)

/* Software dead-time applied to low-side duty (microseconds) */
#define TIMING_SOFTWARE_DEAD_TIME_US           (0.5f)

/* ============================ Pin Mapping (TOM1 CH1..CH6) ============================ */
/* Keep existing board pins as requested */
#include "IfxGtm_PinMap.h"   /* Generic header (valid for TC3xx) */

#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)  /* TOM1 CH2 */
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)  /* TOM1 CH1 */
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)  /* TOM1 CH4 */
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)  /* TOM1 CH3 */
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)  /* TOM1 CH6 */
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)  /* TOM1 CH5 */

/* ============================ Public API ============================ */
/* Initialize 3-phase PWM on TOM1 CH1..CH6 using IfxGtm_Pwm with TOM timebase */
void initGtmTomPwm(void);

/* Periodic duty update per SW Detailed Design (caller controls the 500 ms cadence) */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
