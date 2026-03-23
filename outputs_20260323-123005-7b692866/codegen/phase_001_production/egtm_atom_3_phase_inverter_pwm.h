#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"   /* Generic header per requirement */
#include "IfxPort.h"
#include "IfxSrc.h"
#include "IfxCpu_Irq.h"

/* ============================================================================
 * Configuration values from requirements (MUST use these values)
 * ============================================================================ */
#define NUM_OF_CHANNELS                 (3)
#define PWM_FREQUENCY                   (20000.0f)          /* 20 kHz */
#define ISR_PRIORITY_ATOM               (20)
#define PHASE_U_DUTY                    (25.0f)
#define PHASE_V_DUTY                    (50.0f)
#define PHASE_W_DUTY                    (75.0f)
#define PHASE_DUTY_STEP                 (10.0f)

#define CLOCK_EGTM_FXCLK0_HZ            (100000000u)        /* 100 MHz (from requirements) */
#define CLOCK_DTM_CLK0_HZ               (100000000u)        /* 100 MHz (from requirements) */

/* Dead-time: 1.0 us (from requirements) */
#define DEAD_TIME_S                     (1.0e-6f)

/* LED for ISR diagnostic toggle (TC4xx example) */
#define LED                             &MODULE_P03, 9

/* ============================================================================
 * Pin routing for KIT_A3G_TC4D7_LITE: eGTM0 ATOM2 channels via TOUTSEL
 *   U: P20.8 (HS)  / P20.9 (LS)   -> TOUT64 / TOUT65
 *   V: P20.10 (HS) / P20.11 (LS)  -> TOUT66 / TOUT67
 *   W: P20.12 (HS) / P20.13 (LS)  -> TOUT68 / TOUT69
 * Use unified driver routing via OutputConfig; DO NOT call PinMap_set* APIs.
 * ============================================================================ */
#define PHASE_U_HS   (&IfxEgtm_ATOM2_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM2_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM2_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM2_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM2_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM2_2N_TOUT69_P20_13_OUT)

#ifdef __cplusplus
extern "C" {
#endif

/* Public API (from SW Detailed Design) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
