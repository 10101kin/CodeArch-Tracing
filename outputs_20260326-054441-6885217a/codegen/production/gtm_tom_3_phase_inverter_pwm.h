/***************************************************************************
 * @file    gtm_tom_3_phase_inverter_pwm.h
 * @brief   GTM TOM 3-Phase Inverter PWM driver (TC3xx)
 *
 * Production-ready implementation using iLLD unified PWM driver (IfxGtm_Pwm)
 * following SW Detailed Design behavior and API contract.
 *
 * Notes:
 * - Uses GTM TOM submodule with TOM1 as master time base (TOM1_CH0).
 * - Complementary outputs on P00.{2..7} with dead-time and min-pulse.
 * - Center-aligned PWM at 20 kHz; initial duties: U=25%, V=50%, W=75%.
 * - Clock source: Fxclk0 (IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0 requirement).
 * - Pin routing via IfxGtm_PinMap_setTomTout as per SW design.
 * - No watchdog handling here (must be in CpuN_Main.c only).
 ***************************************************************************/
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= Configuration macros (from requirements) ========================= */
#define NUM_OF_CHANNELS                 (3u)

/* Timing requirements */
#define PWM_FREQUENCY_HZ                (20000.0f)      /* TIMING_PWM_FREQUENCY_HZ */
#define PWM_DEAD_TIME_S                 (5.0e-07f)      /* TIMING_DEAD_TIME_SECONDS = 0.5us */
#define PWM_MIN_PULSE_S                 (1.0e-06f)      /* TIMING_MIN_PULSE_SECONDS = 1.0us */

/* Initial duty cycles in PERCENT (0.0f .. 100.0f) */
#define DUTY_U_INIT_PERCENT             (25.0f)         /* INITIAL_DUTY_CYCLE_PERCENT_U */
#define DUTY_V_INIT_PERCENT             (50.0f)         /* INITIAL_DUTY_CYCLE_PERCENT_V */
#define DUTY_W_INIT_PERCENT             (75.0f)         /* INITIAL_DUTY_CYCLE_PERCENT_W */

/* Runtime duty update behavior (percent domain) */
#define DUTY_STEP_PERCENT               (10.0f)         /* Increment step (10%) */
#define DUTY_MIN_PERCENT                (10.0f)         /* Lower bound (10%) */
#define DUTY_MAX_PERCENT                (90.0f)         /* Upper bound (90%) */

/* Output configuration (from requirements) */
#define OUTPUT_ACTIVE_STATE_CCX         Ifx_ActiveState_high
#define OUTPUT_ACTIVE_STATE_COUTX       Ifx_ActiveState_high
#define OUTPUT_PAD_DRIVER               IfxPort_PadDriver_cmosAutomotiveSpeed1
#define OUTPUT_MODE                     IfxPort_OutputMode_pushPull

/* Master time base (from requirements) */
#define GTM_TOM_MASTER                  IfxGtm_Tom_1
#define GTM_TOM_MASTER_TIMER_CH         IfxGtm_Tom_Ch_0

/* Public API */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);
void GTM_TOM_3_Phase_Inverter_PWM_updateUVW(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
