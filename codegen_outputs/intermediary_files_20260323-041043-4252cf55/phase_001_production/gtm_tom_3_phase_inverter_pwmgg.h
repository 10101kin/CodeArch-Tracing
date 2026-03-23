/*
 * gtm_tom_3_phase_inverter_pwmgg.h
 *
 * Production PWM driver: GTM TOM 3-phase inverter using IfxGtm_Pwm (TC3xx)
 * - Single-ended PWM on TOM1 CH2/4/6 with shared timebase TOM1 CH0
 * - Center-aligned 20 kHz, synchronous shadow-update enabled
 * - Runtime duty update: +10 percentage points every call, wrap at 100% -> 0%
 *
 * This header exposes the public API defined by the SW Detailed Design.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWMGG_H
#define GTM_TOM_3_PHASE_INVERTER_PWMGG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Ifx_Types.h"

/* =============================
 * Requirements-driven constants
 * ============================= */
#define NUM_OF_CHANNELS                 (3u)

/* Timing requirements */
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)        /* 20 kHz */
#define TIMING_SYNCHRONOUS_UPDATE       (1u)              /* TRUE */
#define TIMING_DUTY_UPDATE_PERIOD_MS    (500u)
#define TIMING_DUTY_STEP_PERCENT        (10.0f)
#define TIMING_DEADTIME_NS              (0u)
#define TIMING_MIN_PULSE_NS             (0u)

/* Clock/system requirements (informational) */
#define CLOCK_REQUIRES_XTAL             (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ  (300u)

/* Shared timebase selection (informational macros) */
#define SHARED_TIMEBASE_MODULE_STR              "GTM.TOM1"
#define SHARED_TIMEBASE_MASTER_CHANNEL_INDEX    (0u)

/* Initial duties in percent (per requirements) */
#define DUTY_INIT_U_PERCENT             (25.0f)
#define DUTY_INIT_V_PERCENT             (50.0f)
#define DUTY_INIT_W_PERCENT             (75.0f)

/* Runtime step and range (percent) */
#define DUTY_STEP_PERCENT               (TIMING_DUTY_STEP_PERCENT)
#define DUTY_MIN_PERCENT                (0.0f)
#define DUTY_MAX_PERCENT                (100.0f)

/* Deadtime/min pulse derived from requirements (single-ended, disabled) */
#define PWM_DEADTIME_SEC                (0.0f)
#define PWM_MIN_PULSE_SEC               (0.0f)

/* =============================
 * Pin assignments (validated TC3xx)
 * Single-ended outputs on TOM1 CH2/4/6 as proposed by user
 * ============================= */
#include "IfxGtm_PinMap.h"  /* Generic header per architecture rule */

#define PHASE_U_TOM     (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)  /* U: TOM1 CH2 -> P00.3 */
#define PHASE_V_TOM     (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)  /* V: TOM1 CH4 -> P00.5 */
#define PHASE_W_TOM     (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)  /* W: TOM1 CH6 -> P00.7 */

/* =============================
 * Public API (SW Detailed Design)
 * ============================= */

/**
 * Initialize (on first call) a unified multi-channel PWM configuration using IfxGtm_Pwm
 * and subsequently update the three channel duties each call by +10 percentage points
 * with wrap at 100% back to 0%. Initialization configures:
 *  - GTM module and CMU clocks
 *  - Three single-ended TOM1 channels (CH2/CH4/CH6) using TOM1 CH0 shared timebase
 *  - Center-aligned 20 kHz, synchronous update enabled
 *  - Initial duties: 25%, 50%, 75%
 */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWMGG_H */
