#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =====================================================================
 * Configuration values from requirements (highest priority)
 * ===================================================================== */
#define NUM_OF_CHANNELS                        (3U)

/* Time/frequency requirements */
#define TIMING_PWM_FREQUENCY_HZ                (20000.0f)     /* 20 kHz */
#define TIMING_DEAD_TIME_SECONDS               (5.0e-07f)     /* 0.5 us */
#define TIMING_MIN_PULSE_SECONDS               (1.0e-06f)     /* 1.0 us */

/* Initial duty cycles in PERCENT (0.0f .. 100.0f) */
#define INITIAL_DUTY_CYCLE_PERCENT_U           (25.0f)
#define INITIAL_DUTY_CYCLE_PERCENT_V           (50.0f)
#define INITIAL_DUTY_CYCLE_PERCENT_W           (75.0f)

/* Output configuration requirements */
#define OUTPUT_COMPLEMENTARY                   (1)            /* True */
#define OUTPUT_CCX_ACTIVE_STATE                (1)            /* high */
#define OUTPUT_COUTX_ACTIVE_STATE              (1)            /* high */
/* These names reflect iLLD enums used at callsite */
#define OUTPUT_PORT_OUTPUT_MODE                IfxPort_OutputMode_pushPull
#define OUTPUT_PAD_DRIVER                      IfxPort_PadDriver_cmosAutomotiveSpeed1

/* Update model requirement */
#define OUTPUT_UPDATE_SYNCHRONOUS_ALL_CHANNELS (1)

/* Master timer/time-base requirements */
#define MASTER_TIMER_CHANNEL                   TOM1_CH0
#define MASTER_TIMER_CLOCK_SOURCE              IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0
#define MASTER_TIMER_USED_AS_TIME_BASE_ONLY    (1)

/* Clock requirement */
#define CLOCK_REQUIRES_XTAL                    (1)

/* Runtime behavior: duty step as percent of full scale (fixed fraction) */
#define DUTY_STEP_PERCENT                      (10.0f)

/* Public API (exact signatures) */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);
void GTM_TOM_3_Phase_Inverter_PWM_updateUVW(void);

#ifdef __cplusplus
}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
