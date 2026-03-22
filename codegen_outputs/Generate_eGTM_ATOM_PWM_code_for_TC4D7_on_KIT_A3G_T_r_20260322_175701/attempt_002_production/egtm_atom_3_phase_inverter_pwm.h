#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm.h"
#include "IfxSrc.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration from requirements */
#define NUM_OF_CHANNELS                 (3)
#define EGTM_CLUSTER_INDEX              (1)
#define EGTM_ATOM_UNIT                  (1)
#define PWM_FREQUENCY_HZ                (20000.0f)     /* TIMING_PWM_FREQUENCY_HZ */
#define TIMING_PERIOD_US                (50.0f)
#define TIMING_SYNCSTART                (TRUE)
#define TIMING_SYNCUPDATE               (TRUE)

/* Duty cycle requirements (percent) */
#define PHASE_U_DUTY                    (25.0f)
#define PHASE_V_DUTY                    (50.0f)
#define PHASE_W_DUTY                    (75.0f)
#define PHASE_DUTY_STEP                 (10.0f)

/* Dead-time (seconds) */
#define DEAD_TIME_RISING_S              (1.0e-6f)
#define DEAD_TIME_FALLING_S             (1.0e-6f)

/* ISR priority macro: use EXACT name from reference patterns */
#define ISR_PRIORITY_ATOM               (20)

/* LED for ISR diagnostic toggle (TC4xx example pin) */
#define LED                             &MODULE_P03, 9

/*
 * Proposed pin mapping (validate against IfxEgtm_PinMap.h for TC4D7):
 * Phase U: HS = P02.0 (TOUT0),  LS = P02.7 (TOUT7)
 * Phase V: HS = P02.1 (TOUT1),  LS = P02.4 (TOUT4)
 * Phase W: HS = P02.2 (TOUT2),  LS = P02.5 (TOUT5)
 * Note: Using ATOM1 as per requirements (EGTM_ATOM_UNIT = 1)
 */
#define PHASE_U_HS   &IfxEgtm_ATOM1_0_TOUT0_P02_0_OUT
#define PHASE_U_LS   &IfxEgtm_ATOM1_0N_TOUT7_P02_7_OUT
#define PHASE_V_HS   &IfxEgtm_ATOM1_1_TOUT1_P02_1_OUT
#define PHASE_V_LS   &IfxEgtm_ATOM1_1N_TOUT4_P02_4_OUT
#define PHASE_W_HS   &IfxEgtm_ATOM1_2_TOUT2_P02_2_OUT
#define PHASE_W_LS   &IfxEgtm_ATOM1_2N_TOUT5_P02_5_OUT

/* Driver/application state structure for 3-phase inverter (TC4xx eGTM ATOM) */
typedef struct {
    IfxEgtm_Pwm           pwm;                                /* PWM driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];         /* Channel runtime data */
    float32               dutyCycles[NUM_OF_CHANNELS];       /* Duty cycle values (%) */
    float32               phases[NUM_OF_CHANNELS];           /* Phase shift values */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];        /* Dead-time values */
} EgtmAtom3phInv;

/* Public API (exact signatures from SW Detailed Design) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxEgtm_periodEventFunction(void *data);
void interruptEgtmAtom(void);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
