#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm driver.
 *        Configures 3 complementary PWM pairs (TOM1 on P00.2..P00.7),
 *        center-aligned at 20 kHz with 0.5 us dead-time.
 */
void GTM_TOM_3_Phase_Inverter_PWM_init(void);

/**
 * \brief Step each duty by +10 percentage points with wrap-around at 100% -> 0%.
 *        Applies the three updated duties in one synchronized immediate update.
 */
void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void);

#ifdef __cplusplus

/* Function prototypes (auto-generated) */
void IfxGtm_periodEventFunction(void *data);
void interruptGtmTom(void);

}
#endif

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
