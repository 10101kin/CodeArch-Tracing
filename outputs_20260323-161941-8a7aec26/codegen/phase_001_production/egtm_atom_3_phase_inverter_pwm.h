/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Production driver: EGTM ATOM 3-Phase Inverter PWM (TC4xx)
 *
 * Implements unified IfxEgtm_Pwm high-level driver initialization and duty update
 * according to the SW Detailed Design and mandatory iLLD patterns.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=============================
 * Configuration macros (from requirements)
 *=============================*/
#define NUM_OF_CHANNELS                 (3U)

/* Timing and behavior requirements */
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)   /* 20 kHz */
#define TIMING_CENTER_ALIGNED           (1)          /* TRUE */
#define TIMING_DEADTIME_US              (0.5f)       /* 0.5 us */
#define TIMING_MIN_PULSE_US             (1.0f)       /* 1.0 us */

/* Polarity requirements */
#define POLARITY_HIGH_SIDE_ACTIVE_HIGH  (1)
#define POLARITY_LOW_SIDE_ACTIVE_HIGH   (1)

/* Clock requirements (informational) */
#define CLOCK_REQUIRES_XTAL             (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ  (300U)

/* Runtime parameter macros (percent representation expected by unified driver) */
#define PWM_MIN_PULSE_TIME              (1.0f)
#define DUTY_25_PERCENT                 (25.0f)
#define DUTY_50_PERCENT                 (50.0f)
#define DUTY_75_PERCENT                 (75.0f)
#define DUTY_STEP                       (10.0f)
#define DUTY_MIN                        (10.0f)
#define DUTY_MAX                        (90.0f)

/* ISR priority macro (keep exact name pattern) */
#define ISR_PRIORITY_ATOM               (20)

/* LED for ISR diagnostic (TC4xx example: P03.9) */
#define LED                             &MODULE_P03, 9

/*=============================
 * Resource assumptions (from requirements)
 *=============================*/
#define RESOURCE_ASSUMPTIONS_EGTM_MODULE   MODULE_EGTM
/* Using ATOM1 for all six outputs (U/V/W high+low) as per requirements */

/*=============================
 * Phase pin assignment (KIT_A3G_TC4D7_LITE mapping; confirm in IfxEgtm_PinMap.h)
 * Note: Use generic PinMap header only (no family-specific headers)
 *=============================*/
/* Phase U: TOUT12 (P00.3) high-side, TOUT11 (P00.2) low-side */
#define PHASE_U_HS   (&IfxEgtm_ATOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM1_1_TOUT11_P00_2_OUT)

/* Phase V: TOUT14 (P00.5) high-side, TOUT13 (P00.4) low-side */
#define PHASE_V_HS   (&IfxEgtm_ATOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM1_3_TOUT13_P00_4_OUT)

/* Phase W: TOUT16 (P00.7) high-side, TOUT15 (P00.6) low-side */
#define PHASE_W_HS   (&IfxEgtm_ATOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM1_5_TOUT15_P00_6_OUT)

/*=============================
 * Public API (from SW Detailed Design)
 *=============================*/

/* Initialize the eGTM-based complementary, center-aligned 3-phase PWM for an inverter */
void initEgtmAtom3phInv(void);

/* Update the inverter PWM duty commands synchronously across all channels */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
