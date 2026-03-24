/*
 * egtm_atom_3ph_pwm.h
 *
 * Production module: TC4xx (TC4D7/TC387 compatible subset)
 *
 * This header exposes the initialization API required by the SW Detailed Design.
 * It configures TMADC0 in queued/polled mode and the LED GPIO. No eGTM PWM
 * configuration is performed in this module per the design contract.
 */
#ifndef EGTM_ATOM_3PH_PWM_H
#define EGTM_ATOM_3PH_PWM_H

#include "Ifx_Types.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"

/* ===================== Requirements-driven constants (do NOT change) ===================== */
#define EGTM_CLUSTER                          (0u)
#define EGTM_ATOM                             (0u)
#define EGTM_TRIGGER_OUTPUT_CHANNEL           (3u)
#define EGTM_TRIGGER_OUTPUT_PIN_PORT          P33
#define EGTM_TRIGGER_OUTPUT_PIN_INDEX         (0u)
#define EGTM_TRIGGER_OUTPUT_EDGE_FALLING      (1u)
#define EGTM_TRIGGER_OUTPUT_DUTY_POSITION     (50.0f)     /* percent */
#define EGTM_TRIGGER_OUTPUT_PURPOSE           "TMADC0 external trigger"

#define LED_PIN_PORT                          P03
#define LED_PIN_INDEX                         (9u)
#define LED_ACTIVE_LEVEL_LOW                  (1u)        /* active-low LED */
#define LED_TOGGLE_PERIOD_MS                  (500u)

#define TMADC0_CHANNELS_PER_PERIOD            (5u)
#define TMADC0_CONVERSION_MODE_QUEUED_SCAN_ON_TRIGGER   (1u)
#define TMADC0_SERVICING_POLLED               (1u)
#define TMADC0_ISR                            (0u)

#define TIMING_PWM_FREQUENCY_HZ               (20000u)
#define TIMING_CENTER_ALIGNED                 (1u)
#define TIMING_DEADTIME_US                    (1.0f)
#define TIMING_DUTY_UPDATE_PERIOD_MS          (500u)
#define CLOCK_REQUIRES_XTAL                   (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ        (300u)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize only peripherals present on TC4D7 per SW Detailed Design:
 * 1) (Optional) System timebase via supported TC4Dx resources (not used here)
 * 2) Enable and configure TMADC0 in queued/polled mode with 5 channels
 * 3) Configure LED P03.9 as push-pull output and set initial inactive (high) level
 * 4) No eGTM PWM configuration in this module
 */
void initEgtmAtom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3PH_PWM_H */
