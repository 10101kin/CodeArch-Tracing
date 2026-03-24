/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Production 3‑phase inverter PWM using GTM TOM + PwmHl (TC3xx)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"   /* Generic header per requirement */
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===================== Requirements-derived configuration ===================== */
#define PWM_FREQ_HZ                         (20000.0f)   /* TIMING_PWM_FREQUENCY_HZ = 20000 */
#define DUTY_STEP                           (0.10f)      /* TIMING_DUTY_STEP_PERCENT = 10%  (fraction) */
#define DUTY_25_PERCENT                     (0.25f)
#define DUTY_50_PERCENT                     (0.50f)
#define DUTY_75_PERCENT                     (0.75f)
#define PWM_DEAD_TIME                       (0.5e-6f)    /* 0.5 us */
#define PWM_MIN_PULSE_TIME                  (1.0e-6f)    /* 1.0 us */

#define NUM_PHASES                          (3u)

/* ===================== TOM1 pin assignments (validated for TC3xx) ===================== */
/* Mapping to required pins: U(P00.3/2), V(P00.5/4), W(P00.7/6) on TOM1 */
#define PHASE_U_HS         (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS         (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS         (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS         (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS         (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS         (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* Timer master selection: TOM1, channel 0 time-base */
#define GTM_TOM_MASTER                 (IfxGtm_Tom_1)
#define GTM_TOM_MASTER_TIMER_CH        (IfxGtm_Tom_Ch_0)

/* ===================== Public API ===================== */
/**
 * Initialize GTM TOM1 3‑phase complementary PWM with deadtime and min pulse.
 * - Enables GTM and functional clocks
 * - Configures TOM1 timebase for center‑aligned PWM at 20 kHz
 * - Routes pins for 3 complementary channel pairs on P00.{3/2,5/4,7/6}
 * - Applies deadtime = 0.5 us and min pulse = 1.0 us
 * - Starts timer and loads initial duties U=25%, V=50%, W=75% synchronously
 */
void initGtmTom3phInv(void);

/**
 * Runtime update: increment all three phase on-times by DUTY_STEP (10% of current period)
 * with wrap in [0%..100%). Applies the update synchronously at the next PWM period boundary.
 * Caller is responsible for invoking this every 500 ms.
 */
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
