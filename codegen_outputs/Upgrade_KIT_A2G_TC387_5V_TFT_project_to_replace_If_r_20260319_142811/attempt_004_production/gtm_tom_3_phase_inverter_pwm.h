/*
 * GTM TOM 3-Phase Inverter PWM - Production Header
 * Target: AURIX TC3xx (TC387)
 * Peripheral: GTM TOM PWM (complementary, center-aligned) using IfxGtm_Tom_PwmHl
 *
 * This module provides two production entry points with EXACT signatures required
 * by SW Detailed Design:
 *   - IfxGtm_Tom_PwmHl_init()
 *   - IfxGtm_Tom_PwmHl_setDuty()
 *
 * DO NOT place watchdog code here. Watchdog handling belongs in CpuN_Main.c
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm_PinMap.h"   /* Generic pin map header (not family-specific) */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================== REQUIREMENTS CONSTANTS ========================== */
/* From requirements JSON (must use these exact values) */
#define INITIAL_DUTY_PERCENT_U                      (25.0f)
#define INITIAL_DUTY_PERCENT_V                      (50.0f)
#define INITIAL_DUTY_PERCENT_W                      (75.0f)
#define DUTY_UPDATE_POLICY_UPDATE_INTERVAL_MS       (500u)
#define DUTY_UPDATE_POLICY_INCREMENT_PERCENT        (10.0f)
#define DUTY_UPDATE_POLICY_WRAP_0_TO_100            (1u)
/* clamp_to_minPulse_and_deadtime policy */
#define TIMING_PWM_FREQUENCY_HZ                     (20000.0f)
#define TIMING_DEADTIME_US                          (0.5f)
#define TIMING_USE_HARDWARE_DTM                     (1u)
#define TIMING_MIN_PULSE_US                         (1.0f)
#define TIMING_UPDATE_SYNCHRONOUS_TGC               (1u)
#define CLOCK_REQUIRES_XTAL                         (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ              (300.0f)
#define CLOCK_GTM_FXCLK0_FREQ_MHZ                   (100.0f)

/* Reference macro names used in patterns (percent representation) */
#define PWM_MIN_PULSE_TIME                          (1.0f)      /* us, reference-only name */
#define DUTY_25_PERCENT                             (25.0f)
#define DUTY_50_PERCENT                             (50.0f)
#define DUTY_75_PERCENT                             (75.0f)
#define DUTY_STEP                                   (10.0f)     /* percent step per update */

/* ============================ HARDWARE SELECTION ============================ */
/* 3-phase complementary outputs on TOM1 as per project requirement
   U: TOM1 CH2/P00.3 & CH1/P00.2
   V: TOM1 CH4/P00.5 & CH3/P00.4
   W: TOM1 CH6/P00.7 & CH5/P00.6
   Use generic PinMap symbols (valid for TC3xx). */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

#define NUM_OF_CHANNELS                             (3u)

/* ============================= PUBLIC INTERFACE ============================ */
/* EXACT function signatures required by SW Detailed Design */
void IfxGtm_Tom_PwmHl_init(void);
void IfxGtm_Tom_PwmHl_setDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
