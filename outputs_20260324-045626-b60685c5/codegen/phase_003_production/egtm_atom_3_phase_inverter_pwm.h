/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Production driver for TC4xx EGTM ATOM 3-Phase Inverter PWM using unified IfxEgtm_Pwm driver.
 *
 * Requirements summary:
 * - Cluster: Cluster_1
 * - Submodule: ATOM0 channels 0/1/2
 * - PWM: 20 kHz, center-aligned, complementary via DTM, 1 us dead-time (rising/falling)
 * - Sync start and sync update enabled
 * - Independent duty updates supported
 * - Period event ISR on CPU0, priority 20, toggles LED P13.0
 * - Pin mapping:
 *     U_HS P20.8 / U_LS P20.9
 *     V_HS P20.10 / V_LS P20.11
 *     W_HS P20.12 / W_LS P20.13
 * - Clocks: Enable EGTM GCLK and CLK0 (ATOM uses Clk_0; DTM uses CLK0)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxSrc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration macros from requirements */
#define NUM_OF_CHANNELS             (3)
#define PWM_FREQUENCY               (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM           (20)

/* Initial duties and step (percent representation) */
#define PHASE_U_DUTY                (25.0f)
#define PHASE_V_DUTY                (50.0f)
#define PHASE_W_DUTY                (75.0f)
#define PHASE_DUTY_STEP             (10.0f)

/* Dead-time (seconds) */
#define DEAD_TIME_RISING_S          (1.0e-6f)
#define DEAD_TIME_FALLING_S         (1.0e-6f)

/* LED pin for ISR toggle */
#define LED                         &MODULE_P13, 0

/* Pin assignments (generic PinMap symbols; unified driver handles muxing) */
/* ATOM0 on Cluster_1, mapped to P20.8 .. P20.13 as specified */
#define PHASE_U_HS  (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS  (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS  (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS  (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS  (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS  (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* Driver/application state */
typedef struct {
    IfxEgtm_Pwm           pwm;                               /* PWM driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];         /* Channel data after init */
    float32               dutyCycles[NUM_OF_CHANNELS];       /* Duty cycle values (%) */
    float32               phases[NUM_OF_CHANNELS];           /* Phase offsets (deg or %) */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];        /* Dead-time values (rising/falling) */
} EgtmAtom3phInv;

/* Global driver instance */
extern EgtmAtom3phInv g_egtmAtom3phInv;

/* Period event callback (unified driver calls this if configured) */
void IfxEgtm_periodEventFunction(void *data);

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
