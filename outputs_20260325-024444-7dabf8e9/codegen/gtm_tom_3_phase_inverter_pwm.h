#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration values from requirements */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ      (300)
#define CLOCK_GTM_GCLK_MHZ                 (300)
#define CLOCK_CMU_CLK0_MHZ                 (100)

#define PWM_FREQUENCY_HZ                   (10000.0f)   /* 10 kHz center-aligned */
#define DEAD_TIME_SECONDS                  (5.0e-7f)    /* 500 ns */

#define INITIAL_DUTY_PERCENT_U             (25.0f)
#define INITIAL_DUTY_PERCENT_V             (50.0f)
#define INITIAL_DUTY_PERCENT_W             (75.0f)
#define DUTY_STEP_PERCENT                  (10.0f)

#define NUM_OF_CHANNELS                    (3u)         /* 3 complementary phase legs (U,V,W) */

/* TOM selection to keep all six outputs in TOM0 TGC0 for atomic updates */
#define TOM_SELECTION_MODULE               IfxGtm_Tom_0
#define TOM_MASTER_TIMER_CH                IfxGtm_Tom_Ch_7  /* Dedicated timebase in TOM0, TGC0 */

/* LED for ISR diagnostics (toggle on TOM ISR) */
#define LED                                &MODULE_P13, 0

/* ISR priority macro (TC3xx TOM) */
#ifndef ISR_PRIORITY_TOM
#define ISR_PRIORITY_TOM                   (20)
#endif

/*
 * Pin routing (validated examples for TC3xx TOM0 channels within TGC0)
 * High-side = normal output, Low-side = complementary (_N) output of the same channel
 *
 * Phase U: TOM0 channel 0
 */
#define PHASE_U_HS   &IfxGtm_TOM0_0_TOUT0_P02_0_OUT
#define PHASE_U_LS   &IfxGtm_TOM0_0N_TOUT7_P02_7_OUT

/* Phase V: TOM0 channel 1 */
#define PHASE_V_HS   &IfxGtm_TOM0_1_TOUT86_P14_6_OUT
#define PHASE_V_LS   &IfxGtm_TOM0_1N_TOUT90_P14_10_OUT

/* Phase W: TOM0 channel 2 */
#define PHASE_W_HS   &IfxGtm_TOM0_2_TOUT88_P14_8_OUT
#define PHASE_W_LS   &IfxGtm_TOM0_2N_TOUT93_P13_2_OUT

/* Optional EVADC trigger routing (disabled by default) */
#define EVADC_TRIG_TOM_CH                  IfxGtm_Tom_Ch_11
#define EVADC_TRIG_TOUT                    &IfxGtm_TOM0_11_TOUT3_P02_3_OUT  /* TRIG2 via P02.3 when enabled */

/* API contract - public functions */
void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptGtmTom(void);
void IfxGtm_periodEventFunction(void *data);

}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
