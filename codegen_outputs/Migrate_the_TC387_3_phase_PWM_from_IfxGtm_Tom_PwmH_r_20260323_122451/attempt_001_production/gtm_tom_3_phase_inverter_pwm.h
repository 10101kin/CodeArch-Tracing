/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * Production 3-phase PWM module using IfxGtm_Pwm unified driver (TC3xx, TOM).
 * Implements SW Detailed Design API exactly.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===================== Requirements-derived macros ===================== */
#define NUM_OF_CHANNELS                 (3u)
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)   /* 20 kHz */
#define TIMING_SYNCHRONOUS_UPDATE       (TRUE)
#define TIMING_DUTY_UPDATE_PERIOD_MS    (500u)
#define TIMING_DUTY_STEP_PERCENT        (10.0f)
#define TIMING_DEADTIME_NS              (0u)
#define TIMING_MIN_PULSE_NS             (0u)
#define CLOCK_REQUIRES_XTAL             (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ  (300u)

/* Initial duties in percent (not fraction) */
#define DUTY_25_PERCENT                 (25.0f)
#define DUTY_50_PERCENT                 (50.0f)
#define DUTY_75_PERCENT                 (75.0f)
#define DUTY_STEP                       (TIMING_DUTY_STEP_PERCENT)

/* Shared time base description: GTM.TOM1 master CH0, synchronous updates via TGC */
#define SHARED_TIMEBASE_MODULE_TOM      (1u)  /* TOM1 */
#define SHARED_TIMEBASE_MASTER_CHANNEL  (0u)  /* CH0  */

/* ===================== Public API (from SW Detailed Design) ===================== */

/*
 * Enable GTM and clocks, configure unified multi-channel PWM on TOM1 with shared time base (CH0).
 * Center-aligned 20 kHz, three single-ended channels on CH2/CH4/CH6. Map pins, initialize driver,
 * queue initial duties (25/50/75 %), start synchronized channels. Persistent state is kept
 * internally for runtime duty updates.
 */
void initGtmTomPwm(void);

/*
 * Increment each channel's duty by +10 percentage points; wrap to 0 when reaching 100.
 * Apply the updated array via multi-channel duty update to take effect synchronously
 * at the next PWM period-match.
 */
void updateGtmTomPwmDutyCycles(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
