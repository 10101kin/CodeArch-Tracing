#ifndef EGTM_ATOM_TMADC_3PH_H
#define EGTM_ATOM_TMADC_3PH_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration values from requirements */
#define PWM_CLUSTER                       (0u)
#define PWM_ATOM                          (0u)
#define PWM_TRIGGER_ATOM_CH               (3u)
#define GPIO_LED_PORT                     (&MODULE_P03)
#define GPIO_LED_PIN                      (9u)
#define PWM_TRIGGER_PORT                  (&MODULE_P33)
#define PWM_TRIGGER_PIN                   (0u)
#define PWM_FREQUENCY_HZ                  (20000.0f)
#define TIMING_GTM_REFERENCE_CLOCK_HZ     (100000000.0f)
#define TIMING_PWM_PERIOD_TICKS           (5000u)
#define TIMING_DEADTIME_US                (1.0f)
#define TIMING_DEADTIME_TICKS             (100u)
#define TIMING_SYNC_START                 (TRUE)
#define TIMING_STM0_MAIN_LOOP_PERIOD_MS   (500u)
#define CLOCK_REQUIRES_XTAL               (TRUE)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ    (300u)

/* PWM channel counts */
#define NUM_OF_INV_CHANNELS               (3u)   /* U,V,W complementary phase pairs */
#define NUM_OF_CHANNELS                   (4u)   /* U,V,W + dedicated trigger channel */

/* Initial phase duties in percent */
#define PHASE_U_DUTY                      (25.0f)
#define PHASE_V_DUTY                      (50.0f)
#define PHASE_W_DUTY                      (75.0f)
#define ADC_TRIG_DUTY                     (50.0f)

/* Public API (from SW Detailed Design) */
void initEgtmAtom3phInv(void);
void startEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_TMADC_3PH_H */
