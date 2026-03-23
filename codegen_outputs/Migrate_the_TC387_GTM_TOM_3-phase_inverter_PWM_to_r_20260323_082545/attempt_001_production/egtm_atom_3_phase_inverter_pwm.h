/*
 * egtm_atom_3_phase_inverter_pwm.h
 * TC4xx eGTM ATOM 3-Phase Inverter PWM - Production Interface
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================
 * Requirements-derived macros
 * ============================ */
#define TIMING_PWM_FREQUENCY_HZ            (20000.0f)
#define TIMING_CMU_CLOCK_HZ                (100000000.0f)
#define TIMING_DTM_DEADTIME_TICKS          (100u)
#define TIMING_TIMER_TICKS_FULL_PERIOD     (5000u)
#define TIMING_TIMER_TICKS_HALF_PERIOD     (2500u)
#define TIMING_SYNCHRONOUS_UPDATE          (1)
#define CLOCK_REQUIRES_XTAL                (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ     (300u)
#define CLOCK_EGTM_CMU_CLK_MHZ             (100u)

/* Runtime parameter macros (preserve naming pattern) */
#define NUM_OF_CHANNELS                    (3u)
#define PWM_FREQUENCY                      (TIMING_PWM_FREQUENCY_HZ)
#define ISR_PRIORITY_ATOM                  (20)
#define PHASE_U_DUTY                       (25.0f)
#define PHASE_V_DUTY                       (50.0f)
#define PHASE_W_DUTY                       (75.0f)
#define PHASE_DUTY_STEP                    (10.0f)

/* LED pin for ISR diagnostic toggle */
#define LED                                &MODULE_P03, 9

/* ============================
 * Pin assignments (KIT_A3G_TC4D7_LITE proposal)
 * NOTE: Validate these against project PinMap; update if HW differs.
 * ============================ */
#define PHASE_U_HS   &IfxEgtm_ATOM1_1_TOUT1_P02_1_OUT
#define PHASE_U_LS   &IfxEgtm_ATOM1_1N_TOUT4_P02_4_OUT
#define PHASE_V_HS   &IfxEgtm_ATOM1_0_TOUT0_P02_0_OUT
#define PHASE_V_LS   &IfxEgtm_ATOM1_0N_TOUT7_P02_7_OUT
#define PHASE_W_HS   &IfxEgtm_ATOM1_2_TOUT2_P02_2_OUT
#define PHASE_W_LS   &IfxEgtm_ATOM1_2N_TOUT5_P02_5_OUT

/* ============================
 * Public API
 * ============================ */
/** Initialize the eGTM-based 3-phase complementary PWM per SW Detailed Design */
void initEgtmAtom3phInv(void);

/** Runtime duty update with synchronous apply per SW Detailed Design */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxEgtm_periodEventFunction(void *data);
void interruptEgtmAtom(void);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
