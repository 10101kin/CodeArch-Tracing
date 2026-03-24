/**********************************************************************************************************************
 * 
 *  File: gtm_tom_3_phase_inverter_pwm.h
 *  Brief: GTM TOM 3-Phase Inverter PWM driver interface (TC3xx, IfxGtm_Pwm high-level driver)
 *
 *  Requirements addressed:
 *   - 20 kHz center-aligned PWM, synchronous updates
 *   - Initial duties U=25%, V=50%, W=75%
 *   - Step duties by +10% every 500 ms, wrap at >100% to 0%
 *   - Keep TOM1 channels 1..6 on P00.2..P00.7 (KIT_A2G_TC387_5V_TFT mapping)
 *   - Retain IfxGtm_Tom_Timer as time base
 *
 **********************************************************************************************************************/
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm_PinMap.h"   /* Generic pin map (TC3xx-compatible) */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================== Configuration Macros (from requirements) ============================== */
#define NUM_OF_CHANNELS                 (6)

/* Timing and duties (percent-based for unified driver) */
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)   /* 20 kHz */
#define TIMING_UPDATE_INTERVAL_MS       (500U)       /* Used by caller loop timing */

#define INITIAL_DUTY_PERCENT_U          (25.0f)
#define INITIAL_DUTY_PERCENT_V          (50.0f)
#define INITIAL_DUTY_PERCENT_W          (75.0f)
#define DUTY_STEP_PERCENT               (10.0f)

/* Optional clock expectations from requirements (not enforced here) */
#define CLOCK_REQUIRES_XTAL             (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ  (300U)

/* ============================== Pin assignments (validated TC3xx TOM1 on P00.2..P00.7) ============================== */
/* Keep TOM1 channels 1..6 on P00.2..P00.7 (U pair: ch1/ch2, V pair: ch3/ch4, W pair: ch5/ch6) */
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)  /* TOM1 Ch1 -> P00.2 */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)  /* TOM1 Ch2 -> P00.3 */
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)  /* TOM1 Ch3 -> P00.4 */
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)  /* TOM1 Ch4 -> P00.5 */
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)  /* TOM1 Ch5 -> P00.6 */
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)  /* TOM1 Ch6 -> P00.7 */

/* ============================== Public API ============================== */
/*
 * Initialize GTM module clocks, TOM time base, mux pins, and configure IfxGtm_Pwm with 6 channels (3 complementary pairs).
 * Applies initial duties (U=25%, V=50%, W=75%) atomically and starts synchronized PWM outputs.
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);

/*
 * Step U/V/W duties by +10% (DUTY_STEP_PERCENT) and wrap to 0% if any exceeds 100%.
 * Builds a 6-element duty array [U,U,V,V,W,W] and applies atomically.
 */
void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
