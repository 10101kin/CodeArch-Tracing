#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration values from requirements */
#define NUM_OF_CHANNELS                          (3U)
#define PWM_FREQUENCY                            (20000.0f)   /* 20 kHz */
#define INITIAL_DUTY_PERCENT_U                   (25.0f)
#define INITIAL_DUTY_PERCENT_V                   (50.0f)
#define INITIAL_DUTY_PERCENT_W                   (75.0f)
#define UPDATE_POLICY_INTERVAL_MS                (500U)
#define UPDATE_POLICY_INCREMENT_PERCENT          (10.0f)
#define UPDATE_POLICY_CLAMP_100_TO_PERIOD_MINUS_1_TICK   (1)
#define COMPLEMENTARY_OUTPUTS_REQUIRED           (0)
#define COMPLEMENTARY_OUTPUTS_DEADTIME_US        (0.5f)
#define COMPLEMENTARY_OUTPUTS_ENABLE_TOM_CDTM_DTM (0)
#define TIMING_CENTER_ALIGNED                    (1)
#define TIMING_SYNCHRONOUS_SHADOW_TRANSFER       (1)
#define CLOCK_REQUIRES_XTAL                      (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ           (300U)
#define CLOCK_GTM_CMU_FXCLK_MHZ                  (100U)

/* Pin assignments for KIT_A2G_TC387_5V_TFT (P00.2..P00.7) keeping TOM1 */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)  /* U High-side */
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)  /* U Low-side (unused, set safe) */
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)  /* V High-side */
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)  /* V Low-side (unused, set safe) */
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)  /* W High-side */
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)  /* W Low-side (unused, set safe) */

/* Public API */
void initGtmTomPwm(void);
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
