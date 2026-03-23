/**
 * @file egtm_atom_3_phase_inverter_pwm.h
 * @brief eGTM ATOM 3-Phase Inverter PWM (TC4xx) - Production Header
 *
 * This module configures a 3-phase complementary PWM using the unified IfxEgtm_Pwm driver
 * with DTM-based deadtime insertion and a single period interrupt on CPU0.
 *
 * Requirements implemented:
 *  - 20 kHz center-aligned complementary PWM with 1 us rising/falling deadtime
 *  - Synchronous updates for per-phase duty changes
 *  - eGTM CMU clock setup and ATOM Clk1 selection
 *  - Period ISR on CPU0 with priority 20
 *
 * Notes:
 *  - Watchdog disable must NOT be placed in this driver (only in CpuN_Main.c)
 *  - Use generic pin map headers (no family-specific suffixes)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Configuration constants from requirements (highest priority) 
 * ========================================================================= */
#define NUM_OF_CHANNELS                   (3U)
#define TIMING_PWM_FREQUENCY_HZ           (20000.0f)      /* 20 kHz */
#define TIMING_CMU_CLOCK_HZ               (100000000.0f)  /* 100 MHz */
#define TIMING_DTM_DEADTIME_TICKS         (100U)          /* 100 ticks at 100 MHz = 1 us */
#define TIMING_TIMER_TICKS_FULL_PERIOD    (5000U)
#define TIMING_TIMER_TICKS_HALF_PERIOD    (2500U)
#define TIMING_SYNCHRONOUS_UPDATE         (1)
#define CLOCK_REQUIRES_XTAL               (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ    (300U)
#define CLOCK_EGTM_CMU_CLK_MHZ            (100U)

/* Runtime parameter macros (reference naming preserved) */
#define PWM_FREQUENCY                     (TIMING_PWM_FREQUENCY_HZ)
#define ISR_PRIORITY_ATOM                 (20)
#define PHASE_U_DUTY                      (25.0f)
#define PHASE_V_DUTY                      (50.0f)
#define PHASE_W_DUTY                      (75.0f)
#define PHASE_DUTY_STEP                   (10.0f)

/* Derived deadtime in seconds (100 ticks @ 100 MHz = 1 us) */
#define DEAD_TIME_SECONDS                 ((float32)(TIMING_DTM_DEADTIME_TICKS) / (float32)(TIMING_CMU_CLOCK_HZ))

/* LED for ISR diagnostic toggle (TC4xx example) */
#define LED                               &MODULE_P03, 9

/* =========================================================================
 * Pin selection (use generic PinMap symbols)
 * These selections target typical P02.x pairs; adjust as needed per board.
 * ========================================================================= */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM0_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS   (&IfxEgtm_ATOM0_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS   (&IfxEgtm_ATOM0_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS   (&IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT)

/* =========================================================================
 * Public API
 * ========================================================================= */

/**
 * @brief Initialize the eGTM-based 3-phase complementary PWM (ATOM).
 */
void initEgtmAtom3phInv(void);

/**
 * @brief Runtime duty update with synchronous apply on next PWM period.
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
