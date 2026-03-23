/**********************************************************************************************************************
 * Module: EGTM_ATOM_3_Phase_Inverter_PWM
 * File:   egtm_atom_3_phase_inverter_pwm.h
 * Brief:  TC4xx eGTM ATOM 3-phase inverter PWM (unified IfxEgtm_Pwm driver)
 *
 * This header exposes the public API required by SW Detailed Design.
 *
 * Requirements implemented:
 * - 3-phase inverter (U/V/W) using eGTM ATOM, center-aligned SOMP with complementary outputs via CDTM/DTM
 * - HS active-high, LS active-low
 * - Symmetric deadtime: 1.0 us rising/falling, DTM CLK0
 * - 20 kHz PWM, synchronous start and update on period event
 * - Period-event ISR on cpu0 with priority 20
 * - Duty updates by step in update function (caller controls timing via STM in Cpu0_Main)
 *
 * Notes:
 * - Watchdog handling must be done only in CpuN_Main.c files (NOT here)
 * - Use only generic iLLD headers (no family-suffixed pinmap headers)
 **********************************************************************************************************************/
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Configuration macros from requirements
 * ======================================================================== */
#define NUM_OF_CHANNELS             (3U)

/* Timing */
#define PWM_FREQUENCY               (20000.0f)   /* Hz: TIMING_PWM_FREQUENCY_HZ = 20000 */

/* Deadtime (symmetric, in seconds) */
#define DEADTIME_RISING_S           (1.0e-6f)    /* DEADTIME_RISING_US = 1.0 */
#define DEADTIME_FALLING_S          (1.0e-6f)    /* DEADTIME_FALLING_US = 1.0 */

/* Interrupt priority macro (EXACT name as required) */
#define ISR_PRIORITY_ATOM           (20U)

/* Initial duty in percent per requirements (0.0%) */
#define PHASE_U_DUTY                (0.0f)
#define PHASE_V_DUTY                (0.0f)
#define PHASE_W_DUTY                (0.0f)

/* Step for update function (percent). Caller controls timing cadence via STM in CpuN_Main */
#define PHASE_DUTY_STEP             (10.0f)

/* Status LED: default proposal P10.2, push-pull, active high */
#define LED                         &MODULE_P10, 2

/* ========================================================================
 * Target pin assignment (verify against IfxEgtm_PinMap for KIT_A3G_TC4D7_LITE)
 * Channel assignment: eGTM Cluster 1, ATOM1 CH0/1/2
 * Complementary outputs use the _N suffix pin symbols
 * ======================================================================== */
#include "IfxEgtm_PinMap.h"  /* Generic header per rule */

#define PHASE_U_HS   (&IfxEgtm_ATOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM1_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM1_2N_TOUT5_P02_5_OUT)

/* ========================================================================
 * Public API (EXACT signatures from SW Detailed Design)
 * ======================================================================== */
/** Initialize the eGTM-based 3-phase inverter PWM (see SW Detailed Design for steps). */
void initEgtmAtom3phInv(void);

/** Update duty cycles by PHASE_DUTY_STEP, wrapping at 100%, and queue synchronous update. */
void updateEgtmAtom3phInvDuty(void);

/** ISR for PWM period event on base channel (delegates to unified PWM interrupt handler). */
void interruptEgtmAtomPeriod(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
