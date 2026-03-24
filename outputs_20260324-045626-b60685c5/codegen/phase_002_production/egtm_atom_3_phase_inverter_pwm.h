/*
 * egtm_atom_3_phase_inverter_pwm.h
 * TC4xx EGTM ATOM 3-Phase Inverter PWM (KIT_A3G_TC4D7_LITE / TC387 family)
 *
 * Requirements implemented:
 *  - Cluster_1, ATOM0 CH0/1/2, 20 kHz, center-aligned
 *  - Complementary via DTM, 1 us rising/falling dead-time
 *  - syncStart and syncUpdate enabled
 *  - Independent duty updates with immediate multi-channel apply
 *  - Period event interrupt on CPU0 with priority 20 toggling LED P13.0
 *  - Pin mapping: U(P20.8/P20.9), V(P20.10/P20.11), W(P20.12/P20.13)
 *
 * Notes:
 *  - Uses unified high-level driver IfxEgtm_Pwm (no explicit PinMap or start calls post init)
 *  - EGTM CMU clock enable block is mandatory and handled in initEgtmAtom3phInv()
 *  - Watchdog handling is NOT included here (belongs in CpuN_Main.c)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------------
 * Configuration Macros from Requirements
 * ---------------------------------------------------------------------------- */
#define NUM_OF_CHANNELS            (3)
#define PWM_FREQUENCY              (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM          (20)

/* Initial duty in percent (unified driver expects percent) */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* Dead-time (seconds) */
#define DEADTIME_RISING_SEC        (1.0e-6f)
#define DEADTIME_FALLING_SEC       (1.0e-6f)

/* LED definition for ISR toggle: P13.0 */
#define LED                        &MODULE_P13, 0

/* ----------------------------------------------------------------------------
 * Pin mapping (generic PinMap macros, not family-specific headers)
 * ATOM0 channels CH0/CH1/CH2 mapped to P20.8/9, P20.10/11, P20.12/13
 * ---------------------------------------------------------------------------- */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ----------------------------------------------------------------------------
 * Driver state structure (unified EGTM PWM)
 * ---------------------------------------------------------------------------- */
typedef struct
{
    IfxEgtm_Pwm           pwm;                            /* PWM driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];      /* Channel runtime data */
    float32               dutyCycles[NUM_OF_CHANNELS];    /* Duty cycle values (%) */
    float32               phases[NUM_OF_CHANNELS];        /* Phase shift values */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];     /* Dead-time values */
} EgtmAtom3phInv;

/* ----------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------- */
/**
 * Initialize the eGTM-based 3-phase inverter PWM as per requirements:
 *  - EGTM enable and CMU clock setup
 *  - Unified IfxEgtm_Pwm configuration (ATOM0, Cluster_1, center, 20 kHz)
 *  - Complementary outputs with DTM dead-time on each channel
 *  - Period-event interrupt routed to CPU0 with priority 20
 *  - Sync start and sync update enabled
 *  - LED P13.0 configured as push-pull output (initially low)
 */
void initEgtmAtom3phInv(void);

/**
 * Runtime duty update (ramp with wrap-around):
 *  - For each channel, if (duty + step) >= 100 then wrap to 0, then add step
 *  - Apply all three duty values immediately using unified driver API
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
