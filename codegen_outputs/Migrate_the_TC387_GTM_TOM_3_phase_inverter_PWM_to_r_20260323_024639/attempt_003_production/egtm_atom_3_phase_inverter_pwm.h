#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Configuration constants from requirements
 * ======================================================================== */
#define NUM_OF_CHANNELS                           (3)
#define EGTM_RESOURCES_CLUSTER                    IfxEgtm_Cluster_0
#define EGTM_RESOURCES_ATOM_CH_U_BASE             IfxEgtm_Pwm_SubModule_Ch_0
#define EGTM_RESOURCES_ATOM_CH_V_BASE             IfxEgtm_Pwm_SubModule_Ch_2
#define EGTM_RESOURCES_ATOM_CH_W_BASE             IfxEgtm_Pwm_SubModule_Ch_4
#define PWM_FREQUENCY                             (20000.0f)   /* 20 kHz */
#define PWM_MODE_CENTER_ALIGNED                   (1)
#define PWM_MODE_COMPLEMENTARY_PAIRS              (1)
#define PWM_MODE_SYNCSTART                        (1)
#define PWM_MODE_SYNCUPDATE                       (1)
#define DEADTIME_USE_CDTM                         (1)
#define DEADTIME_DEADTIME_US                      (1.0f)
#define DEADTIME_DEADTIME_SEC                     (1.0e-6f)
#define DEADTIME_DEADTIME_TICKS_AT_CLK0_100MHZ    (100u)
#define DUTY_UPDATE_INDEPENDENT_UV_W              (1)
#define DUTY_UPDATE_INITIAL_DUTY_PER_PHASE_U      (0.0f)
#define DUTY_UPDATE_INITIAL_DUTY_PER_PHASE_V      (0.0f)
#define DUTY_UPDATE_INITIAL_DUTY_PER_PHASE_W      (0.0f)
#define DUTY_UPDATE_STEP_PERCENT                  (10.0f)
#define POLARITY_HIGH_SIDE_NON_INVERTED           (1)
#define POLARITY_LOW_SIDE_INVERTED                (1)
#define ISR_PRIORITY_ATOM                         (20)

/* Status LED pin: adapt if board uses a different LED */
#define LED                                       &MODULE_P03, 9

/* ========================================================================
 * Pin routing (EGTM ATOM TOUT pins) - select valid pins for your board
 * The pairs below follow: HS uses channel base (0/2/4), LS uses complementary (N)
 * ======================================================================== */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM0_2_TOUT2_P02_2_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM0_2N_TOUT3_P02_3_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM0_4_TOUT4_P02_4_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM0_4N_TOUT5_P02_5_OUT)

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);
void interruptEgtmAtom(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
