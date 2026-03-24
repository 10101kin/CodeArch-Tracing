/*
 * egtm_atom_3ph_pwm.h
 *
 * Production module for TC4xx (TC4D7) implementing TMADC polled configuration
 * and LED GPIO setup, per SW Detailed Design. No eGTM PWM configuration is
 * performed in this module.
 */
#ifndef EGTM_ATOM_3PH_PWM_H
#define EGTM_ATOM_3PH_PWM_H

#include "Ifx_Types.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =====================================================================
 * REQUIREMENTS-DRIVEN CONFIGURATION MACROS (from requirements JSON)
 * ===================================================================== */
#define EGTM_CLUSTER                             (0U)
#define EGTM_ATOM                                (0U)
#define EGTM_TRIGGER_OUTPUT_CHANNEL              (3U)
/* Note: P33.0 is the observable trigger output; PWM/trigger routing is out of scope here */
#define EGTM_TRIGGER_OUTPUT_PIN_PORT             (MODULE_P33)
#define EGTM_TRIGGER_OUTPUT_PIN_INDEX            (0U)
/* falling edge trigger, 50% duty - routed from eGTM ATOM0 CH3, configured elsewhere */

#define LED_PIN_PORT                             (MODULE_P03)
#define LED_PIN_INDEX                            (9U)
/* LED is active low; set initial state to inactive (high) */
#define LED_ACTIVE_LEVEL_LOW                     (1U)
#define LED_TOGGLE_PERIOD_MS                     (500U)

#define TMADC0_TRIGGER_SOURCE_DESCRIPTION        "EGTM.CLC0.ATOM0.CH3"
#define TMADC0_CONVERSION_MODE_STR               "queued_scan_on_trigger"
#define TMADC0_CHANNELS_PER_PERIOD               (5U)
#define TMADC0_SERVICING_POLLED                  (1U)
#define TMADC0_ISR_ENABLED                       (0U)

#define TIMING_PWM_FREQUENCY_HZ                  (20000.0f)
#define TIMING_CENTER_ALIGNED                    (1U)
#define TIMING_DEADTIME_US                       (1.0f)
#define TIMING_DUTY_UPDATE_PERIOD_MS             (500U)

#define CLOCK_REQUIRES_XTAL                      (1U)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ           (300U)

/* =====================================================================
 * PUBLIC API (from SW Detailed Design)
 * ===================================================================== */

/**
 * Initialize only peripherals present on TC4D7 per SW Detailed Design:
 * 1) Optional system timebase (e.g., WUT) is intentionally NOT configured here.
 * 2) Enable ADC and configure TMADC in queued/polled mode: initialize the module
 *    and channels (ADC0.CH0..CH4) for a 5-channel sequence; start TMADC in polled mode.
 *    External trigger source is eGTM ATOM0.CH3 (20 kHz falling edge, 50% duty),
 *    which is configured outside of this module.
 * 3) Configure LED GPIO P03.9 as push-pull output and set initial inactive state (high).
 * 4) No eGTM PWM (3-phase inverter) configuration is performed in this module.
 *
 * Note:
 * - This module must not contain watchdog disable/enable code.
 * - TMADC interrupt is not used (polled mode per requirements).
 */
void initEgtmAtom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3PH_PWM_H */
