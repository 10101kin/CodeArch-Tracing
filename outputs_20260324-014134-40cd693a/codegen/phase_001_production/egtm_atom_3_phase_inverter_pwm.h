/*****************************************************************************
 * File:    egtm_atom_3_phase_inverter_pwm.h
 * Brief:   TC4xx eGTM ATOM 3-Phase inverter PWM driver (unified IfxEgtm_Pwm)
 * Author:  Generated
 *****************************************************************************/
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"
#include "IfxSrc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ Configuration macros ============================ */
#define NUM_OF_CHANNELS           (3U)
#define ISR_PRIORITY_ATOM         (20)

/* Timing requirements */
#define PWM_FREQUENCY             (20000.0f)      /* 20 kHz */
#define DEAD_TIME_US              (1.0f)          /* 1 us */
#define DEAD_TIME_SEC             (1.0e-6f)       /* seconds */

/* Clock requirements */
#define CLOCK_TARGET_FXCLK0_HZ    (100000000.0f)  /* 100 MHz */

/* LED pin for ISR toggle (P13.0) */
#define LED                       &MODULE_P13, 0

/* ============================ Pin assignments (ATOM0 CH0/1/2, Cluster 1) ===== */
/*
 * Mapping per requirements:
 *  - PHASE_U HS=P20.8,  LS=P20.9
 *  - PHASE_V HS=P20.10, LS=P20.11
 *  - PHASE_W HS=P20.12, LS=P20.13
 * Use generic PinMap symbols (IfxEgtm_PinMap.h). Unified driver reads these
 * via OutputConfig; do NOT call PinMap API directly.
 */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ============================ Driver state structures ======================== */

typedef struct {
    IfxEgtm_Pwm          pwm;                               /* Unified PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];         /* Channels populated by init */
    float32              dutyCycles[NUM_OF_CHANNELS];        /* Duty in percent [0..100] */
    float32              phases[NUM_OF_CHANNELS];            /* Phase offsets (deg or s) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];        /* Dead-time values */
} EgtmAtom3phInv;

/* Basic status and error flags (production visibility) */
typedef struct {
    boolean pwm_initialized;
    boolean dtm_configured;
    boolean clock_ok;
    boolean update_in_progress;
    boolean error_clock;
    boolean error_pinmap;
    boolean error_init;
} EgtmAtom3phInv_Status;

/* Duty request interface (fractional [0.0 .. 1.0]) per requirements */
extern float32 g_egtmAtom3phInv_requestedDuty[NUM_OF_CHANNELS];

/* Status flags exported */
extern EgtmAtom3phInv_Status g_egtmAtom3phInv_status;

/* ============================ Public API (exact signatures) ================== */
/* Initialize the eGTM-based 3-phase complementary PWM (unified IfxEgtm_Pwm) */
void IfxEgtm_Atom_Pwm_init(void);

/* Apply runtime duty cycle updates using synchronized shadow transfer */
void IfxEgtm_Atom_Pwm_setDutyCycle(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
