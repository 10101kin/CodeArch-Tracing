/*
 * gtm_tom_3_phase_inverter_pwm.h
 *
 * 3-phase inverter PWM using unified IfxGtm_Pwm driver on AURIX TC3xx (TC387)
 *
 * This module configures three complementary PWM pairs (TOM1 on P00.2..P00.7)
 * at 20 kHz, center-aligned, with 0.5 us dead-time. Initial duties are
 * U=25%, V=50%, W=75%. A runtime step function increases each duty by +10%
 * with wrap-around 0..100% and updates all channels synchronously.
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"  /* Generic header; selects correct family internally */
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Requirements-derived timing/config macros */
#define TIMING_PWM_FREQUENCY_HZ          (20000.0f)       /* 20 kHz */
#define TIMING_DEAD_TIME_US              (0.5f)           /* 0.5 microseconds */
#define CLOCK_REQUIRES_XTAL              (1)              /* informational */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ   (300)

/* Runtime behavior macros (percent domain 0..100) */
#define DUTY_25_PERCENT                  (25.0f)
#define DUTY_50_PERCENT                  (50.0f)
#define DUTY_75_PERCENT                  (75.0f)
#define DUTY_STEP_PERCENT                (10.0f)

/* Minimum pulse policy for unified driver usage: allow 0%/100% */
#define PWM_MIN_PULSE_TIME               (0.0f)

/* Channel count for 3-phase inverter */
#define NUM_OF_CHANNELS                  (3u)

/*
 * Pin assignments: KIT_A2G_TC387_5V_TFT (TOM1 on P00.2..P00.7)
 * High-side (HS) and Low-side (LS) complementary pairs per phase.
 */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* Public API */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);
void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
