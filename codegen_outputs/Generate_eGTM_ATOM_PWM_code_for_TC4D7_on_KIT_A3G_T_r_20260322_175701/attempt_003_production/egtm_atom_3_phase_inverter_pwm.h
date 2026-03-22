/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Production eGTM ATOM 3-phase inverter PWM driver (TC4xx / TC387 target)
 *
 * Follows unified IfxEgtm_Pwm API initialization pattern with OutputConfig,
 * DtmConfig, ChannelConfig arrays and interrupt configuration through
 * IfxEgtm_Pwm_InterruptConfig.
 *
 * Notes:
 * - Watchdog handling MUST be done only in CpuN_Main.c (not here).
 * - Generic PinMap header is used (no family-specific suffixes).
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxSrc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================
 * Requirements-driven constants
 * ============================= */
#define NUM_OF_CHANNELS                  (3u)

/* Timing requirements */
#define PWM_FREQUENCY                    (20000.0f)   /* 20 kHz */
#define TIMING_PERIOD_US                 (50.0f)
#define TIMING_SYNCSTART                 (TRUE)
#define TIMING_SYNCUPDATE                (TRUE)

/* Clocks and device configuration requirements */
#define CLOCK_REQUIRES_XTAL              (TRUE)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ   (300u)

/* eGTM cluster/unit from requirements */
#define EGTM_CLUSTER_INDEX               (1u)
#define EGTM_ATOM_UNIT                   (1u)         /* Selected via pin mapping */

/* Dead-time (identical rising/falling) */
#define DEAD_TIME_SECONDS                (1.0e-6f)    /* 1 us */

/* Initial duty cycles (percent representation: 0..100) */
#define PHASE_U_DUTY                     (25.0f)
#define PHASE_V_DUTY                     (50.0f)
#define PHASE_W_DUTY                     (75.0f)

/* Runtime duty step (behavioral requirement for ramp update) */
#define PHASE_DUTY_STEP                  (10.0f)

/* ISR priority macro (use EXACT name as reference patterns) */
#define ISR_PRIORITY_ATOM                (20)

/* LED pin (diagnostic toggle in ISR) - TC4xx example */
#define LED                               &MODULE_P03, 9

/* =============================
 * Proposed TOUT pin assignments
 * =============================
 * From requirements (proposed):
 *   U: HS=P02.0, LS=P02.7
 *   V: HS=P02.1, LS=P02.4
 *   W: HS=P02.2, LS=P02.5
 *
 * Note: Use generic PinMap symbols. Verify actual availability in
 * IfxEgtm_PinMap.h for your specific device/board variant.
 */
#define PHASE_U_HS   &IfxEgtm_ATOM1_0_TOUT0_P02_0_OUT
#define PHASE_U_LS   &IfxEgtm_ATOM1_0N_TOUT7_P02_7_OUT
#define PHASE_V_HS   &IfxEgtm_ATOM1_1_TOUT1_P02_1_OUT
#define PHASE_V_LS   &IfxEgtm_ATOM1_1N_TOUT4_P02_4_OUT
#define PHASE_W_HS   &IfxEgtm_ATOM1_2_TOUT2_P02_2_OUT
#define PHASE_W_LS   &IfxEgtm_ATOM1_2N_TOUT5_P02_5_OUT

/* =============================
 * Driver/application data model
 * ============================= */
typedef struct {
    IfxEgtm_Pwm           pwm;                          /* PWM driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];    /* Channel data after init */
    float32               dutyCycles[NUM_OF_CHANNELS];  /* Duty cycle values (percent) */
    float32               phases[NUM_OF_CHANNELS];      /* Phase shift values (deg or s) */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];   /* Dead-time (rising/falling) */
} EgtmAtom3phInv;

/* Global instance (internal linkage provided in C file) */
extern EgtmAtom3phInv g_egtmAtom3phInv;

/* =============================
 * Public API (from SW Detailed Design)
 * ============================= */

/*
 * Initialize the eGTM for 3-phase complementary PWM using ATOM with DTM dead-time.
 * Implements the exact high-level configuration pattern with:
 *  - eGTM enable + CMU GCLK/CLK0 setup
 *  - IfxEgtm_Pwm_initConfig + per-channel Output/DTM/Interrupt configs
 *  - Center-aligned, syncStart, syncUpdate, frequency, and clock sources
 *  - Complementary outputs (HS active-high, LS active-low), push-pull, pad driver CMOS auto speed1
 */
void initEgtmAtom3phInv(void);

/*
 * Perform a synchronous duty-cycle ramp update (wrap-to-zero before increment when >= 100%).
 * Uses the unified driver's duty update immediate API so shadowed updates take effect
 * coherently at the next period boundary (with syncUpdateEnabled in config).
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
