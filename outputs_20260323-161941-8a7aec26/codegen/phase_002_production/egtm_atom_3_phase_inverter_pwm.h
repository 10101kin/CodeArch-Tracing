/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * TC4xx (TC4D7) - eGTM ATOM 3-Phase Inverter PWM (unified IfxEgtm_Pwm driver)
 *
 * Requirements implemented:
 *  - 20 kHz center-aligned complementary PWM, single ATOM instance
 *  - Dead-time = 0.5 us, Min pulse = 1.0 us (software-clamped in update)
 *  - Pins mapped to KIT_A3G_TC4D7_LITE: U(P00.3/P00.2), V(P00.5/P00.4), W(P00.7/P00.6)
 *  - Use eGTM CMU CLK0 from GCLK
 *  - Unified high-level driver configuration pattern (Config/init with arrays)
 *
 * Notes:
 *  - Watchdog disable is not present here by design (only allowed in CpuN_Main.c)
 *  - Only generic iLLD headers are included
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================
 * Configuration constants
 * ========================= */
#define NUM_OF_CHANNELS                 (3u)

/* Interrupt priority macro (use exact name as reference patterns) */
#define ISR_PRIORITY_ATOM               (20u)

/* Requirements-driven timing configuration */
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)      /* 20 kHz */
#define TIMING_DEADTIME_US              (0.5f)          /* 0.5 us */
#define TIMING_MIN_PULSE_US             (1.0f)          /* 1.0 us */

/* Derived duty boundary in percent, based on min pulse and PWM period */
#define PWM_PERIOD_US                   (1000000.0f / (TIMING_PWM_FREQUENCY_HZ))
#define MIN_DUTY_PERCENT                ((TIMING_MIN_PULSE_US / PWM_PERIOD_US) * 100.0f)  /* e.g., 2.0% for 1us@20kHz */
#define MAX_DUTY_PERCENT                (100.0f - MIN_DUTY_PERCENT)

/* Reference-style macros (percent representation) */
#define PWM_MIN_PULSE_TIME              (TIMING_MIN_PULSE_US)
#define DUTY_25_PERCENT                 (25.0f)
#define DUTY_50_PERCENT                 (50.0f)
#define DUTY_75_PERCENT                 (75.0f)
#define DUTY_STEP                       (10.0f)    /* duty step in percent for ramp */
#define DUTY_MIN                        (MIN_DUTY_PERCENT)
#define DUTY_MAX                        (MAX_DUTY_PERCENT)

/*
 * Pin assignment for KIT_A3G_TC4D7_LITE (to be validated against IfxEgtm_PinMap.h):
 *  U phase: HS = P00.3 (TOUT12), LS = P00.2 (TOUT11)
 *  V phase: HS = P00.5 (TOUT14), LS = P00.4 (TOUT13)
 *  W phase: HS = P00.7 (TOUT16), LS = P00.6 (TOUT15)
 *
 *  These map to eGTM ATOM1 tentative channels as shown. Adjust if board routing differs.
 */
#define PHASE_U_HS   (&IfxEgtm_ATOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM1_5_TOUT15_P00_6_OUT)

/* LED for ISR diagnostic (TC4xx example) */
#define LED                          &MODULE_P03, 9

/* =========================
 * Driver/application state
 * ========================= */

typedef struct {
    IfxEgtm_Pwm          pwm;                             /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];       /* Channels realized after init */
    float32              dutyCycles[NUM_OF_CHANNELS];      /* Duty in percent [0..100] */
    float32              phases[NUM_OF_CHANNELS];          /* Optional phase shift in degrees */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];      /* Dead-time configuration snapshot */
} EgtmAtom3phInv;

/* =========================
 * Public API
 * ========================= */

/*
 * Initialize the eGTM-based complementary, center-aligned 3-phase PWM for an inverter
 * using a single ATOM instance (ATOM1 by requirement). See SW Detailed Design for steps.
 */
void initEgtmAtom3phInv(void);

/*
 * Update inverter PWM duty commands synchronously across complementary pairs
 * with clamping to maintain a minimum pulse, as per SW Detailed Design.
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
