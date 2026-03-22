/*
 * egtm_atom_3_phase_inverter_pwm.h
 * TC4xx eGTM ATOM 3-Phase Inverter PWM (production)
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration values from requirements */
#define NUM_OF_CHANNELS                 (3U)
#define PWM_MODE_CENTER_ALIGNED         (1U) /* informational */
#define PWM_COMPLEMENTARY_OUTPUTS       (1U) /* True */
#define PWM_DEADTIME_US                 (1.0f)
#define PWM_UPDATE_MODE_SYNC_ON_PERIOD  (1U)
#define TIMING_CMU_CLK0_HZ              (100000000.0f)
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)
#define TIMING_TICKS_PER_PERIOD         (2500U)
#define TIMING_DEADTIME_TICKS           (100U)
#define CLOCK_REQUIRES_XTAL             (1U)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ  (300U)

/* Runtime/Reference-compatible macros */
#define PWM_FREQUENCY                   (20000.0f)   /* Hz (from requirements) */
#define ISR_PRIORITY_ATOM               (20)
#define PHASE_U_DUTY                    (25.0f)
#define PHASE_V_DUTY                    (50.0f)
#define PHASE_W_DUTY                    (75.0f)
#define PHASE_DUTY_STEP                 (10.0f)

/* LED for ISR diagnostic toggle (TC4xx example pin) */
#define LED                             &MODULE_P03, 9

/*
 * Pin selection note (requirements):
 * Select six valid EGTM_TOUT pins and O-selects for X1/X2 balls per TC4D7 LFBGA-292 Port Function tables.
 * Placeholders left as NULL until validated. Do not bind unverified pins.
 */
#define PHASE_U_HS                      (0)  /* placeholder: IfxEgtm_ATOM0_0_TOUTxxx_Pyy_z_OUT */
#define PHASE_U_LS                      (0)  /* placeholder: complementary (_N) TOUT */
#define PHASE_V_HS                      (0)
#define PHASE_V_LS                      (0)
#define PHASE_W_HS                      (0)
#define PHASE_W_LS                      (0)

/* Basic status reporting (from requirements) */
typedef struct
{
    boolean basicStatusEnabled;          /* STATUS_REPORTING_ENABLE_BASIC_STATUS */
    boolean clk0Enabled;                 /* STATUS_REPORTING_FIELDS_CLK0_ENABLED */
    boolean dtmClkFromClk0;              /* STATUS_REPORTING_FIELDS_DTM_CLK_FROM_CLK0 */
    uint32  isrMissedCount;              /* STATUS_REPORTING_FIELDS_ISR_MISSED_COUNT */
    uint32  lastErrorCode;               /* STATUS_REPORTING_FIELDS_LAST_ERROR_CODE */
    uint8   phaseEnabledMask;            /* STATUS_REPORTING_FIELDS_PHASE_ENABLED_MASK */
} EgtmAtom3phInv_Status;

extern volatile EgtmAtom3phInv_Status g_egtmAtom3phInvStatus;

/* Public API (from SW Detailed Design) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
