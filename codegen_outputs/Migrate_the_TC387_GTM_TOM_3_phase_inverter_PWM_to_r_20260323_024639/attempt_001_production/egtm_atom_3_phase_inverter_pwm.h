/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * TC4xx eGTM ATOM 3-Phase Inverter PWM - Production Header
 *
 * Requirements implemented:
 *  - 20 kHz center-aligned PWM
 *  - Complementary pairs with 1 us deadtime via CDTM
 *  - Cluster_0, ATOM0, channel pairs: U(0/1), V(2/3), W(4/5)
 *  - Sync start and sync update enabled
 *  - Period IRQ on cpu0 priority 20
 *  - CLK0 = 100 MHz derived from GCLK (expected 300 MHz)
 *
 * Notes:
 *  - Pin routing is TBD and must be provided by the project using valid IfxEgtm ATOM TOUT map symbols.
 *  - Watchdog handling must be placed only in CpuN_Main.c files (not here).
 */

#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"  /* Generic pin map header - family-agnostic */
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== Configuration Macros (from requirements) ======================== */
#define NUM_OF_CHANNELS                 (3U)
#define PWM_FREQUENCY                   (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM               (20U)        /* Period IRQ priority on cpu0 */

/* User runtime behavior (reference ramp step) */
#define PHASE_DUTY_STEP                 (10.0f)      /* Duty increment step in percent */

/* Dead-time requirements */
#define DEAD_TIME_US                    (1.0f)
#define DEAD_TIME_TICKS_CLK0_100MHZ     (100U)

/* Expected clock assumptions (for CMU configuration) */
#define SYSTEM_GCLK_TARGET_HZ           (300000000.0f)  /* 300 MHz */
#define EGTM_CLK0_TARGET_HZ             (100000000.0f)  /* 100 MHz */

/* LED pin for ISR toggle (adapt to target board); generic example uses P03.9 */
#define LED                             &MODULE_P03, 9U

/* ======================== Public API ======================== */

/*
 * Initialize eGTM for a 3-phase inverter using the unified PWM driver on ATOM.
 * Implements: module enable + CMU clocks, PWM config, outputs (complementary), DTM/CDTM, ISR, and start.
 */
void initEgtmAtom3phInv(void);

/*
 * Runtime duty update: for each phase, if (duty + STEP) >= 100, wrap to 0; then increment by STEP;
 * apply via IfxEgtm_Pwm_updateChannelsDutyImmediate.
 */
void updateEgtmAtom3phInvDuty(void);

/*
 * ISR for PWM period event on the base channel. Minimal processing: delegate to unified PWM handler
 * and toggle a status LED.
 */
void interruptEgtmAtom(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
