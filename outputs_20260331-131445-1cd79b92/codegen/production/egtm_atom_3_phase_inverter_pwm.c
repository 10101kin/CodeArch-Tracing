/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver: EGTM Cluster 0 ATOM0 CH0..CH2 complementary PWM @20 kHz (center-aligned, 1 us dead-time)
 * plus ATOM0 CH3 edge-aligned 50% trigger routed to ADC trigger fabric.
 *
 * Notes:
 * - Clocks are enabled using the mandatory EGTM CMU enable guard.
 * - Complementary outputs use active-high for HS and active-low for LS, with 1.0 us hardware dead-time.
 * - No watchdog handling here (per AURIX architecture rules).
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD types and drivers */
#include "Ifx_Types.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Atom_Timer.h"
#include "IfxEgtm_Trigger.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =========================
 * Driver configuration macros
 * ========================= */
#define EGTM_NUM_OF_CHANNELS         (3U)
#define EGTM_PWM_FREQUENCY_HZ        (20000.0f)
#define PHASE_U_DUTY_INIT_PERCENT    (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT    (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT    (75.0f)
#define PHASE_DUTY_STEP_PERCENT      (0.01f)

/* LED: P03.9 (compound macro arguments for IfxPort_* API) */
#define LED                           (&MODULE_P03), (9U)

/* Complementary PWM pin assignments (validated pin symbols only). 
 * CH0 (U phase) pins available in validated list; CH1/CH2 placeholders are NULL_PTR to be replaced at integration.
 */
#define PHASE_U_HS                    (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                    (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)
#define PHASE_V_HS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT when available */
#define PHASE_V_LS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT when available */
#define PHASE_W_HS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT when available */
#define PHASE_W_LS                    (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT when available */

/* ADC trigger TOUT (user requested P33.0 not present in validated list) */
#define EGTM_TRIG_TOUT                (NULL_PTR) /* Replace with proper &IfxEgtm_ATOM0_3[_N]_TOUTx_P33_0_OUT when available */

/* =========================
 * Module persistent state
 * ========================= */
typedef struct
{
    IfxEgtm_Pwm             pwm;                                      /* Unified EGTM PWM driver handle */
    IfxEgtm_Pwm_Channel     channels[EGTM_NUM_OF_CHANNELS];           /* Persistent channel runtime data */
    float32                 dutyCycles[EGTM_NUM_OF_CHANNELS];         /* Duty in percent (0..100) */
    float32                 phases[EGTM_NUM_OF_CHANNELS];             /* Phase offsets in percent (0..100) */
    IfxEgtm_Pwm_DeadTime    deadTimes[EGTM_NUM_OF_CHANNELS];          /* Dead-time per channel (seconds) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInvState;

/* =========================
 * Internal helpers / ISR
 * ========================= */

/*
 * Period event callback placeholder (not used; kept for completeness)
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/*
 * Configure ATOM trigger pin and route to ADC trigger fabric
 */
static void configureEgtmAtomTrigger(uint8 channel)
{
    /* Route ATOM0 CH3 to a physical TOUT pad if the pin symbol is available */
    if (EGTM_TRIG_TOUT != NULL_PTR)
    {
        IfxEgtm_PinMap_setAtomTout(EGTM_TRIG_TOUT, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }

    /* Connect EGTM trigger to ADC trigger fabric (Cluster 0, source/channel per system) */
    /* Using casts to satisfy enum types without inventing identifiers. Signal index 0 as requested. */
    (void)IfxEgtm_Trigger_trigToAdc(
        IfxEgtm_Cluster_0,
        (IfxEgtm_TrigSource)0u,                /* Source: ATOM0 */
        (IfxEgtm_TrigChannel)channel,          /* Channel: CH3 */
        (IfxEgtm_Cfg_AdcTriggerSignal)0u       /* AdcTriggerSignal_0 */
    );
}

/*
 * ADC result ISR: toggle LED and return
 */
void resultISR(void)
{
    IfxPort_togglePin(LED);
}

/* =========================
 * Public API implementations
 * ========================= */

/*
 * Initialize EGTM CLS0 ATOM0 CH0..CH2 complementary PWM (20 kHz, center-aligned, 1 us DT)
 * and ATOM0 CH3 edge-aligned 50% trigger routed to ADC trigger fabric.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM_NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_NUM_OF_CHANNELS];
    IfxEgtm_Atom_Timer           triggerTimer; /* ATOM trigger helper */

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration for complementary pairs */
    /* Phase U (CH0) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* HS active-high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* LS active-low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (CH1) */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS; /* to be updated at integration */
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS; /* to be updated at integration */
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (CH2) */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS; /* to be updated at integration */
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS; /* to be updated at integration */
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (1.0 us rising/falling) using DTM clock = CMU CLK0 */
    dtmConfig[0].deadTime.rising = 1.0e-6f; dtmConfig[0].deadTime.falling = 1.0e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1.0e-6f; dtmConfig[1].deadTime.falling = 1.0e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1.0e-6f; dtmConfig[2].deadTime.falling = 1.0e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Per-channel configuration (logical channel indices 0..2) */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR; /* no ISR attachment */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* Main PWM config */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)EGTM_NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;      /* Use CMU CLK0 for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;

    /* 6) EGTM enable guard + CMU setup (GCLK=module freq, CLK0=module freq, enable FXCLK|CLK0) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Initialize unified EGTM PWM (applies initial duty and dead-time) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInvState.pwm, &g_egtmAtom3phInvState.channels[0], &config);

    /* Persist initial state */
    g_egtmAtom3phInvState.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInvState.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInvState.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInvState.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInvState.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInvState.phases[2]     = channelConfig[2].phase;
    g_egtmAtom3phInvState.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInvState.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInvState.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 8) Configure ATOM0 CH3 as 50% edge-aligned trigger */
    {
        (void)IfxEgtm_Atom_Timer_setFrequency(&triggerTimer, EGTM_PWM_FREQUENCY_HZ);
        {
            uint32 period = IfxEgtm_Atom_Timer_getPeriod(&triggerTimer);
            uint32 triggerPoint = (period >> 1); /* 50% position */
            IfxEgtm_Atom_Timer_setTrigger(&triggerTimer, triggerPoint);
        }
        IfxEgtm_Atom_Timer_run(&triggerTimer);
    }

    /* 9) Route trigger to pin (if available) and connect to ADC trigger fabric */
    configureEgtmAtomTrigger((uint8)IfxEgtm_Pwm_SubModule_Ch_3);

    /* 10) LED: configure as push-pull output; do not drive a level here */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update three PWM channel duties immediately. requestDuty[] is in percent [0..100].
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    float32 d0 = requestDuty[0];
    float32 d1 = requestDuty[1];
    float32 d2 = requestDuty[2];

    /* Clamp to [0, 100] */
    if (d0 < 0.0f) { d0 = 0.0f; } else if (d0 > 100.0f) { d0 = 100.0f; }
    if (d1 < 0.0f) { d1 = 0.0f; } else if (d1 > 100.0f) { d1 = 100.0f; }
    if (d2 < 0.0f) { d2 = 0.0f; } else if (d2 > 100.0f) { d2 = 100.0f; }

    /* Store state */
    g_egtmAtom3phInvState.dutyCycles[0] = d0;
    g_egtmAtom3phInvState.dutyCycles[1] = d1;
    g_egtmAtom3phInvState.dutyCycles[2] = d2;

    /* Apply immediately to hardware */
    IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3phInvState.pwm, IfxEgtm_Pwm_SyncChannelIndex_0, d0);
    IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3phInvState.pwm, IfxEgtm_Pwm_SyncChannelIndex_1, d1);
    IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3phInvState.pwm, IfxEgtm_Pwm_SyncChannelIndex_2, d2);
}
