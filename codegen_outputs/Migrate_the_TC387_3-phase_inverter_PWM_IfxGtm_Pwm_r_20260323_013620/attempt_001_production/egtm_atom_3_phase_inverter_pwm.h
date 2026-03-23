#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the eGTM-based 3-phase PWM for a motor inverter using the unified IfxEgtm_Pwm driver.
 * Behavior per SW Detailed Design:
 *  - Enable eGTM and CMU clocks, select FXCLK source for ATOM, DTM clock from CMU clock 0
 *  - Configure ATOM (center-aligned), 20 kHz, syncStart and syncUpdate enabled
 *  - Prepare 3 contiguous channels (0,1,2) with phase=0, initial duties, and 1.0 us dead-time (rise/fall)
 *  - Configure complementary outputs (HS active-high, LS active-low), push-pull, automotive pad driver
 *  - Configure single period-event interrupt on base channel (CPU0, priority 20)
 *  - Initialize PWM driver with channel array and configuration; install ISR for LED heartbeat
 *  - Channels start synchronized (via syncStart) and syncUpdate remains enabled for runtime updates
 */
void initEgtmAtom3phInv(void);

/**
 * Runtime duty update for 3-phase inverter.
 * Behavior per SW Detailed Design:
 *  - For each channel: if (duty + STEP) >= 100.0 then wrap to 0.0
 *  - After boundary checks: increment each channel's duty by STEP
 *  - Apply the new duty array immediately using the unified driver's multi-channel API
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void EgtmAtomIsr_periodEvent(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif // EGTM_ATOM_3_PHASE_INVERTER_PWM_H
