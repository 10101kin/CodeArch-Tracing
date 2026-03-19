/*
 * gtm_tom_3_phase_inverter_pwm.h
 * GTM TOM 3-Phase Inverter PWM - Production Header
 *
 * Target: TC3xx (TC387)
 * Peripheral: GTM TOM (3-phase complementary PWM using unified IfxGtm_Pwm)
 *
 * Notes:
 * - This module exposes two public functions with EXACT names per SW Detailed Design:
 *     IfxGtm_Tom_PwmHl_init()
 *     IfxGtm_Tom_PwmHl_setDuty()
 * - Internally, it uses the unified high-level IfxGtm_Pwm driver configured for TOM
 *   to implement complementary outputs, dead-time via DTM, and synchronous updates.
 * - Watchdog disable is NOT included here (must be placed only in CpuN_Main.c).
 */

#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"  /* Generic header - auto-selects correct family */
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= Requirements-derived configuration ========================= */
#define PWM_FREQUENCY_HZ                          (20000.0f)   /* TIMING_PWM_FREQUENCY_HZ */
#define TIMING_DEADTIME_US                        (0.5f)       /* TIMING_DEADTIME_US */
#define TIMING_MIN_PULSE_US                       (1.0f)       /* TIMING_MIN_PULSE_US */
#define DUTY_UPDATE_POLICY_UPDATE_INTERVAL_MS     (500U)       /* DUTY_UPDATE_POLICY_UPDATE_INTERVAL_MS */
#define DUTY_UPDATE_POLICY_INCREMENT_PERCENT      (10.0f)      /* DUTY_UPDATE_POLICY_INCREMENT_PERCENT */
#define DUTY_UPDATE_POLICY_WRAP_0_TO_100          (1)          /* True */
#define TIMING_USE_HARDWARE_DTM                   (1)
#define TIMING_UPDATE_SYNCHRONOUS_TGC             (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ            (300U)
#define CLOCK_GTM_FXCLK0_FREQ_MHZ                 (100U)

/* Initial duty cycle percentages (percent representation 0.0..100.0) */
#define INITIAL_DUTY_PERCENT_U                    (25.0f)
#define INITIAL_DUTY_PERCENT_V                    (50.0f)
#define INITIAL_DUTY_PERCENT_W                    (75.0f)

/* ========================= Channel and pin mapping ========================= */
#define NUM_OF_CHANNELS                           (3U)

/* TOM1 mapping retained as per user's requirement (KIT_A2G_TC387_5V_TFT reference) */
#define PHASE_U_HS    (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT) /* U high-side: TOM1 CH2 -> P00.3 */
#define PHASE_U_LS    (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT) /* U low-side:  TOM1 CH1 -> P00.2 */
#define PHASE_V_HS    (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT) /* V high-side: TOM1 CH4 -> P00.5 */
#define PHASE_V_LS    (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT) /* V low-side:  TOM1 CH3 -> P00.4 */
#define PHASE_W_HS    (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT) /* W high-side: TOM1 CH6 -> P00.7 */
#define PHASE_W_LS    (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT) /* W low-side:  TOM1 CH5 -> P00.6 */

/* LED for ISR heartbeat (TC3xx example: P13.0) */
#define LED                              &MODULE_P13, 0
#define ISR_PRIORITY_TOM                 (10U)  /* Keep macro name EXACT per reference rule */

/* ========================= Public API (EXACT names per SW Detailed Design) ========================= */

/**
 * Initialize the GTM for 3-phase complementary, center-aligned PWM on TOM1 using IfxGtm_Pwm.
 * Behavior (from SW Detailed Design):
 * 1) Enable GTM and clocks
 * 2) Prepare unified IfxGtm_Pwm configuration for TOM, fxclk0, center-aligned, sync-start, sync-update
 * 3) Configure complementary outputs via OutputConfig[] (no direct PinMap calls), DTM for 0.5us
 * 4) Bind period event interrupt (optional) to channel 0 with ISR_PRIORITY_TOM
 * 5) Set initial duties 25/50/75 percent and start synchronously
 */
void IfxGtm_Tom_PwmHl_init(void);

/**
 * Runtime duty update function (EXACT name preserved):
 * - On each call, add +10 percentage points to U/V/W (wrap 0..100)
 * - Compute safeMin = (minPulse + 2*deadTime) / period, safeMax = 1 - safeMin
 * - Clamp each duty to [safeMin, safeMax]
 * - Apply via unified driver synchronous update (shadow-to-active at next period)
 */
void IfxGtm_Tom_PwmHl_setDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptGtmTom(void);
void IfxGtm_periodEventFunction(void *data);

}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
