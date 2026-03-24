/**********************************************************************************************************************
 * Module: GTM_TOM_3_Phase_Inverter_PWM
 * File:   gtm_tom_3_phase_inverter_pwm.h
 * Brief:  TC3xx GTM/TOM 3-phase inverter PWM using unified IfxGtm_Pwm with TOM time base
 *
 * Production requirements:
 * - Implements EXACT API contract from SW Detailed Design
 * - Uses REAL iLLD drivers and GENERIC headers
 * - No watchdog handling here (only in CpuN_Main.c)
 *********************************************************************************************************************/
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

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

/* ========================= Requirements-defined constants ========================= */
#define INITIAL_DUTY_PERCENT_U       (25.0f)     /* FROM REQUIREMENTS */
#define INITIAL_DUTY_PERCENT_V       (50.0f)     /* FROM REQUIREMENTS */
#define INITIAL_DUTY_PERCENT_W       (75.0f)     /* FROM REQUIREMENTS */
#define TIMING_PWM_FREQUENCY_HZ      (20000.0f)  /* FROM REQUIREMENTS: 20 kHz */
#define TIMING_UPDATE_INTERVAL_MS    (500U)      /* FROM REQUIREMENTS: 500 ms */
#define CLOCK_REQUIRES_XTAL          (1)         /* FROM REQUIREMENTS */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ (300U)    /* FROM REQUIREMENTS */

/* ========================= Application configuration ========================= */
#define GTM_TOM_PWM_NUM_PHASES       (3U)
#define GTM_TOM_PWM_NUM_OUTPUTS      (6U)        /* 3 complementary pairs → 6 outputs */
#define GTM_TOM_PWM_DUTY_STEP        (10.0f)     /* +10% per update */

/* Pin assignment: Keep TOM1 channels 1..6 on P00.2..P00.7 (KIT_A2G_TC387_5V_TFT requirement) */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* Output electrical characteristics */
#define GTM_TOM_PWM_OUTPUT_MODE      (IfxPort_OutputMode_pushPull)
#define GTM_TOM_PWM_PAD_DRIVER       (IfxPort_PadDriver_cmosAutomotiveSpeed1)

/* Public API (EXACT signatures) */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);
void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
