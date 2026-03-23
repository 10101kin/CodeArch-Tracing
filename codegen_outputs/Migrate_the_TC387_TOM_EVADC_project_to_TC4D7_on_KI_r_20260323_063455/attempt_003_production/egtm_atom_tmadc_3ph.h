/*
 * egtm_atom_tmadc_3ph.h
 * TC4xx (TC4D7/TC387 target family) - eGTM ATOM 3-phase inverter PWM + TMADC trigger
 *
 * Public API per SW Detailed Design.
 */
#ifndef EGTM_ATOM_TMADC_3PH_H
#define EGTM_ATOM_TMADC_3PH_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Requirements-driven configuration values (do not change) */
#define PWM_CLUSTER                         (0)                 /* Cluster 0 */
#define PWM_ATOM                            (0)                 /* ATOM0 */
#define PWM_TRIGGER_ATOM_CH                 (3)                 /* ATOM0.CH3 as trigger */

#define TIMING_PWM_FREQUENCY_HZ             (20000.0f)         /* 20 kHz */
#define TIMING_DEADTIME_US                  (1.0f)             /* 1 us dead-time */
#define TIMING_GTM_REFERENCE_CLOCK_HZ       (100000000u)       /* 100 MHz reference */
#define TIMING_PWM_PERIOD_TICKS             (5000u)            /* 100 MHz / 20 kHz = 5000 */
#define TIMING_DEADTIME_TICKS               (100u)             /* 1us @100MHz = 100 ticks */
#define TIMING_SYNC_START                   (1)                 /* TRUE */
#define TIMING_STM0_MAIN_LOOP_PERIOD_MS     (500u)             /* 500 ms */

#define ADC_TYPE_TMADC                      (1)
#define ADC_MODULE_INDEX                    (0)

/* GPIO requirements */
#define GPIO_LED_PORT                       (&MODULE_P03)
#define GPIO_LED_PIN_INDEX                  (9u)                /* P03.9 */

/* PWM duty presets in PERCENT (not fraction) */
#define PHASE_U_INIT_DUTY_PCT               (25.0f)
#define PHASE_V_INIT_DUTY_PCT               (50.0f)
#define PHASE_W_INIT_DUTY_PCT               (75.0f)

/* Channel sizing: 3 inverter phases + 1 trigger channel = 4 total */
#define NUM_OF_PHASES                       (3u)
#define NUM_OF_CHANNELS                     (4u)

/* Public API (preserve exact names/signatures) */
void initEgtmAtom3phInv(void);
void startEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_TMADC_3PH_H */
