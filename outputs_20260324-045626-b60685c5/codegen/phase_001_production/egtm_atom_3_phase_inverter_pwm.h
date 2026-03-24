/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * TC4xx eGTM ATOM 3-Phase Inverter PWM - Production Header
 *
 * This module configures a 3-phase inverter PWM using the unified IfxEgtm_Pwm driver
 * on TC4xx (eGTM/ATOM). It follows the authoritative iLLD initialization pattern:
 *   - IfxEgtm_Pwm_Config initialized with IfxEgtm_Pwm_initConfig()
 *   - OutputConfig/ChannelConfig/DtmConfig/InterruptConfig arrays populated
 *   - IfxEgtm_Pwm_init() called with driver handle, channels array, and config
 *   - No redundant post-init calls (syncStart and syncUpdate are configured)
 *
 * Requirements implemented:
 *   - Cluster_1, ATOM0 CH0/1/2, 20 kHz, center-aligned
 *   - Complementary outputs via DTM, 1 us rising and falling dead-time
 *   - syncStart and syncUpdate enabled
 *   - Period-event ISR on CPU0, priority 20, toggles LED P13.0
 *   - Pin mapping:
 *       U_HS P20.8 / U_LS P20.9
 *       V_HS P20.10 / V_LS P20.11
 *       W_HS P20.12 / W_LS P20.13
 *
 * Notes:
 *   - Watchdog disables are NOT placed here (must be in CpuN_Main.c only).
 *   - Generic PinMap header is used (IfxEgtm_PinMap.h).
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration macros from requirements */
#define NUM_OF_CHANNELS            (3U)
#define PWM_FREQUENCY              (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM          (20)

/* Duty-cycle initialization and step (percent) */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* Dead-time configuration (seconds) */
#define DEAD_TIME_RISING           (1.0e-6f)
#define DEAD_TIME_FALLING          (1.0e-6f)

/* LED for ISR toggle (P13.0) */
#define LED                        &MODULE_P13, 0

/*
 * Pin assignment via generic PinMap (IfxEgtm_PinMap.h)
 * Mapping per requirement:
 *   U_HS -> P20.8,  U_LS -> P20.9
 *   V_HS -> P20.10, V_LS -> P20.11
 *   W_HS -> P20.12, W_LS -> P20.13
 * Note: The specific TOUT mappings are resolved via the generic PinMap symbols.
 */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
