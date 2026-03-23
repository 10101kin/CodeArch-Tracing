/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * TC4xx (TC387 board target) - eGTM ATOM 3-Phase Inverter PWM + ADC Trigger
 *
 * Implements the API contract from SW Detailed Design using unified iLLD driver
 * IfxEgtm_Pwm. Follows mandatory initialization and configuration patterns.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxWtu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===============================
 * Requirement-driven configuration
 * =============================== */
#define EGTM_CLUSTER                        (0u)              /* EGTM Cluster 0 */
#define EGTM_ATOM                           (0u)              /* ATOM0 */
#define EGTM_TRIGGER_CHANNEL                (3u)              /* ATOM0.CH3 trigger */

#define TIMING_PWM_FREQUENCY_HZ             (20000.0f)        /* 20 kHz */
#define TIMING_DEADTIME_US                  (1.0f)            /* 1 us */
#define TIMING_HEARTBEAT_PERIOD_MS          (500u)            /* caller-controlled */
#define TIMING_DUTY_UPDATE_PERIOD_MS        (500u)            /* caller-controlled */

#define EGTM_ALIGNMENT_CENTER               (1u)              /* center-aligned */

/* GPIOs (from requirements) */
#define HEARTBEAT_LED_PORT                  (&MODULE_P03)
#define HEARTBEAT_LED_PIN                   (9u)              /* P03.9 */

/* Number of inverter PWM channels (U,V,W) */
#define NUM_OF_CHANNELS                     (3u)
#define NUM_OF_TRIG_CHANNELS                (1u)

/* Duty representation: Percent [0.0f .. 100.0f] */
#define DUTY_MIN_PERCENT                    (0.0f)
#define DUTY_MAX_PERCENT                    (100.0f)

/* ===============================
 * Pin routing macros (generic PinMap)
 * Use ATOM0 CH0..CH2 on P20.8..P20.13 as complementary pairs
 * High-side: active high; Low-side: active low (complementary)
 * =============================== */
/* Phase U: CH0: HS=P20.8, LS=P20.9 */
#define EGTM_ATOM0_CH0_HS   ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define EGTM_ATOM0_CH0_LS   ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
/* Phase V: CH1: HS=P20.10, LS=P20.11 */
#define EGTM_ATOM0_CH1_HS   ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define EGTM_ATOM0_CH1_LS   ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
/* Phase W: CH2: HS=P20.12, LS=P20.13 */
#define EGTM_ATOM0_CH2_HS   ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define EGTM_ATOM0_CH2_LS   ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* Trigger export: ATOM0.CH3 on P33.0 (50% edge-aligned) */
#define EGTM_ATOM0_CH3_TRIG ((IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM0_3_TOUT20_P33_0_OUT)

/* ===============================
 * Public API
 * =============================== */

/*
 * Initialize the eGTM for 3-phase complementary PWM and a 50% edge-aligned trigger.
 * Also configures TMADC trigger routing and heartbeat GPIO.
 */
void initEgtmAtom3phInv(void);

/*
 * Update the three phase duties immediately and toggle the heartbeat LED.
 * requestDuty: array[3] with percent values [0..100].
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
