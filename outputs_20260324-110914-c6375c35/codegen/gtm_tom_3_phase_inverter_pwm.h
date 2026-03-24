#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

/*
 * GTM TOM 3-Phase Inverter PWM - Production Header
 * Target: AURIX TC3xx (TC387)
 * Driver level: Unified IfxGtm_Pwm with TOM time base
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

/* ========================================================================== */
/* Requirements-driven configuration values                                    */
/* ========================================================================== */
#define INITIAL_DUTY_PERCENT_U          (25.0f)     /* FROM REQUIREMENTS */
#define INITIAL_DUTY_PERCENT_V          (50.0f)     /* FROM REQUIREMENTS */
#define INITIAL_DUTY_PERCENT_W          (75.0f)     /* FROM REQUIREMENTS */
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)  /* 20 kHz - FROM REQUIREMENTS */
#define TIMING_UPDATE_INTERVAL_MS       (500U)      /* FROM REQUIREMENTS */
#define CLOCK_REQUIRES_XTAL             (1)         /* True - FROM REQUIREMENTS */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ  (300U)      /* FROM REQUIREMENTS */

/* Duty step behavior (user requirement) */
#define DUTY_STEP_PERCENT               (10.0f)

/* ========================================================================== */
/* Application channel topology                                                */
/* ========================================================================== */
#define NUM_PWM_CHANNELS                (6U)        /* Six TOM outputs (U,V,W complementary pairs) */

/* TOM base selection: retain TOM1 as in the reference project */
#define GTM_TOM_MASTER                  IfxGtm_Tom_1
#define GTM_TOM_MASTER_TIMER_CH        IfxGtm_Tom_Ch_0

/* ========================================================================== */
/* Validated pin assignments for TC387 (TOM1 channels 1..6 on P00.2..P00.7)    */
/* ========================================================================== */
#define PHASE_U_LS_PIN   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)  /* U low-side  - TOM1 CH1 -> P00.2 */
#define PHASE_U_HS_PIN   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)  /* U high-side - TOM1 CH2 -> P00.3 */
#define PHASE_V_LS_PIN   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)  /* V low-side  - TOM1 CH3 -> P00.4 */
#define PHASE_V_HS_PIN   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)  /* V high-side - TOM1 CH4 -> P00.5 */
#define PHASE_W_LS_PIN   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)  /* W low-side  - TOM1 CH5 -> P00.6 */
#define PHASE_W_HS_PIN   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)  /* W high-side - TOM1 CH6 -> P00.7 */

/* ========================================================================== */
/* Public API                                                                  */
/* ========================================================================== */
/**
 * Initialize GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm (unified driver) and
 * IfxGtm_Tom_Timer as time base. Configures six TOM outputs as three complementary
 * pairs (U, V, W), sets 20 kHz center-aligned PWM with synchronous updates, muxes
 * pins, applies initial duties (U=25%, V=50%, W=75%), and starts synced outputs.
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);

/**
 * Step phase duties by +10% and wrap to 0% on overflow, then apply synchronously
 * to all six outputs (complementary pairs share the same duty).
 */
void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
