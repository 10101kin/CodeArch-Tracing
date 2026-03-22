/*
 * egtm_atom_3_phase_inverter_pwm.h
 * Production 3-phase inverter PWM driver using eGTM ATOM unified iLLD (TC4xx)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Requirement-driven configuration values */
#define EGTM_CLUSTER_INDEX                 (1u)
#define EGTM_ATOM_UNIT                     (1u)
#define TIMING_PWM_FREQUENCY_HZ            (20000.0f)   /* 20 kHz */
#define TIMING_PERIOD_US                   (50.0f)
#define TIMING_SYNCSTART                   (1)
#define TIMING_SYNCUPDATE                  (1)
#define CLOCK_REQUIRES_XTAL                (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ     (300u)

/* PWM application constants */
#define NUM_OF_CHANNELS                    (3u)
#define PWM_FREQUENCY                      (TIMING_PWM_FREQUENCY_HZ)
#define ISR_PRIORITY_ATOM                  (20)

/* Initial duties in percent (unified driver expects percent, not fraction) */
#define PHASE_U_DUTY                       (25.0f)
#define PHASE_V_DUTY                       (50.0f)
#define PHASE_W_DUTY                       (75.0f)
#define PHASE_DUTY_STEP                    (10.0f)

/* LED pin for ISR diagnostics (adapt to board if needed) */
#define LED                                &MODULE_P03, 9

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
