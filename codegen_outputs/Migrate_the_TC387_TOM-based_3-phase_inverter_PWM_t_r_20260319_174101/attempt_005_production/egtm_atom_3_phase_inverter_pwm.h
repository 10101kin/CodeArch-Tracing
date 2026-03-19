/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * TC4xx (TC4D7) eGTM ATOM 3-Phase Inverter PWM - Production Header
 *
 * Requirements implemented:
 *  - Unified IfxEgtm_Pwm driver
 *  - 20 kHz center-aligned complementary PWM
 *  - 1 us dead-time via DTM
 *  - Sync start and sync update enabled
 *  - Period ISR on CPU0 priority 20 (ISR_PRIORITY_ATOM)
 *  - In-memory status reporting (lastError bitmask, ISR count, missed updates)
 *
 * Notes:
 *  - TOUT routing is configured via OutputConfig in init; actual pins are TBD for the target board.
 *  - Watchdog disable is NOT included here per AURIX architecture rules.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= Configuration Macros (from Requirements) ========================= */
#define NUM_OF_CHANNELS            (3)
#define PWM_FREQUENCY              (20000.0f)    /* TIMING_PWM_FREQUENCY_HZ = 20000 */
#define DEADTIME_NS                (1000U)       /* TIMING_DEADTIME_NS = 1000 ns */
#define ISR_PRIORITY_ATOM          (20)          /* INTERRUPTS_PERIOD_ISR_PRIORITY = 20 */

/* LED port/pin used for ISR diagnostics (TC4xx example) */
#define LED                        &MODULE_P03, 9

/* Initial duty and update step (percent) */
#define PHASE_INIT_DUTY            (50.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* ========================= Status Reporting ========================= */
/**
 * Error bitmask for the module (expand as needed)
 */
typedef enum
{
    EGTM_PWM_ERROR_NONE              = 0u,
    EGTM_PWM_ERROR_CLOCK_INIT        = (1u << 0),
    EGTM_PWM_ERROR_DRIVER_INIT       = (1u << 1),
    EGTM_PWM_ERROR_PIN_ROUTE         = (1u << 2)
} EgtmPwm_ErrorBits;

/**
 * Status structure maintained in memory (as per requirements)
 */
typedef struct
{
    volatile uint32        isrCount;           /* Period ISR counter */
    volatile uint32        missedUpdates;      /* Missed duty update detections */
    volatile uint32        lastRequestEpoch;   /* Epoch at which the last duty update was requested */
    volatile EgtmPwm_ErrorBits lastError;      /* Last error bitmask */
} EgtmPwm_Status;

/* Expose status instance for monitoring */
extern EgtmPwm_Status g_egtmPwmStatus;

/* ========================= Public API ========================= */
/**
 * Initialize the eGTM for 3-phase center-aligned complementary PWM generation using ATOM channels
 * and CDTM DTM for hardware dead-time.
 */
void initEgtmAtom3phInv(void);

/**
 * Cyclic duty update: increment each phase by PHASE_DUTY_STEP with wrap at 100% and apply immediately.
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
