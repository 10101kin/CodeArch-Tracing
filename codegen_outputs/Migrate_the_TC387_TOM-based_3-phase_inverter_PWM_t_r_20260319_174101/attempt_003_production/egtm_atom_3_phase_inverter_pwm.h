/**********************************************************************************************************************
 * Module: EGTM_ATOM_3_Phase_Inverter_PWM
 * File:   egtm_atom_3_phase_inverter_pwm.h
 * Target: TC4xx (TC4D7/TC387 family)
 * Desc:   3-Phase inverter PWM using eGTM ATOM with unified IfxEgtm_Pwm driver.
 *
 * Notes:
 *  - Uses unified high-level IfxEgtm_Pwm driver as per iLLD patterns.
 *  - Pin routing must be provided via OutputConfig (no direct PinMap calls in init).
 *  - eGTM clocks are enabled in init as mandated.
 *  - Period ISR on CPU0, priority from requirements.
 *  - Status reporting is in-memory via a simple struct.
 **********************************************************************************************************************/
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Requirements-driven configuration macros
 * ==========================================================================*/
#define NUM_OF_CHANNELS                 (3U)

/* Timing Requirements */
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)   /* 20 kHz */
#define TIMING_DEADTIME_NS              (1000U)      /* 1 us (1000 ns) */
#define DEAD_TIME_SECONDS               (1.0e-6f)    /* 1 us in seconds */

/* Clock Expectations (documentary) */
#define CLOCK_REQUIRES_XTAL             (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ  (300U)

/* Interrupt Requirements */
#define ISR_PRIORITY_ATOM               (20U)        /* CPU0 priority 20 */

/* LED for ISR diagnostic toggle (can be overridden externally if needed) */
#ifndef LED
#define LED &MODULE_P03, 9
#endif

/* Initial duty (percent) and step for runtime update */
#define PHASE_INIT_DUTY                 (50.0f)
#define PHASE_DUTY_STEP                 (10.0f)

/* ============================================================================
 * Pin assignment placeholders (TOUT pins TBD pending board pin availability)
 * Provide real pin symbols from IfxEgtm_PinMap.h in your project configuration.
 * Example when available: &IfxEgtm_ATOM2_4_TOUTxx_Pyy_z_OUT
 * ==========================================================================*/
#ifndef PHASE_U_HS
#define PHASE_U_HS                      (0)   /* Replace with &IfxEgtm_ATOM2_4_TOUTxx_Pyy_z_OUT */
#endif
#ifndef PHASE_U_LS
#define PHASE_U_LS                      (0)   /* Replace with &IfxEgtm_ATOM2_4N_TOUTxx_Pyy_z_OUT if available, or complementary pin */
#endif
#ifndef PHASE_V_HS
#define PHASE_V_HS                      (0)   /* Replace with &IfxEgtm_ATOM2_5_TOUTxx_Pyy_z_OUT */
#endif
#ifndef PHASE_V_LS
#define PHASE_V_LS                      (0)   /* Replace with &IfxEgtm_ATOM2_5N_TOUTxx_Pyy_z_OUT if available, or complementary pin */
#endif
#ifndef PHASE_W_HS
#define PHASE_W_HS                      (0)   /* Replace with &IfxEgtm_ATOM2_6_TOUTxx_Pyy_z_OUT */
#endif
#ifndef PHASE_W_LS
#define PHASE_W_LS                      (0)   /* Replace with &IfxEgtm_ATOM2_6N_TOUTxx_Pyy_z_OUT if available, or complementary pin */
#endif

/* ============================================================================
 * Status reporting (in-memory) per requirements
 * ==========================================================================*/
typedef enum
{
    EGTM_ATOM_PWM_STATUS_OK           = 0x00000000u,
    EGTM_ATOM_PWM_STATUS_CLOCK_ERROR  = 0x00000001u,
    EGTM_ATOM_PWM_STATUS_CFG_ERROR    = 0x00000002u,
    EGTM_ATOM_PWM_STATUS_PIN_ERROR    = 0x00000004u
} EgtmAtom3phInv_LastError;

typedef struct
{
    EgtmAtom3phInv_LastError lastError; /* enum/bitmask */
    uint32                    isrCount;  /* ISR invocation counter */
    uint32                    missedUpdates; /* Missed duty updates detected in SW */
} EgtmAtom3phInv_Status;

/* ============================================================================
 * Driver/application state for 3-phase inverter
 * ==========================================================================*/
typedef struct
{
    IfxEgtm_Pwm           pwm;                              /* PWM Driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];        /* Channel data after init */
    float32               dutyCycles[NUM_OF_CHANNELS];      /* Duty cycle values (%) */
    float32               phases[NUM_OF_CHANNELS];          /* Phase shift values (deg or %) */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];       /* Dead time values (rising/falling) */
} EgtmAtom3phInv;

/* Global status (in-memory) */
extern EgtmAtom3phInv_Status g_egtmAtom3phInvStatus;

/* Public APIs (from SW Detailed Design) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxEgtm_periodEventFunction(void *data);
void interruptEgtmAtomPeriod(void);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
