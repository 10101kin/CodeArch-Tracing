#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

/**
 * GTM TOM 3-Phase Inverter PWM driver (TC3xx / TC387)
 * - Uses IfxGtm_Pwm unified multi-channel driver with a common IfxGtm_Tom_Timer timebase
 * - Six TOM1 channels mapped to board pins as requested
 * - Software dead-time applied when computing complementary LS duties
 * - Duty units are percent [0.0f .. 100.0f]
 */

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================
 * Configuration Macros (from requirements)
 * ============================= */
#define NUM_OF_CHANNELS                     (6U)

/* Timebase (GTM.TOM1) */
#define TIMEBASE_FREQUENCY_HZ               (20000.0f)   /* 20 kHz */
#define TIMEBASE_ALIGNMENT_CENTER           (1)          /* marker only; actual alignment set in IfxGtm_Pwm_Config */
#define TIMEBASE_CLOCK_SOURCE               (IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0)

/* Initial duties (percent) */
#define INITIAL_DUTY_PERCENT_U              (25.0f)
#define INITIAL_DUTY_PERCENT_V              (50.0f)
#define INITIAL_DUTY_PERCENT_W              (75.0f)

/* Update policy */
#define DUTY_UPDATE_POLICY_INTERVAL_MS      (500U)
#define DUTY_UPDATE_POLICY_STEP_PERCENT     (10.0f)

/* Software dead time (for LS = 100 - HS - deadTimePct) */
#define TIMING_SOFTWARE_DEAD_TIME_US        (0.5f)

/* PWM timing */
#define TIMING_PWM_FREQUENCY_HZ             (20000.0f)

/* Reference-style macros (percent representation) */
#define DUTY_25_PERCENT                     (25.0f)
#define DUTY_50_PERCENT                     (50.0f)
#define DUTY_75_PERCENT                     (75.0f)
#define DUTY_STEP                           (10.0f)

/* =============================
 * Pin Mapping (KIT_A2G_TC387_5V_TFT as requested)
 * Keep existing pin mapping and TOM submodule: TOM1 CH1..CH6
 * ============================= */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)  /* U HS: TOM1 CH2 -> TOUT12 P00.3 */
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)  /* U LS: TOM1 CH1 -> TOUT11 P00.2 */
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)  /* V HS: TOM1 CH4 -> TOUT14 P00.5 */
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)  /* V LS: TOM1 CH3 -> TOUT13 P00.4 */
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)  /* W HS: TOM1 CH6 -> TOUT16 P00.7 */
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)  /* W LS: TOM1 CH5 -> TOUT15 P00.6 */

/* =============================
 * Public API
 * ============================= */
/**
 * Initialize the GTM-based three-phase PWM inverter on TOM1 (channels 1..6):
 * 1) Enable GTM and FXCLK domain.
 * 2) Configure TOM1 timer as common timebase at 20 kHz, center-aligned via Fxclk0.
 * 3) Map six TOM1 channels to assigned pins, set output mode and pad driver.
 * 4) Initialize unified multi-channel PWM driver (IfxGtm_Pwm) for six channels bound to TOM1.
 * 5) Start TOM timer and enable/start synchronized PWM channels.
 * 6) Compute HS initial duties (U=25,V=50,W=75). LS = 100 - HS - deadTimePct (deadTimePct from 0.5us/Fxclk0/period), clamp [0..100].
 * 7) Apply all six initial duties atomically using immediate multi-channel update.
 */
void initGtmTomPwm(void);

/**
 * Periodic duty update (caller enforces 500 ms period):
 * 1) Read period ticks from TOM timebase.
 * 2) Increment HS duty of U,V,W by +10% and wrap in [0..100) by subtracting 100% if exceeded.
 * 3) Convert 0.5us dead-time to ticks using Fxclk0, derive deadTimePct = deadTicks/periodTicks*100.
 * 4) LS duty per phase = 100 - HS - deadTimePct, clamp to [0..100].
 * 5) Build six-element duty array in channel order and apply via IfxGtm_Pwm_updateChannelsDutyImmediate gated by timer update disable/apply.
 */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
