#ifndef EGTM_ATOM_3_PHASE_INVERTER_H
#define EGTM_ATOM_3_PHASE_INVERTER_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxWtu.h"
#include "IfxPort.h"
#include "IfxSrc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= Requirements-derived configuration ========================= */
#define PWM_OUTPUTS_ALIGNMENT                 IfxEgtm_Pwm_Alignment_center
#define PWM_OUTPUTS_DEADTIME_US              (1.0f)
#define PWM_OUTPUTS_DTM_MODE                 /* CDTM required - configured via DTM API */
#define PWM_OUTPUTS_FREQUENCY_HZ             (20000.0f)

#define PWM_INITIAL_DUTY_U_PERCENT           (25.0f)
#define PWM_INITIAL_DUTY_V_PERCENT           (50.0f)
#define PWM_INITIAL_DUTY_W_PERCENT           (75.0f)

#define ADC_TRIGGER_EDGE_ALIGNED_DUTY        (50.0f)

#define LED_PORT                             (&MODULE_P03)
#define LED_PIN_IDX                          (9u)
#define LED_LOW_ACTIVE                       (1u)
#define LED_DEFAULT_LEVEL_HIGH               (1u)

#define TIMING_PWM_FREQUENCY_HZ              (20000.0f)
#define TIMING_DEADTIME_RISE_US              (1.0f)
#define TIMING_DEADTIME_FALL_US              (1.0f)
#define TIMING_DUTY_UPDATE_INTERVAL_MS       (500u)
#define TIMING_DTM_CLOCK_ASSUMPTION_MHZ      (100.0f)
#define TIMING_DEADTIME_TOLERANCE_US         (0.1f)

/* Number of channels used in the unified driver: 3 inverter phases + 1 ADC trigger */
#define EGTM_ATOM_INV_NUM_CHANNELS           (4u)

/* ISR priority macro - use EXACT name as per reference. Provide a default if not externally defined. */
#ifndef ISR_PRIORITY_ATOM
#define ISR_PRIORITY_ATOM                    (10)
#endif

/* Public API (EXACT signatures from SW Detailed Design) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_H */
