/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * TC4xx (TC4D7/TC387) eGTM ATOM 3-phase inverter PWM - Production Header
 *
 * This module configures a 3-phase complementary, center-aligned PWM using the
 * unified IfxEgtm_Pwm high-level driver on a single ATOM instance.
 *
 * Follows mandatory initialization patterns and API signatures from the
 * authoritative iLLD documentation and SW Detailed Design.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"  /* Generic PinMap header (no family-specific suffixes) */
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Configuration constants from requirements                                  */
/* ========================================================================== */

/* Number of complementary PWM pairs (3-phase inverter) */
#define NUM_OF_CHANNELS             (3U)

/* Timing requirements */
#define TIMING_PWM_FREQUENCY_HZ     (20000.0f)   /* 20 kHz */
#define TIMING_DEADTIME_US          (0.5f)       /* 0.5 microseconds */
#define TIMING_MIN_PULSE_US         (1.0f)       /* 1.0 microseconds */

/* Runtime duty stepping macros (names preserved from reference patterns) */
#define PWM_MIN_PULSE_TIME          (1.0f)       /* microseconds - matches TIMING_MIN_PULSE_US */
#define DUTY_25_PERCENT             (25.0f)
#define DUTY_50_PERCENT             (50.0f)
#define DUTY_75_PERCENT             (75.0f)
#define DUTY_STEP                   (0.10f)      /* 10% per update step (example ramp) */
#define DUTY_MIN                    (10.0f)      /* 10% - not used directly; clamp computed from min pulse */
#define DUTY_MAX                    (90.0f)      /* 90% - not used directly; clamp computed from min pulse */

/* ISR configuration (macro name preserved from reference) */
#define ISR_PRIORITY_ATOM           (20)

/* LED for ISR diagnostic toggle (TC4xx example: P03.9). Keep macro as two args. */
#define LED                         &MODULE_P03, 9

/* ========================================================================== */
/* Phase pin mapping (KIT_A3G_TC4D7_LITE target assumption - confirm in PinMap)*/
/* Using ATOM1 channels mapped to TOUT12..TOUT11, TOUT14..TOUT13, TOUT16..TOUT15 */
/* Final pin mapping confirmation = TBD per project hardware.                  */
/* ========================================================================== */
#define PHASE_U_HS   (&IfxEgtm_ATOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM1_5_TOUT15_P00_6_OUT)

/* ========================================================================== */
/* Public API                                                                 */
/* ========================================================================== */

/* Initialize the eGTM-based complementary, center-aligned 3-phase PWM */
void initEgtmAtom3phInv(void);

/* Update all three phase duties synchronously with min-pulse clamping */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
