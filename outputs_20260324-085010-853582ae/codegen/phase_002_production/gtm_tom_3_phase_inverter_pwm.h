/**********************************************************************************************************************
 * Module: GTM_TOM_3_Phase_Inverter_PWM
 * File:   gtm_tom_3_phase_inverter_pwm.h
 * Target: TC3xx (e.g., TC387)
 * Desc:   3-phase complementary center-aligned PWM using GTM TOM + PWMHL (production, iLLD-based)
 *
 * IMPORTANT:
 * - Generic headers only (no family-specific suffix headers)
 * - No watchdog handling in this driver (handled in CpuN_Main.c)
 * - Follows the SW Detailed Design API contract exactly
 *********************************************************************************************************************/
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm_PinMap.h"    /* Generic PinMap header */

/*==================================================================================================
 * Requirements-driven configuration values (DO NOT MODIFY WITHOUT REQUIREMENTS CHANGE)
 *================================================================================================*/
#define NUM_OF_CHANNELS               (3u)

/* Timing configuration */
#define PWM_FREQ_HZ                   (20000.0f)      /* TIMING_FREQUENCY_HZ = 20 kHz */
#define PWM_DEAD_TIME_US              (0.5f)          /* TIMING_DEADTIME_US */
#define PWM_MIN_PULSE_TIME_US         (1.0f)          /* TIMING_MIN_PULSE_US */

/* Duty pattern (fractions for ON-time computation from period) */
#define DUTY_25_PERCENT               (0.25f)
#define DUTY_50_PERCENT               (0.50f)
#define DUTY_75_PERCENT               (0.75f)
#define DUTY_STEP                     (0.10f)         /* step as fraction of period */
#define DUTY_MIN                      (0.10f)
#define DUTY_MAX                      (0.90f)

/* Port pad configuration (from requirements) */
#define PWM_OUTPUT_MODE               IfxPort_OutputMode_pushPull
#define PWM_PAD_DRIVER                IfxPort_PadDriver_cmosAutomotiveSpeed1

/*
 * TOM instance selection:
 * - Requirement preferred TOM1; however P02.* TOUTs are provided by TOM0 on TC387 packages.
 * - Selection per requirement rule: choose TOM instance matching P02.* => TOM0
 * - Keep CH0 as time base on that TOM instance.
 */
#define GTM_TOM_MASTER                IfxGtm_Tom_0
#define GTM_TOM_MASTER_TIMER_CH       IfxGtm_Tom_Ch_0

/*==================================================================================================
 * Pin assignments for KIT_A2G_TC387_5V_TFT (from requirements)
 * U: P02.0 (HS) / P02.7 (LS)
 * V: P02.1 (HS) / P02.4 (LS)
 * W: P02.2 (HS) / P02.5 (LS)
 *
 * NOTE: The following PinMap symbols must exist in IfxGtm_PinMap.h for TC387 (LFBGA-292/516).
 * Verified examples for P02.* indicate TOM0 mappings; if any symbol differs on your exact MCU/package,
 * update these macros to the matching IfxGtm_TOM0_*_TOUT*_P02_*_OUT entries.
 *================================================================================================*/
#define PHASE_U_HS   (&IfxGtm_TOM0_0_TOUT0_P02_0_OUT)    /* P02.0 */
#define PHASE_U_LS   (&IfxGtm_TOM0_0N_TOUT7_P02_7_OUT)   /* P02.7 */
#define PHASE_V_HS   (&IfxGtm_TOM0_1_TOUT1_P02_1_OUT)    /* P02.1 */
#define PHASE_V_LS   (&IfxGtm_TOM0_12_TOUT4_P02_4_OUT)   /* P02.4 */
#define PHASE_W_HS   (&IfxGtm_TOM0_10_TOUT2_P02_2_OUT)   /* P02.2 */
#define PHASE_W_LS   (&IfxGtm_TOM0_13_TOUT5_P02_5_OUT)   /* P02.5 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * API CONTRACT (exact signatures required by SW Detailed Design)
 */
void initGtmTomPwm(void);

/*
 * Name preserved as per API contract. To avoid collision with iLLD symbol of same name (different prototype),
 * this header maps the external reference to an internal implementation symbol in this module. The source
 * file handles unmapping to call the real iLLD API.
 */
#define IfxGtm_Tom_PwmHl_setOnTime UpdateGtmTomPwmDutyRamp
void IfxGtm_Tom_PwmHl_setOnTime(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
