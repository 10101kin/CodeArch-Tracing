#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/*
 * EGTM_ATOM_3_Phase_Inverter_PWM
 * Target: TC4xx (TC387 board, eGTM/ATOM)
 * Production code using unified IfxEgtm_Pwm driver per iLLD patterns.
 */

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= Configuration Macros (from requirements) ========================= */
#define NUM_OF_CHANNELS                 (3)
#define EGTM_CLUSTER                    (1)
#define EGTM_ATOM                       (0)
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)         /* 20 kHz */
#define TIMING_DEADTIME_US              (1.0f)             /* 1 us */
#define CLOCK_TARGET_FXCLK0_HZ          (100000000.0f)     /* 100 MHz */
#define ISR_PRIORITY_ATOM               (20)

/* LED for ISR diagnostic toggle: P13.0 */
#define LED                             &MODULE_P13, 0

/* ========================= eGTM ATOM Output Pin Mapping (KIT_A3G_TC4D7_LITE) ========================= */
/* PHASE_U: HS=P20.8, LS=P20.9 */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
/* PHASE_V: HS=P20.10, LS=P20.11 */
#define PHASE_V_HS   (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
/* PHASE_W: HS=P20.12, LS=P20.13 */
#define PHASE_W_HS   (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ========================= Public software interface ========================= */
/*
 * Software input array for requested duty cycles in normalized units [0.0 .. 1.0]
 * Index 0: Phase U, 1: Phase V, 2: Phase W
 * The runtime API IfxEgtm_Atom_Pwm_setDutyCycle() reads and applies these.
 */
extern float32 g_egtmAtom3phInv_requestedDuty[NUM_OF_CHANNELS];

/*
 * Initialize the eGTM-based 3-phase complementary PWM using IfxEgtm_Pwm.
 * Follows unified iLLD initialization pattern and requirements.
 */
void IfxEgtm_Atom_Pwm_init(void);

/*
 * Apply runtime duty cycle updates from g_egtmAtom3phInv_requestedDuty[] in a synchronized manner.
 * Values are clamped to [0.0 .. 1.0], converted to percent, and updated immediately.
 */
void IfxEgtm_Atom_Pwm_setDutyCycle(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
