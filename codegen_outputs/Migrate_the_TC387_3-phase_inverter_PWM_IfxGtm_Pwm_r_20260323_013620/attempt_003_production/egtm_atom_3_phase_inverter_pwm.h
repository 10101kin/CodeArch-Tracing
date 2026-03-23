/*
 * egtm_atom_3_phase_inverter_pwm.h
 * TC4xx eGTM ATOM 3-Phase Inverter PWM - Production Header
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration values from requirements */
#define NUM_OF_CHANNELS                 (3u)
#define PWM_FREQUENCY                   (20000.0f)   /* Hz */
#define ISR_PRIORITY_ATOM               (20u)
#define PHASE_U_DUTY                    (25.0f)      /* percent */
#define PHASE_V_DUTY                    (50.0f)      /* percent */
#define PHASE_W_DUTY                    (75.0f)      /* percent */
#define PHASE_DUTY_STEP                 (10.0f)      /* percent per update */
#define DEAD_TIME_US                    (1.0f)

/* LED status pin (placeholder: confirm on target board) */
#define LED                             &MODULE_P13, 0

/*
 * TOUT mapping placeholders for KIT_A3G_TC4D7_LITE
 * Note: Pin mapping TBD – set to first available ATOM1 CH0/1/2 and complementary N pins
 * from IfxEgtm_PinMap once board pins are confirmed. Placeholders use NULL to allow build.
 */
#include "IfxEgtm_PinMap.h"  /* Generic header – concrete symbols to be assigned when available */
#define PHASE_U_HS                      ((IfxEgtm_Atom_ToutMap*)0) /* ATOM1 CH0 HS - TBD */
#define PHASE_U_LS                      ((IfxEgtm_Atom_ToutMap*)0) /* ATOM1 CH0 LS (0N) - TBD */
#define PHASE_V_HS                      ((IfxEgtm_Atom_ToutMap*)0) /* ATOM1 CH1 HS - TBD */
#define PHASE_V_LS                      ((IfxEgtm_Atom_ToutMap*)0) /* ATOM1 CH1 LS (1N) - TBD */
#define PHASE_W_HS                      ((IfxEgtm_Atom_ToutMap*)0) /* ATOM1 CH2 HS - TBD */
#define PHASE_W_LS                      ((IfxEgtm_Atom_ToutMap*)0) /* ATOM1 CH2 LS (2N) - TBD */

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void EgtmAtomIsr_periodEvent(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
