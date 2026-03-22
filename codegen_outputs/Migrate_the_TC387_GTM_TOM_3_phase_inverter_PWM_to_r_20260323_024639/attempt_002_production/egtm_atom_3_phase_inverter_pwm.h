/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Production-ready TC4xx eGTM ATOM 3-phase inverter PWM driver (unified IfxEgtm_Pwm)
 *
 * Requirements implemented:
 * - TC4xx, TC387 board target (EGTM on TC4xx family)
 * - 20 kHz, center-aligned PWM, complementary HS/LS per phase
 * - 1 us dead-time via CDTM (CLK0 = 100 MHz)
 * - Independent U/V/W duty updates using IfxEgtm_Pwm_updateChannelsDutyImmediate
 * - Period IRQ on cpu0 with priority = ISR_PRIORITY_ATOM (20)
 * - eGTM clocks: enable module, GCLK from module freq, CLK0 = 100 MHz, enable clocks
 * - Cluster_0, ATOM0, channel pairs U: ch0, V: ch2, W: ch4 (complements via *_N pins)
 * - syncStart and syncUpdate enabled
 *
 * Notes:
 * - Pin routing is provided via OutputConfig; no direct PinMap API calls are used here.
 * - Status LED pin is configurable; default set to P03.9 (adapt as needed per board).
 * - Watchdog handling is NOT part of this driver; CPUs handle watchdog separately.
 */

#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Ifx_Types.h"

/* =============================
 * Configuration Macros (from requirements)
 * ============================= */
#define NUM_OF_CHANNELS                 (3u)
#define PWM_FREQUENCY_HZ                (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM               (20u)        /* period IRQ priority on cpu0 */

/* Dead-time: 1 us (CLK0 = 100 MHz -> ~100 ticks); unified driver uses time units */
#define DEAD_TIME_SEC                   (1.0e-6f)

/* Duty update step for runtime ramp demo (reference-aligned) */
#define PHASE_DUTY_STEP                 (10.0f)

/* Initial duty per phase (percent) */
#define DUTY_INIT_U_PERCENT             (0.0f)
#define DUTY_INIT_V_PERCENT             (0.0f)
#define DUTY_INIT_W_PERCENT             (0.0f)

/* LED status/toggle pin (port, pin) - adapt to your board */
#define LED                             &MODULE_P03, 9

/* Public API - EXACT signatures from SW Detailed Design */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);
void interruptEgtmAtom(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
