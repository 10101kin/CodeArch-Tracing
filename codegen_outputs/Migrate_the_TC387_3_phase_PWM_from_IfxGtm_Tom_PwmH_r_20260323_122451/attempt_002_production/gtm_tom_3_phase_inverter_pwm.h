/*
 * GTM TOM 3-Phase Inverter PWM - Unified IfxGtm_Pwm driver (TC3xx)
 *
 * This module initializes three single-ended PWM channels on GTM TOM1 using a
 * shared time base (master TOM1 CH0) at 20 kHz, center-aligned, with synchronous
 * updates via TGC. It starts with duty cycles 25%/50%/75% and provides a runtime
 * update API that steps each by +10 percentage points, wrapping at 100% to 0%.
 *
 * Requirements enforced here:
 * - SHARED_TIMEBASE_MODULE = GTM.TOM1
 * - SHARED_TIMEBASE_MASTER_CHANNEL = 0
 * - TIMING_PWM_FREQUENCY_HZ = 20000
 * - TIMING_SYNCHRONOUS_UPDATE = TRUE
 * - TIMING_DUTY_UPDATE_PERIOD_MS = 500 (timing done by caller)
 * - TIMING_DUTY_STEP_PERCENT = 10
 * - TIMING_DEADTIME_NS = 0 (no dead-time)
 * - TIMING_MIN_PULSE_NS = 0 (no min pulse constraint)
 * - CLOCK_EXPECTED_SYSTEM_FREQ_MHZ = 300 (SCU handled externally)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"
#include "IfxGtm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Channel count for this module */
#define NUM_OF_CHANNELS              (3u)

/* Requirement-backed configuration values */
#define SHARED_TIMEBASE_MODULE_TOM   (1u)         /* TOM1 */
#define SHARED_TIMEBASE_MASTER_CH    (0u)         /* CH0 time base */
#define TIMING_PWM_FREQUENCY_HZ      (20000.0f)   /* 20 kHz */
#define TIMING_SYNCHRONOUS_UPDATE    (1u)
#define TIMING_DUTY_UPDATE_PERIOD_MS (500u)
#define TIMING_DUTY_STEP_PERCENT     (10.0f)
#define TIMING_DEADTIME_NS           (0.0f)
#define TIMING_MIN_PULSE_NS          (0.0f)

/* Runtime parameter macros (names preserved from reference) */
#define PWM_MIN_PULSE_TIME           (0.0f)
#define DUTY_25_PERCENT              (25.0f)
#define DUTY_50_PERCENT              (50.0f)
#define DUTY_75_PERCENT              (75.0f)
#define DUTY_STEP                    (10.0f)
#define DUTY_MIN                     (0.0f)
#define DUTY_MAX                     (100.0f)

/*
 * Pin assignments (single-ended outputs; former low-side pins unused)
 * Mapping per user proposal (validated for TC3xx family):
 *  U = P00.3  (TOM1 CH2 -> TOUT12)
 *  V = P00.5  (TOM1 CH4 -> TOUT14)
 *  W = P00.7  (TOM1 CH6 -> TOUT16)
 */
#define PHASE_U_TOM_TOUT   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_V_TOM_TOUT   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_W_TOM_TOUT   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)

/* Public API */

/**
 * Initialize GTM TOM 3-phase inverter PWM (three single-ended channels).
 * - Enables GTM and functional clocks
 * - Configures unified IfxGtm_Pwm for TOM1 center-aligned @ 20 kHz
 * - Assigns three synchronized channels and routes pins
 * - Initializes driver and applies initial duties (25/50/75 %)
 * - Starts synced channels (updates applied on period-match)
 */
void initGtmTomPwm(void);

/**
 * Runtime duty update:
 * - Increment each channel by +10 percentage points
 * - If any reaches >= 100%, wrap to 0%
 * - Apply via multi-channel duty update (shadowed; sync at period)
 */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
