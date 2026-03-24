#ifndef EGTM_ATOM_3PH_PWM_H
#define EGTM_ATOM_3PH_PWM_H

#include "Ifx_Types.h"
#include "IfxPort.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"

/* ========================================================================
 * Requirements-driven configuration values (do NOT change)
 * ======================================================================== */
#define EGTM_CLUSTER                          (0)      /* From requirements */
#define EGTM_ATOM                             (0)      /* From requirements */
#define EGTM_TRIGGER_OUTPUT_CHANNEL           (3)      /* From requirements */
/* P33.0 trigger observation pin (PWM not configured in this module) */
#define EGTM_TRIGGER_OUTPUT_PIN_PORT          (&MODULE_P33)
#define EGTM_TRIGGER_OUTPUT_PIN_INDEX         (0u)
#define EGTM_TRIGGER_OUTPUT_EDGE_FALLING      (1)
#define EGTM_TRIGGER_OUTPUT_DUTY_POSITION     (50.0f)
#define EGTM_TRIGGER_OUTPUT_PURPOSE           "TMADC0 external trigger"

#define LED_PIN_PORT                          (&MODULE_P03)
#define LED_PIN_INDEX                         (9u)
#define LED_ACTIVE_LEVEL_LOW                  (1)      /* LED active low */
#define LED_TOGGLE_PERIOD_MS                  (500u)

#define TMADC0_TRIGGER_SOURCE                 "EGTM.CLC0.ATOM0.CH3" /* informational */
#define TMADC0_CONVERSION_MODE                "queued_scan_on_trigger"
#define TMADC0_CHANNELS_PER_PERIOD            (5u)
#define TMADC0_SERVICING_POLLED               (1)
#define TMADC0_ISR_ENABLED                    (0)

#define TIMING_PWM_FREQUENCY_HZ               (20000.0f)
#define TIMING_CENTER_ALIGNED                 (1)
#define TIMING_DEADTIME_US                    (1.0f)
#define TIMING_DUTY_UPDATE_PERIOD_MS          (500u)
#define CLOCK_REQUIRES_XTAL                   (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ        (300u)

/* Initial phase duties for user behavioral requirements (informational, PWM not configured here) */
#define PHASE_U_DUTY                          (25.0f)
#define PHASE_V_DUTY                          (50.0f)
#define PHASE_W_DUTY                          (75.0f)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize only peripherals present on TC4D7 according to SW Detailed Design.
 * Behavior:
 *  1) Optionally configure a system timebase using supported TC4Dx resources (not in this module).
 *  2) Enable ADC module and configure TMADC in queued/polled mode: prepare a queue of required
 *     channels, initialize their configurations, and start TMADC in polled (non-interrupt) mode.
 *  3) Configure the LED GPIO as a push-pull output and set its initial inactive level (LED off).
 *  4) Any PWM generation for a 3-phase inverter must use TC4Dx-supported timer modules and is
 *     not configured here.
 */
void initEgtmAtom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3PH_PWM_H */
