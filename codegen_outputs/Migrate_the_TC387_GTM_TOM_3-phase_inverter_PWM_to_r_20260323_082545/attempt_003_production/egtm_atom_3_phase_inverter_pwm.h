#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Requirements-driven configuration values */
#define TIMING_PWM_FREQUENCY_HZ            (20000.0f)     /* 20 kHz */
#define TIMING_CMU_CLOCK_HZ                (100000000.0f) /* 100 MHz GCLK */
#define TIMING_DTM_DEADTIME_TICKS          (100U)
#define TIMING_TIMER_TICKS_FULL_PERIOD     (5000U)
#define TIMING_TIMER_TICKS_HALF_PERIOD     (2500U)
#define TIMING_SYNCHRONOUS_UPDATE          (1)
#define CLOCK_REQUIRES_XTAL                (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ     (300U)
#define CLOCK_EGTM_CMU_CLK_MHZ             (100U)

/* Runtime/demo parameters (percent-based duty model, as in unified driver) */
#define NUM_OF_CHANNELS                    (3U)
#define ISR_PRIORITY_ATOM                  (20)
#define PWM_FREQUENCY                      (TIMING_PWM_FREQUENCY_HZ)
#define PHASE_U_DUTY                       (25.0f)
#define PHASE_V_DUTY                       (50.0f)
#define PHASE_W_DUTY                       (75.0f)
#define PHASE_DUTY_STEP                    (10.0f)

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);

}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
