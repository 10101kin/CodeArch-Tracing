/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver for EGTM ATOM 3-phase complementary PWM with ADC trigger (TC4xx)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Atom_Timer.h"
#include "IfxEgtm_Trigger.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =========================================================
 * Configuration macros (from requirements and migration values)
 * ========================================================= */
#define EGTM_PWM_NUM_CHANNELS           (3u)
#define EGTM_ADC_TRIG_NUM_CHANNELS      (1u)

#define PWM_FREQUENCY_HZ                (20000.0f)
#define PHASE_U_DUTY_PERCENT            (25.0f)
#define PHASE_V_DUTY_PERCENT            (50.0f)
#define PHASE_W_DUTY_PERCENT            (75.0f)
#define PHASE_DUTY_STEP_PERCENT         (0.01f)
#define ADC_TRIG_DUTY_PERCENT           (50.0f)

/* Interrupt priorities (not used here, defined per requirements) */
#define INTERRUPT_PRIORITY_EVADC_CUR    (100u)
#define ISR_PRIORITY_ATOM               (25u)   /* ADC result ISR priority per requirements */

/* LED pin macro (compound: port, pin) */
#define LED &MODULE_P03, 9

/* Validated pin symbols (only using those provided) */
#define PHASE_U_HS  (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS  (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)

/* These mappings are not present in the validated list; leave NULL_PTR with guidance comments. */
#define PHASE_V_HS  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT when available */
#define PHASE_V_LS  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT when available */
#define PHASE_W_HS  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT when available */
#define PHASE_W_LS  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT when available */

/* ATOM0 CH3 trigger TOUT on P33.0 - symbol not in validated list; set to NULL_PTR with guidance. */
#define ATOM0_CH3_TOUT_P33_0  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_3_TOUTx_P33_0_OUT when available */

/* ADC trigger selection placeholders (type-safe casts; replace with concrete enums at integration) */
#define EGTM_ADC_TRIG_SIGNAL    ((IfxEgtm_Cfg_AdcTriggerSignal)0)   /* AdcTriggerSignal_0 */
#define EGTM_TRIG_SOURCE_ATOM0  ((IfxEgtm_TrigSource)0)             /* ATOM source selection */

/* =========================================================
 * Module persistent state
 * ========================================================= */

typedef struct
{
    IfxEgtm_Pwm            pwm;                                     /* PWM driver handle */
    IfxEgtm_Pwm_Channel    channels[EGTM_PWM_NUM_CHANNELS];         /* Channel runtime data (must persist) */
    float32                dutyCycles[EGTM_PWM_NUM_CHANNELS];       /* Duty cycle state in percent */
    float32                phases[EGTM_PWM_NUM_CHANNELS];           /* Phase offset state in percent */
    IfxEgtm_Pwm_DeadTime   deadTimes[EGTM_PWM_NUM_CHANNELS];        /* Dead-time state (s) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;                   /* Zero-initialized persistent state */

/* =========================================================
 * Internal helper (private)
 * ========================================================= */
static void configureEgtmAtomTrigger(uint8 channel)
{
    /* Route ATOM0 CH3 to its TOUT pin if mapping available */
    if (ATOM0_CH3_TOUT_P33_0 != NULL_PTR)
    {
        IfxEgtm_PinMap_setAtomTout(ATOM0_CH3_TOUT_P33_0, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }

    /* Connect ATOM0 CH3 to ADC trigger fabric. Edge/mux selection depends on SoC integration; use default signal 0. */
    (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0, EGTM_TRIG_SOURCE_ATOM0, (IfxEgtm_TrigChannel)channel, EGTM_ADC_TRIG_SIGNAL);
}

/* =========================================================
 * Minimal ADC result ISR (public API per requirements)
 * ========================================================= */
/**
 * ADC conversion-complete ISR: toggle LED and return (no additional processing).
 */
void resultISR(void)
{
    IfxPort_togglePin(LED);
}

/* =========================================================
 * Initialization
 * ========================================================= */
/**
 * Initialize EGTM CLS0 ATOM resources for 3-phase complementary PWM on logical channels 0..2 at 20 kHz
 * center-aligned with 1 us hardware dead-time. Also configures ATOM0 CH3 as 50% edge-aligned trigger
 * output routed to the ADC trigger fabric.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration structures locally */
    IfxEgtm_Pwm_Config         config;
    IfxEgtm_Pwm_ChannelConfig  channelConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig   output[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig      dtmConfig[EGTM_PWM_NUM_CHANNELS];

    IfxEgtm_Atom_Timer         atomTimer; /* Trigger timer handle (logical channel 3) */

    /* 2) Load default configuration */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Populate output[] for three complementary phases (HS active-high, LS active-low) */
    /* Phase U (CH0) */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (CH1) */
    output[1].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS; /* To be assigned during board integration */
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS; /* To be assigned during board integration */
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (CH2) */
    output[2].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS; /* To be assigned during board integration */
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS; /* To be assigned during board integration */
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1.0 us rising/falling, DTM clock from CMU CLK0 */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Per-channel configuration: CH0..CH2, center-aligned duties 25/50/75%, no interrupt */
    /* CH0 → Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR;    /* No period interrupt */

    /* CH1 → Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* CH2 → Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* Main PWM configuration */
    config.cluster              = IfxEgtm_Cluster_0;
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;
    config.alignment            = IfxEgtm_Pwm_Alignment_center;
    config.numChannels          = (uint8)EGTM_PWM_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.atom     = (uint32)IfxEgtm_Cmu_Clk_0;              /* Use CMU CLK0 for ATOM */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;      /* DTM clock from CMU CLK0 */
    config.syncUpdateEnabled    = TRUE;                                    /* Shadow transfer on period */
    config.syncStart            = TRUE;                                    /* Start channels synchronously */

    /* 6) EGTM module/clock enable guard (MANDATORY pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Initialize PWM driver; this applies initial duties and dead-times */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Persist initial state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure ATOM0 CH3 as edge-aligned 20 kHz trigger at 50% */
    {
        boolean ok = IfxEgtm_Atom_Timer_setFrequency(&atomTimer, PWM_FREQUENCY_HZ);
        if (ok == TRUE)
        {
            uint32 periodTicks  = IfxEgtm_Atom_Timer_getPeriod(&atomTimer);
            uint32 triggerPoint = (periodTicks >> 1); /* 50% */
            IfxEgtm_Atom_Timer_setTrigger(&atomTimer, triggerPoint);
            IfxEgtm_Atom_Timer_run(&atomTimer);
        }
        /* Route TOUT and connect to ADC trigger fabric */
        configureEgtmAtomTrigger(3u);
    }

    /* 10) Configure LED pin as push-pull output; no initial drive here */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =========================================================
 * Duty update API
 * ========================================================= */
/**
 * Update the three logical PWM channel duties immediately.
 * requestDuty[0..2] are in percent [0..100]. Values are clamped, stored to state, and applied.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    if (requestDuty == NULL_PTR)
    {
        return;
    }

    /* Clamp, store, and apply per-channel */
    {
        float32 d0 = requestDuty[0];
        if (d0 < 0.0f) { d0 = 0.0f; } else if (d0 > 100.0f) { d0 = 100.0f; }
        g_egtmAtom3phInv.dutyCycles[0] = d0;
        IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3phInv.pwm, IfxEgtm_Pwm_SyncChannelIndex_0, d0);
    }
    {
        float32 d1 = requestDuty[1];
        if (d1 < 0.0f) { d1 = 0.0f; } else if (d1 > 100.0f) { d1 = 100.0f; }
        g_egtmAtom3phInv.dutyCycles[1] = d1;
        IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3phInv.pwm, IfxEgtm_Pwm_SyncChannelIndex_1, d1);
    }
    {
        float32 d2 = requestDuty[2];
        if (d2 < 0.0f) { d2 = 0.0f; } else if (d2 > 100.0f) { d2 = 100.0f; }
        g_egtmAtom3phInv.dutyCycles[2] = d2;
        IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3phInv.pwm, IfxEgtm_Pwm_SyncChannelIndex_2, d2);
    }
}
