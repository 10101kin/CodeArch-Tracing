#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxSrc.h"
#include "IfxWtu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===============================
 * Requirements-based definitions
 * =============================== */
#define EGTM_CLUSTER                          (0u)
#define EGTM_ATOM                             (0u)
#define EGTM_TRIGGER_CHANNEL                  (3u)
#define EGTM_ALIGNMENT_CENTER                 (1u)
#define EGTM_COMPLEMENTARY                    (1u)
#define EGTM_DEADTIME_US                      (1.0f)
#define EGTM_POLARITY_HIGH_SIDE               (1u)
#define EGTM_POLARITY_LOW_SIDE                (0u)

#define TIMING_PWM_FREQUENCY_HZ               (20000.0f)
#define TIMING_DEADTIME_US                    (1.0f)
#define TIMING_DUTY_UPDATE_PERIOD_MS          (500u)
#define TIMING_HEARTBEAT_PERIOD_MS            (500u)

#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ        (300u)
#define CLOCK_GTM_CMU_CLK_MHZ                 (100u)

/* Channel count definitions */
#define NUM_OF_CHANNELS                       (3u)
#define NUM_OF_ADC_TRIG_CHANNELS              (1u)

/* ISR priority macro (must use exact name pattern) */
#ifndef ISR_PRIORITY_ATOM
#define ISR_PRIORITY_ATOM                     (10u)
#endif

/* LED: P03.9 */
#define HEARTBEAT_LED_PORT                    (&MODULE_P03)
#define HEARTBEAT_LED_PIN                     (9u)

/* ===============================
 * TOUT pin selections (from requirements)
 * ATOM0 CH0..2 complementary pairs on P20.8..P20.13
 * Trigger: ATOM0 CH3 on P33.0
 * =============================== */
/* Phase U: P20.8 (HS), P20.9 (LS) */
#define EGTM_ATOM0_CH0_HS_TOUT                ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define EGTM_ATOM0_CH0_LS_TOUT                ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)

/* Phase V: P20.10 (HS), P20.11 (LS) */
#define EGTM_ATOM0_CH1_HS_TOUT                ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define EGTM_ATOM0_CH1_LS_TOUT                ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)

/* Phase W: P20.12 (HS), P20.13 (LS) */
#define EGTM_ATOM0_CH2_HS_TOUT                ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define EGTM_ATOM0_CH2_LS_TOUT                ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* Trigger export pin: ATOM0 CH3 -> P33.0 */
#define EGTM_ATOM0_CH3_TOUT                   ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_3_TOUT24_P33_0_OUT)

/* ===============================
 * Data structures
 * =============================== */
typedef struct {
    IfxEgtm_Pwm          pwm;                      /* PWM Driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];
    float32              dutyCycles[NUM_OF_CHANNELS];
    float32              phases[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];
} EgtmAtom3phInv;

typedef struct {
    IfxEgtm_Pwm          pwm;
    IfxEgtm_Pwm_Channel  channels[NUM_OF_ADC_TRIG_CHANNELS];
    float32              dutyCycles[NUM_OF_ADC_TRIG_CHANNELS];
    float32              phases[NUM_OF_ADC_TRIG_CHANNELS];
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_ADC_TRIG_CHANNELS];
} EgtmAtomTrigOut;

/* ===============================
 * Public API
 * =============================== */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
