#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"      /* Types for PWM driver and pin map pointers */
#include "IfxEgtm_PinMap.h"   /* Generic pin map header (family-independent) */

#ifdef __cplusplus
extern "C" {
#endif

/* =============================
 * Configuration macros (from requirements)
 * ============================= */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY              (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM          (20u)

#define TIMING_DEAD_TIME_US        (1.0f)       /* 1.0 microsecond */

/* Initial duty cycles in percent (reference-friendly values) */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)

/* Duty step for runtime ramping (percent) */
#define PHASE_DUTY_STEP            (10.0f)

/* LED for ISR heartbeat: default to P13.0 (update per board if needed) */
#define LED                        &MODULE_P13, 0

/* =============================
 * TOUT pin mapping placeholders (TBD per board)
 *
 * NOTE:
 *  - Select concrete TOUT mappings for ATOM1 CH0/1/2 and complementary 0N/1N/2N
 *    from IfxEgtm_PinMap for KIT_A3G_TC4D7_LITE.
 *  - These placeholders intentionally default to NULL. Replace with real symbols like:
 *      (IfxEgtm_Pwm_ToutMap*)&IfxEgtm_ATOM1_0_TOUTXXX_PYY_Z_OUT
 *    before deploying to hardware.
 * ============================= */
#define PHASE_U_HS                 ((IfxEgtm_Pwm_ToutMap*)0)
#define PHASE_U_LS                 ((IfxEgtm_Pwm_ToutMap*)0)
#define PHASE_V_HS                 ((IfxEgtm_Pwm_ToutMap*)0)
#define PHASE_V_LS                 ((IfxEgtm_Pwm_ToutMap*)0)
#define PHASE_W_HS                 ((IfxEgtm_Pwm_ToutMap*)0)
#define PHASE_W_LS                 ((IfxEgtm_Pwm_ToutMap*)0)

/* =============================
 * Public API (from SW Detailed Design)
 * ============================= */
/**
 * Initialize the eGTM-based 3-phase PWM for a motor inverter using the unified IfxEgtm_Pwm driver.
 * See SW Detailed Design for the exact behavior and configuration steps.
 */
void initEgtmAtom3phInv(void);

/**
 * Perform per-phase independent duty ramping and apply immediately using the unified driver.
 * Implements: for each channel, if (duty + STEP) >= 100, wrap to 0, then increment by STEP; finally update.
 */
void updateEgtmAtom3phInvDuty(void);

/* Period-event ISR name mandated by requirements */
void EgtmAtomIsr_periodEvent(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
