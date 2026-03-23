#ifndef EGTM_ATOM_TMADC_3PH_H
#define EGTM_ATOM_TMADC_3PH_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/* Configuration values from requirements (HIGH PRIORITY) */
/*============================================================================*/
#define PWM_CLUSTER                        (0U)
#define PWM_ATOM                           (0U)
#define PWM_TRIGGER_ATOM_CH                (3U)

#define PWM_TRIGGER_PIN_PORT               (&MODULE_P33)
#define PWM_TRIGGER_PIN_INDEX              (0U)

#define PWM_DTM_ENABLED                    (1)
#define PWM_DTM_DEADTIME_US                (1.0f)

#define ADC_TYPE_TMADC                     (1)
#define ADC_MODULE_INDEX                   (0U)

#define GPIO_LED_PORT                      (&MODULE_P03)
#define GPIO_LED_PIN_INDEX                 (9U)

#define TIMING_PWM_FREQUENCY_HZ            (20000.0f)
#define TIMING_DEADTIME_US                 (1.0f)
#define TIMING_GTM_REFERENCE_CLOCK_HZ      (100000000UL)
#define TIMING_PWM_PERIOD_TICKS            (5000UL)
#define TIMING_DEADTIME_TICKS              (100UL)
#define TIMING_SYNC_START                  (1)
#define TIMING_STM0_MAIN_LOOP_PERIOD_MS    (500UL)
#define CLOCK_REQUIRES_XTAL                (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ     (300UL)

/* Initial phase duties (percent representation) */
#define PHASE_U_DUTY                       (25.0f)
#define PHASE_V_DUTY                       (50.0f)
#define PHASE_W_DUTY                       (75.0f)

/* Number of inverter PWM channels (U, V, W) */
#define NUM_OF_CHANNELS                    (3U)

/* Public API (from SW Detailed Design) */
void initEgtmAtom3phInv(void);
void startEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_TMADC_3PH_H */
