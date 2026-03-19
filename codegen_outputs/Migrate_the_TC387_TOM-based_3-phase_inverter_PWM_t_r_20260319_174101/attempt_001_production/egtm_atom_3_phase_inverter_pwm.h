/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Production 3-phase inverter PWM driver for AURIX TC4xx eGTM ATOM using unified IfxEgtm_Pwm.
 *
 * - Center-aligned complementary PWM, 20 kHz, with 1 us dead-time (DTM/CDTM)
 * - Unified driver configuration via IfxEgtm_Pwm_Config and per-channel arrays
 * - Interrupt on period event (CPU0, priority 20) using ISR_PRIORITY_ATOM
 * - Status reporting in-memory: lastError (enum/bitmask), isrCount, missedUpdates
 * - NO watchdog handling here (CpuN_Main.c is responsible)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxPort.h"
#include "IfxEgtm_Pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================
 * Requirements-based macros
 * ==========================
 */
#define NUM_OF_CHANNELS               (3)

/* TIMING_PWM_FREQUENCY_HZ = 20000 (20 kHz) */
#define PWM_FREQUENCY                 (20000.0f)

/* TIMING_DEADTIME_NS = 1000 ns (1 us) */
#define DEAD_TIME_SECONDS             (1.0e-6f)

/* INTERRUPTS: CPU0 priority 20 */
#ifndef ISR_PRIORITY_ATOM
#define ISR_PRIORITY_ATOM             (20)
#endif

/* Diagnostic LED (KIT_A3G_TC4D7_LITE typical user LED on P03.9) */
#ifndef LED
#define LED                           &MODULE_P03, 9
#endif

/* Initial duties (percent). All 50% as per requirements default example. */
#define PHASE_U_DUTY                  (50.0f)
#define PHASE_V_DUTY                  (50.0f)
#define PHASE_W_DUTY                  (50.0f)

/* Step for runtime duty update example */
#define PHASE_DUTY_STEP               (10.0f)

/* ==========================
 * TOUT pin placeholders (TBD board routing)
 * ==========================
 * Provide override macros for high-side and low-side (complementary) pins per phase.
 * Use valid IfxEgtm_ATOM2_4/5/6 TOUT mappings from IfxEgtm_PinMap.h for your board.
 * If not yet assigned, they default to NULL (no routing) and PWM runs without pin output.
 */
#ifndef PHASE_U_HS
#define PHASE_U_HS                    (NULL)
#endif
#ifndef PHASE_U_LS
#define PHASE_U_LS                    (NULL)
#endif
#ifndef PHASE_V_HS
#define PHASE_V_HS                    (NULL)
#endif
#ifndef PHASE_V_LS
#define PHASE_V_LS                    (NULL)
#endif
#ifndef PHASE_W_HS
#define PHASE_W_HS                    (NULL)
#endif
#ifndef PHASE_W_LS
#define PHASE_W_LS                    (NULL)
#endif

/* ==========================
 * Status reporting structures
 * ==========================
 */
/**
 * Status/error bitmask for the PWM driver. STATUS_REPORTING_STRUCT_LASTERROR = enum/bitmask
 */
typedef enum
{
    EGTM_ATOM_PWM_STATUS_OK                = 0u,
    EGTM_ATOM_PWM_STATUS_CLK_ERROR         = 1u << 0,
    EGTM_ATOM_PWM_STATUS_CONFIG_ERROR      = 1u << 1,
    EGTM_ATOM_PWM_STATUS_RUNTIME_ERROR     = 1u << 2
} EgtmAtom3phInv_StatusFlag;

/**
 * In-memory status: lastError, ISR count, missed updates
 */
typedef struct
{
    volatile uint32           isrCount;        /* STATUS_REPORTING_STRUCT_ISRCOUNT */
    volatile uint32           missedUpdates;   /* STATUS_REPORTING_STRUCT_MISSEDUPDATES */
    volatile EgtmAtom3phInv_StatusFlag lastError; /* STATUS_REPORTING_STRUCT_LASTERROR */
} EgtmAtom3phInv_Status;

/**
 * Driver state container following unified IfxEgtm_Pwm patterns
 */
typedef struct
{
    IfxEgtm_Pwm           pwm;                               /* PWM Driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];         /* Channel data after init */
    float32               dutyCycles[NUM_OF_CHANNELS];       /* Duty cycle values (0..100 %) */
    float32               phases[NUM_OF_CHANNELS];           /* Phase shift values (deg or fraction as per API) */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];        /* Dead-time values (s) */
    volatile uint32       epoch;                             /* ISR epoch counter */
    volatile uint32       lastAppliedEpoch;                  /* Last epoch applied in update() */
    EgtmAtom3phInv_Status status;                            /* Status struct */
} EgtmAtom3phInv;

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtomPeriod(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
