#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/**
 * EGTM_ATOM_3_Phase_Inverter_PWM
 *
 * Production driver for 3-phase inverter PWM using TC4xx eGTM ATOM unified driver (IfxEgtm_Pwm).
 * - Center-aligned 20 kHz PWM
 * - Complementary outputs with 1 us dead-time via CDTM
 * - Sync start and sync update enabled
 * - Period-event ISR on CPU0 with priority 20
 * - Pins routed via TOUTSEL to P20.8/9 (U), P20.10/11 (V), P20.12/13 (W)
 *
 * Notes:
 * - Uses only generic iLLD headers (no family-specific pinmaps)
 * - Watchdog handling belongs only in CpuN_Main.c (not here)
 */

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================
 * Configuration Macros
 * ========================== */
#define NUM_OF_CHANNELS               (3)

/* Timing requirements (Hz) */
#define PWM_FREQUENCY                 (20000.0f)     /* TIMING_PWM_FREQUENCY_HZ */
#define DEADTIME_US                   (1.0f)         /* TIMING_DEADTIME_US */

/* Clock configuration (Hz) */
#define EGTM_CLK0_FREQUENCY_HZ        (100000000.0f) /* CLOCK_EGTM_FXCLK0_HZ / CLOCK_DTM_CLK0_HZ */

/* Interrupt priority macro - MUST use exact name per reference */
#define ISR_PRIORITY_ATOM             (20)

/* Initial duties and runtime step (percent) */
#define PHASE_U_DUTY                  (25.0f)
#define PHASE_V_DUTY                  (50.0f)
#define PHASE_W_DUTY                  (75.0f)
#define PHASE_DUTY_STEP               (10.0f)

/* LED for ISR diagnostic toggle (port, pin) */
#define LED                           &MODULE_P03, 9

/* ==========================
 * Pin Assignments (TOUTSEL routed)
 * eGTM0 ATOM2 channels: CH0=U, CH1=V, CH2=W
 * High-side (HS) active high, Low-side (LS) active low (complementary)
 * ========================== */
#define PHASE_U_HS  (&IfxEgtm_ATOM2_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS  (&IfxEgtm_ATOM2_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS  (&IfxEgtm_ATOM2_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS  (&IfxEgtm_ATOM2_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS  (&IfxEgtm_ATOM2_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS  (&IfxEgtm_ATOM2_2N_TOUT69_P20_13_OUT)

/* ==========================
 * Public API (exact signatures)
 * ========================== */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
