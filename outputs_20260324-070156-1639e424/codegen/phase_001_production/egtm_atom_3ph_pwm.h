#ifndef EGTM_ATOM_3PH_PWM_H
#define EGTM_ATOM_3PH_PWM_H

#include "Ifx_Types.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * Requirements-derived configuration values (do not change)
 * ============================================================================= */
#define EGTM_CLUSTER                          (0)
#define EGTM_ATOM                             (0)
#define EGTM_TRIGGER_OUTPUT_CHANNEL           (3)
#define EGTM_TRIGGER_OUTPUT_DUTY_POSITION     (50)
/* P33.0: provided for reference; routing is done in board/system code */
#define EGTM_TRIGGER_OUTPUT_PORT              MODULE_P33
#define EGTM_TRIGGER_OUTPUT_PIN_INDEX         (0u)

#define LED_PORT                              MODULE_P03
#define LED_PIN_INDEX                         (9u)
#define LED_ACTIVE_LEVEL_LOW                  (1)

#define LED_TOGGLE_PERIOD_MS                  (500u)

#define TMADC0_TRIGGER_SOURCE_DESC            "EGTM.CLC0.ATOM0.CH3"
#define TMADC0_CONVERSION_MODE_DESC           "queued_scan_on_trigger"
#define TMADC0_CHANNELS_PER_PERIOD            (5u)
#define TMADC0_SERVICING_POLLED               (1)

#define TIMING_PWM_FREQUENCY_HZ               (20000.0f)
#define TIMING_CENTER_ALIGNED                 (1)
#define TIMING_DEADTIME_US                    (1.0f)
#define TIMING_DUTY_UPDATE_PERIOD_MS          (500u)

#define CLOCK_REQUIRES_XTAL                   (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ        (300u)

/* Initial phase duties as percent values */
#define PHASE_U_DUTY                          (25.0f)
#define PHASE_V_DUTY                          (50.0f)
#define PHASE_W_DUTY                          (75.0f)

/* =============================================================================
 * Public API
 * ============================================================================= */

/**
 * Initialize only peripherals present on TC4D7 per SW Detailed Design:
 * 1) Optionally configure a system timebase (not implemented here; use WUT in CPU code if needed)
 * 2) Enable ADC module and configure TMADC in queued/polled mode for 5 channels
 * 3) Configure LED GPIO (P03.9) as push-pull output, set initial inactive level (active-low LED -> drive high)
 * 4) No eGTM PWM configuration is performed in this module
 */
void initEgtmAtom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3PH_PWM_H */
