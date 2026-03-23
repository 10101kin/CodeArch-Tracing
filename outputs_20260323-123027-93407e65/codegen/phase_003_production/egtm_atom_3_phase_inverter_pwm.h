#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxWtu.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================
 * Requirements-based macros
 *============================*/
#define EGTM_CLUSTER                          (0)
#define EGTM_ATOM                             (0)
#define EGTM_TRIGGER_CHANNEL                  (3)
#define EGTM_ALIGNMENT_CENTER                 (1)
#define EGTM_COMPLEMENTARY                    (1)
#define EGTM_DEADTIME_US                      (1.0f)
#define EGTM_POLARITY_HS_ACTIVE               (1)
#define EGTM_POLARITY_LS_ACTIVE               (0)

#define TIMING_PWM_FREQUENCY_HZ               (20000.0f)
#define TIMING_DEADTIME_US                    (1.0f)
#define TIMING_DUTY_UPDATE_PERIOD_MS          (500U)
#define TIMING_HEARTBEAT_PERIOD_MS            (500U)

/* Pins (from requirements) */
#define PINS_PHASE_U_HS_PORT                  (&MODULE_P20)
#define PINS_PHASE_U_HS_PIN                   (8U)
#define PINS_PHASE_U_LS_PORT                  (&MODULE_P20)
#define PINS_PHASE_U_LS_PIN                   (9U)
#define PINS_PHASE_V_HS_PORT                  (&MODULE_P20)
#define PINS_PHASE_V_HS_PIN                   (10U)
#define PINS_PHASE_V_LS_PORT                  (&MODULE_P20)
#define PINS_PHASE_V_LS_PIN                   (11U)
#define PINS_PHASE_W_HS_PORT                  (&MODULE_P20)
#define PINS_PHASE_W_HS_PIN                   (12U)
#define PINS_PHASE_W_LS_PORT                  (&MODULE_P20)
#define PINS_PHASE_W_LS_PIN                   (13U)

#define PINS_TRIGGER_OUT_PORT                 (&MODULE_P33)
#define PINS_TRIGGER_OUT_PIN                  (0U)

#define PINS_HEARTBEAT_LED_PORT               (&MODULE_P03)
#define PINS_HEARTBEAT_LED_PIN                (9U)

/* Channel counts */
#define NUM_OF_PHASE_CHANNELS                 (3U)
#define NUM_OF_TRIG_CHANNELS                  (1U)

/*===============
 * Data structs
 *===============*/

typedef struct {
    IfxEgtm_Pwm          pwm;                              /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_PHASE_CHANNELS];  /* Channel handles */
    float32              dutyCycles[NUM_OF_PHASE_CHANNELS];/* Duty values in percent [0..100] */
    float32              phases[NUM_OF_PHASE_CHANNELS];    /* Phase shift values in degrees */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_PHASE_CHANNELS]; /* Dead time values (seconds) */
} EgtmAtom3phInv;

/* Trigger context (single channel) */
typedef struct {
    IfxEgtm_Pwm          pwm;                              /* PWM driver for trigger */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_TRIG_CHANNELS];   /* Single channel */
} EgtmAtomTrigCtx;

/*============================
 * Public API (exact names)
 *============================*/
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
