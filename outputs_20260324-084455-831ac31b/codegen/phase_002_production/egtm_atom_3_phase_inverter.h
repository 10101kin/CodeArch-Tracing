#ifndef EGTM_ATOM_3_PHASE_INVERTER_H
#define EGTM_ATOM_3_PHASE_INVERTER_H

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxWtu.h"
#include "IfxPort.h"
#include "IfxEgtm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Requirements-driven configuration macros */
#define NUM_OF_CHANNELS                     (3u)
#define PWM_FREQUENCY_HZ                    (20000.0f)   /* 20 kHz */
#define PWM_DEADTIME_US                     (1.0f)       /* 1.0 us */
#define PWM_ALIGNMENT_CENTER                (1u)         /* casted to enum at use */
#define PWM_SUBMODULE_ATOM                  (1u)         /* casted to enum at use */
#define EGTM_CLUSTER_0                      (0u)
#define EGTM_CLOCK_CLK0                     (0u)
#define EGTM_DTM_CLOCK_CMUCLK0              (0u)

#define PHASE_U_DUTY_PERCENT                (25.0f)
#define PHASE_V_DUTY_PERCENT                (50.0f)
#define PHASE_W_DUTY_PERCENT                (75.0f)

#define DTM_CLOCK_ASSUMPTION_MHZ            (100.0f)

/* LED configuration (low-active, default high/off) */
#define LED_PORT                            (&MODULE_P03)
#define LED_PIN                             (9u)

/* ADC trigger requirements (ATOM0, Cluster 0, CH3) */
#define ADC_TRIG_CLUSTER                    (0u)
#define ADC_TRIG_SOURCE_ATOM0               (0u)
#define ADC_TRIG_CHANNEL_3                  (3u)
#define ADC_TRIG_SIGNAL_TMADC0              (0u)

/* Driver structure for EGTM ATOM 3-phase inverter */
typedef struct {
    IfxEgtm_Pwm          pwm;                            /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];      /* Channel data after init */
    float32              dutyCycles[NUM_OF_CHANNELS];    /* Duty cycle values (percent) */
    float32              phases[NUM_OF_CHANNELS];        /* Phase shift values (deg or percent based on driver) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];     /* Dead-time values (driver format) */
} EgtmAtom3phInv;

/* Public API (from SW Detailed Design) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_H */
