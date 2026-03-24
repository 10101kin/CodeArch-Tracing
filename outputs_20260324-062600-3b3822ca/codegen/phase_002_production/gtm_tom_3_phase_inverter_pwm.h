/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Production driver: GTM TOM 3-Phase Inverter PWM (single-ended outputs using IfxGtm_Pwm)
 * Target: TC3xx (TC387)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm.h"
#include "IfxPort.h"

/* ================= Requirements-driven configuration values ================= */
#define NUM_OF_CHANNELS                 (3u)

/* Timebase and timing requirements */
#define TIMEBASE_CHANNEL                TOM1_CH0                /* From requirements */
#define TIMEBASE_ALIGNMENT              IfxGtm_Pwm_Alignment_center
#define TIMEBASE_FREQUENCY_HZ           (20000.0f)              /* 20 kHz */
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)
#define TIMING_DUTY_UPDATE_PERIOD_MS    (500u)
#define TIMING_DUTY_STEP_PERCENT        (10.0f)
#define TIMING_WRAPAROUND_AT_100_TO_0   (1u)
#define TIMING_CPU0_WAIT_TIME_MS        (500u)
#define CLOCK_REQUIRES_XTAL             (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ  (300u)

/* Initial duties in percent (unified IfxGtm_Pwm expects percent [0..100]) */
#define DUTY_INIT_U_PERCENT             (25.0f)
#define DUTY_INIT_V_PERCENT             (50.0f)
#define DUTY_INIT_W_PERCENT             (75.0f)

/* ISR/LED diagnostic (priority macro matches TOM naming) */
#define ISR_PRIORITY_TOM                (20)
#define LED                             &MODULE_P13, 0

/* ===================== Pin assignments (validated pattern) ================== */
/* User-specified: TOM1 CH2/CH4/CH6 routed to P02.0/P02.1/P02.2 respectively */
#define PHASE_U_TOM_PIN   (&IfxGtm_TOM1_2_TOUT12_P02_0_OUT)
#define PHASE_V_TOM_PIN   (&IfxGtm_TOM1_4_TOUT14_P02_1_OUT)
#define PHASE_W_TOM_PIN   (&IfxGtm_TOM1_6_TOUT16_P02_2_OUT)

/* ============================== Public API ================================= */
/* Initialize GTM TOM PWM (3 single-ended channels) */
void initGtmTom3PhaseInverterPwm(void);

/* Runtime update: increment duties by +10% with wrap 100->0 and apply immediately */
void updateGtmTom3PhaseDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptGtmTom(void);

}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
