/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Production-ready eGTM ATOM 3-phase inverter PWM driver (TC4xx - TC387/TC4D7 family).
 *
 * EXACT API CONTRACT (public):
 *   void IfxEgtm_Atom_Pwm_init(void);
 *   void IfxEgtm_Atom_Pwm_setDutyCycle(void);
 *
 * Notes:
 * - Uses unified high-level IfxEgtm_Pwm driver per iLLD reference.
 * - Interrupt configured via channelConfig[0].interrupt; ISR toggles LED P13.0.
 * - Caller writes normalized duty requests [0.0 .. 1.0] into
 *     g_egtmAtom3phInv_requestDuty[3] before calling IfxEgtm_Atom_Pwm_setDutyCycle().
 * - Do NOT place watchdog code in this module (only in CpuN_Main.c).
 */

#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"   /* Pin map symbols (generic header) */
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Configuration macros from requirements
 * ========================================================================= */
#define NUM_OF_CHANNELS                 (3)
#define ISR_PRIORITY_ATOM               (20)              /* Period ISR priority on CPU0 */

/* Timing requirements */
#define PWM_FREQUENCY                   (20000.0f)        /* 20 kHz */
#define TIMING_DEADTIME_US              (1.0f)            /* 1 us */
#define EGTM_PWM_DEADTIME_S             (TIMING_DEADTIME_US * 1.0e-6f)

/* Clock requirements */
#define CLOCK_TARGET_FXCLK0_HZ          (100000000.0f)    /* 100 MHz */

/* eGTM resource selection */
#define EGTM_CLUSTER                    (1)               /* Cluster 1 */
#define EGTM_ATOM                       (0)               /* ATOM0 */

/* LED pin: Toggle in eGTM PWM period ISR */
#define LED                             &MODULE_P13, 0

/* =========================================================================
 * TOUT Pin assignments (complementary pair per phase)
 * Map outputs: U: P20.8 (HS), P20.9 (LS); V: P20.10 (HS), P20.11 (LS);
 *              W: P20.12 (HS), P20.13 (LS)
 * Note: Use ATOM0 channels CH0/CH1/CH2 per requirements. The following
 *       symbols are expected from IfxEgtm_PinMap.h for TC4xx.
 * ========================================================================= */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* =========================================================================
 * Driver state structure
 * ========================================================================= */
typedef struct
{
    IfxEgtm_Pwm             pwm;                               /* PWM driver handle */
    IfxEgtm_Pwm_Channel     channels[NUM_OF_CHANNELS];         /* Channel runtime data */
    float32                 dutyCycles[NUM_OF_CHANNELS];       /* Duty in percent [0..100] */
    float32                 phases[NUM_OF_CHANNELS];           /* Phase offsets (deg or frac) */
    IfxEgtm_Pwm_DeadTime    deadTimes[NUM_OF_CHANNELS];        /* Configured dead-times */

    /* Status and error flags */
    boolean                 pwmInitialized;                     /* TRUE after successful init */
    boolean                 dtmConfigured;                      /* TRUE if DTM configured */
    boolean                 updateInProgress;                   /* TRUE during duty update */
    boolean                 errorClock;                         /* TRUE if FXCLK0 not enabled */
    boolean                 errorPinMap;                        /* TRUE if pin mapping failed (not used with unified driver) */
    boolean                 errorInit;                          /* TRUE if init sequence failed */
    boolean                 errorDutyOutOfRange;                /* TRUE if input duty clamped */
} EgtmAtom3phInv;

/* Global driver instance (defined in .c) */
extern EgtmAtom3phInv g_egtmAtom3phInv;

/* Normalized requested duty array [0.0 .. 1.0] for U, V, W (set by application) */
extern volatile float32 g_egtmAtom3phInv_requestDuty[NUM_OF_CHANNELS];

/* Public API (EXACT names/signatures from SW Detailed Design) */
void IfxEgtm_Atom_Pwm_init(void);
void IfxEgtm_Atom_Pwm_setDutyCycle(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
