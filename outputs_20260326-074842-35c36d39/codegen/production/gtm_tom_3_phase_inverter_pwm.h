#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= Requirements-derived configuration ========================= */
#define MASTER_TIMEBASE_TOM_MODULE                 (1u)         /* TOM1 */
#define MASTER_TIMEBASE_CHANNEL                    (0u)

#define MASTER_TIMEBASE_CLOCK                      IfxGtm_Cmu_Fxclk_0  /* cmuFxclk0 */
#define MASTER_TIMEBASE_CLOCK_SOURCE_IS_GCLK       (1u)         /* TRUE */

#define INITIAL_DUTY_PERCENT_U                     (25.0f)
#define INITIAL_DUTY_PERCENT_V                     (50.0f)
#define INITIAL_DUTY_PERCENT_W                     (75.0f)

#define SYNCHRONIZED_UPDATE_USE_DISABLE_APPLY_UPDATE   (1u)

#define TIMING_PWM_FREQUENCY_HZ                    (20000.0f)   /* 20 kHz */
#define TIMING_DEADTIME_US                         (0.5f)       /* microseconds */
#define TIMING_MIN_PULSE_US                        (1.0f)       /* microseconds */

#define CLOCK_REQUIRES_XTAL                        (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ             (300u)
#define CLOCK_EXPECTED_GTM_GCLK_MHZ                (100u)

/* Derived constants */
#define GTM_GCLK_FREQ_HZ                           ((float32)((CLOCK_EXPECTED_GTM_GCLK_MHZ) * 1000000.0f))
#define PWM_FREQUENCY_HZ                           (TIMING_PWM_FREQUENCY_HZ)
#define PWM_DEADTIME_S                             ((float32)((TIMING_DEADTIME_US) * 1.0e-6f))
#define PWM_MIN_PULSE_S                            ((float32)((TIMING_MIN_PULSE_US) * 1.0e-6f))

/* ========================= Pin assignment (validated TOM1 routing on KIT_A2G_TC387_5V_TFT) ========================= */
/* Three complementary pairs on TOM1 routed to Port 00 as per reference-valid mapping. */
#define PHASE_U_HS                                 IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS                                 IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS                                 IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS                                 IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS                                 IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS                                 IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* ========================= Public API ========================= */
void GTM_TOM_3PhaseInverterPWM_init(void);
void GTM_TOM_3PhaseInverterPWM_updateDuties(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
