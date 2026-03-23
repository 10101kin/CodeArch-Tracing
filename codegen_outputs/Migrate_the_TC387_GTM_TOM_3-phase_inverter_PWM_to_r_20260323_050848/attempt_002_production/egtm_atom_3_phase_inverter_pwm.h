/*
 * egtm_atom_3_phase_inverter_pwm.h
 * TC4xx - EGTM ATOM 3-Phase Inverter PWM (Unified IfxEgtm_Pwm)
 *
 * Requirements implemented:
 *  - 3 phases U/V/W at 20 kHz, center-aligned, SOMP with complementary outputs
 *  - Deadtime via CDTM: rising=1.0us, falling=1.0us, clock DTM CLK0
 *  - HS active-high, LS active-low
 *  - Synchronous start and update (shadow at period)
 *  - Period-event ISR on cpu0 with priority 20
 *  - LED/status pin: P10.2, push-pull, active-high
 *
 *  NOTE: Watchdog handling must be performed only in CpuN_Main.c files.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"  /* Generic PinMap header */
#include "IfxEgtm.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"
#include "IfxSrc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration macros from requirements */
#define NUM_OF_CHANNELS                 (3u)
#define PWM_FREQUENCY                   (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM               (20u)

/* Deadtime configuration (symmetric, in seconds) */
#define DEADTIME_RISING_S               (1.0e-6f)
#define DEADTIME_FALLING_S              (1.0e-6f)

/* Duty cycle behavior */
#define TIMING_INITIAL_DUTY_CYCLE_PERCENT   (0.0f)
#define PHASE_DUTY_STEP                     (10.0f)

/* Cluster/Module selection per requirements */
#define EGTM_CLUSTER_USED               IfxEgtm_Cluster_1

/* LED pin (status/diagnostic): P10.2, push-pull, active high */
#define LED                              &MODULE_P10, 2

/*
 * Pin assignment placeholders.
 * IMPORTANT: Replace NULL_PTR with validated IfxEgtm_ATOMx_y[_N]_TOUTz_Ppp_q_OUT symbols
 * from IfxEgtm_PinMap.h for your target hardware.
 */
#define PHASE_U_HS   (NULL_PTR)  /* e.g., &IfxEgtm_ATOM1_0_TOUTxxx_P.._OUT */
#define PHASE_U_LS   (NULL_PTR)  /* e.g., &IfxEgtm_ATOM1_0N_TOUTxxx_P.._OUT */
#define PHASE_V_HS   (NULL_PTR)
#define PHASE_V_LS   (NULL_PTR)
#define PHASE_W_HS   (NULL_PTR)
#define PHASE_W_LS   (NULL_PTR)

/* Driver state structure (TC4xx - EGTM ATOM) */
typedef struct {
    IfxEgtm_Pwm          pwm;                              /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];        /* Channel data (populated by init) */
    float32              dutyCycles[NUM_OF_CHANNELS];      /* Duty cycle values in percent */
    float32              phases[NUM_OF_CHANNELS];          /* Phase shift values (deg or ratio as per API) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];       /* Dead-time values */
} EgtmAtom3phInv;

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);
void interruptEgtmAtomPeriod(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
