/*
 * egtm_atom_3_phase_inverter_pwm.h
 * Production-ready eGTM ATOM 3-phase inverter PWM driver (TC4xx / TC387)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"   /* Generic PinMap header (family-agnostic) */
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================
 * Requirements-driven macros
 * ========================= */
#define NUM_OF_CHANNELS                   (3U)

/* Timing requirements */
#define PWM_FREQUENCY                     (20000.0f)        /* 20 kHz */
#define PHASE_DUTY_STEP                   (10.0f)           /* Duty increment step (%) for runtime demo */
#define TIMING_INITIAL_DUTY_CYCLE_PERCENT (0.0f)

/* Dead-time (CDTM/DTM) requirements */
#define DEADTIME_RISING_US                (1.0f)
#define DEADTIME_FALLING_US               (1.0f)

/* Interrupt priority (CPU0) */
#define ISR_PRIORITY_ATOM                 (20U)

/* LED/status pin (default proposal from requirements: P10.2) */
#define LED                               &MODULE_P10, 2

/*
 * Pin assignments (verify against IfxEgtm_PinMap for your board/derivative)
 * Proposed placement per requirements: eGTM Cluster 1, ATOM1 CH0/1/2 and P02.x pins
 * NOTE: Validate these mappings on your specific hardware (KIT_A3G_TC4D7_LITE or custom HW).
 */
#define PHASE_U_HS   (&IfxEgtm_ATOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM1_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM1_2N_TOUT5_P02_5_OUT)

/* =========================
 * Public API (from SW Design)
 * ========================= */

/**
 * Initialize the eGTM-based 3-phase inverter PWM.
 * Steps (summarized):
 * 1) Enable eGTM and configure clocks for ATOM and DTM.
 * 2) Initialize unified IfxEgtm_Pwm config (center-aligned, 20 kHz, base+sync channels).
 * 3) Configure complementary outputs (HS active-high, LS active-low) via OutputConfig.
 * 4) Configure symmetric dead-time = 1 us (CDTM/DTM) per channel.
 * 5) Configure period-event interrupt on base channel (CPU0, priority 20), install ISR.
 * 6) Initialize channels with unified driver (shadow update enabled, sync start on init).
 * 7) Leave PWM ready for synchronous duty updates.
 */
void initEgtmAtom3phInv(void);

/**
 * Runtime duty update: for each phase, if (duty + STEP) >= 100% then wrap to 0%, else increment by STEP.
 * Queue updated duties for synchronous transfer at period boundary using unified IfxEgtm_Pwm API.
 */
void updateEgtmAtom3phInvDuty(void);

/**
 * ISR for PWM period event (base channel). Minimal work: delegate to unified driver handler.
 */
void interruptEgtmAtomPeriod(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
