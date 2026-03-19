/*
 * egtm_atom_3_phase_inverter_pwm.h
 * TC4xx (TC387 board), eGTM ATOM 3-phase inverter PWM using unified IfxEgtm_Pwm driver
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration values from requirements */
#define NUM_OF_CHANNELS                 (3U)
#define PWM_FREQUENCY                   (20000.0f)      /* TIMING_PWM_FREQUENCY_HZ = 20000 */
#define TIMING_DEADTIME_NS              (1000U)         /* TIMING_DEADTIME_NS = 1000 */
#define DEAD_TIME_SEC                   (1.0e-6f)       /* 1000 ns -> 1 us */
#define ISR_PRIORITY_ATOM               (20U)           /* INTERRUPTS_PERIOD_ISR_PRIORITY = 20 */

/* LED diagnostic pin for ISR toggle (TC4xx example) */
#define LED                             &MODULE_P03, 9

/* Initial duties (percent) and step, per unified driver semantics */
#define PHASE_U_DUTY                    (50.0f)
#define PHASE_V_DUTY                    (50.0f)
#define PHASE_W_DUTY                    (50.0f)
#define PHASE_DUTY_STEP                 (10.0f)

/* Status reporting per requirements: in-memory, lastError as enum/bitmask, counters as uint32 */
typedef enum
{
    EgtmAtom3phInv_Error_none            = 0u,
    EgtmAtom3phInv_Error_clockInit       = 1u << 0,
    EgtmAtom3phInv_Error_pwmInit         = 1u << 1,
    EgtmAtom3phInv_Error_isrInstall      = 1u << 2
} EgtmAtom3phInv_Error;

typedef struct
{
    EgtmAtom3phInv_Error lastError;     /* enum/bitmask */
    uint32               isrCount;      /* STATUS_REPORTING_STRUCT_ISRCOUNT */
    uint32               missedUpdates; /* STATUS_REPORTING_STRUCT_MISSEDUPDATES */
} EgtmAtom3phInv_Status;

/* Driver state for TC4xx (EGTM ATOM) unified PWM */
typedef struct
{
    IfxEgtm_Pwm              pwm;                                /* PWM driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];          /* Channel data after init */
    float32                  dutyCycles[NUM_OF_CHANNELS];        /* Duty cycle values in percent */
    float32                  phases[NUM_OF_CHANNELS];            /* Phase shift values in percent of period */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];         /* Dead-time cfg snapshot (rising/falling) */
    EgtmAtom3phInv_Status    status;                             /* In-memory status reporting */
    uint32                   lastAppliedEpoch;                   /* For missed update detection */
} EgtmAtom3phInv;

extern EgtmAtom3phInv g_egtmAtom3phInv;

/* Public API (from SW Detailed Design) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxEgtm_periodEventFunction(void *data);
void interruptEgtmAtom(void);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
